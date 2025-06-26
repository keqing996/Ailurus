#include <Ailurus/Application/RenderSystem/RenderPass/RenderPass.h>
#include <Ailurus/Utility/EnumReflection.h>
#include <Ailurus/Application/Application.h>
#include <Ailurus/Application/RenderSystem/RenderSystem.h>
#include <Ailurus/Application/RenderSystem/Uniform/UniformBindingPoint.h>
#include <Ailurus/Application/AssetsSystem/Mesh/Mesh.h>
#include <Ailurus/Application/AssetsSystem/Model/Model.h>
#include <Ailurus/Application/SceneSystem/SceneSystem.h>
#include <Ailurus/Application/SceneSystem/Component/CompStaticMeshRender.h>
#include <VulkanSystem/VulkanSystem.h>
#include <VulkanSystem/CommandBuffer/VulkanCommandBuffer.h>
#include <VulkanSystem/Pipeline/VulkanPipelineManager.h>
#include <VulkanSystem/RenderPass/VulkanRenderPass.h>
#include <VulkanSystem/Buffer/VulkanVertexBuffer.h>
#include <VulkanSystem/Buffer/VulkanIndexBuffer.h>
#include <VulkanSystem/Buffer/VulkanUniformBuffer.h>
#include <VulkanSystem/Vertex/VulkanVertexLayoutManager.h>
#include <Ailurus/Utility/Logger.h>
#include "Ailurus/Application/RenderSystem/Uniform/UniformSet.h"
#include "Detail/RenderIntermediateVariable.h"

namespace Ailurus
{
    void RenderSystem::CollectCameraViewProjectionMatrix()
    {
		const auto& projMat = _pMainCamera->GetProjectionMatrix();
		const auto& viewMat = _pMainCamera->GetEntity()->GetModelMatrix();
		_pIntermediateVariable->viewProjectionMatrix = projMat * viewMat;
    }

    void RenderSystem::CollectPipelineMeshMap()
    {
		const auto allEntities = Application::Get<SceneSystem>()->GetAllRawEntities();
        auto& renderInfo = _pIntermediateVariable->renderInfo;
        for (const auto pEntity : allEntities)
        {
			const auto pMeshRender = pEntity->GetComponent<CompStaticMeshRender>();
            if (pMeshRender == nullptr)
                continue;

            const auto& modelRef = pMeshRender->GetModelAsset();
            if (!modelRef)
                continue;

            const auto& materialInstRef = pMeshRender->GetMaterialInstanceAsset();
            if (!materialInstRef)
				continue;

			const auto* pMaterial = materialInstRef->GetTargetMaterial();
        	const auto materialId = pMaterial->GetAssetId();
            for (auto i = 0; i < EnumReflection<RenderPassType>::Size(); i++)
            {
                auto passType = static_cast<RenderPassType>(i);

            	// Fill render pass info
            	auto& renderPassInfo = renderInfo.renderPassesMap[passType];

                if (!pMaterial->HasRenderPass(passType))
                    continue;

            	// Fill material render info
            	if (!renderPassInfo.materialsMap.contains(materialId))
            		renderPassInfo.materialsMap[materialId].pMaterial = pMaterial;

				auto& materialInfo =  renderPassInfo.materialsMap[materialId];

				const auto& allMeshes = modelRef->GetMeshes();
				for (const auto& pMesh: allMeshes)
				{
					const auto vertexLayoutId = pMesh->GetVertexLayoutId();

					// Create vertex layout render info
					if (!materialInfo.vertexLayoutsMap.contains(vertexLayoutId))
					{
						const auto* pLayout = Application::Get<VulkanSystem>()->GetVertexLayoutManager()->GetLayout(vertexLayoutId);
						materialInfo.vertexLayoutsMap[vertexLayoutId].pVertexLayout = pLayout;
					}

					auto& vertexLayoutInfo = materialInfo.vertexLayoutsMap[vertexLayoutId];
					for (const auto& mesh : modelRef.Get()->GetMeshes())
						vertexLayoutInfo.meshes.push_back({ mesh.get(), pEntity });
				}
			}
		}
    }

	void RenderSystem::CreateIntermediateVariable()
    {
        _pIntermediateVariable = std::make_unique<RenderIntermediateVariable>();
    }

	void RenderSystem::RenderScene()
	{
		if (_needRebuildSwapChain)
			ReBuildSwapChain();

        CollectCameraViewProjectionMatrix();
        CollectPipelineMeshMap();

		auto* pCommandBuffer = Application::Get<VulkanSystem>()->GetFrameContext()->GetRecordingCommandBuffer();
		
		UpdateGlobalUniformBuffer();
		RenderForwardPass(pCommandBuffer);

		Application::Get<VulkanSystem>()->RenderFrame(&_needRebuildSwapChain);
	}

	void RenderSystem::UpdateGlobalUniformBuffer()
	{
		auto bindingPointOffset = _pGlobalUniformSet->GetBindingPointOffsetInUniformBuffer(0);
		const UniformBindingPoint* pBindingPoint = _pGlobalUniformSet->GetBindingPoint(0);

		auto vpMatOffset = pBindingPoint->GetAccessOffset(GetGlobalUniformAccessNameViewProjMat());
		if (vpMatOffset)
		{
			auto mat = _pIntermediateVariable->viewProjectionMatrix;
			_pGlobalUniformBuffer->WriteData(bindingPointOffset + *vpMatOffset, mat);
		}

		auto cameraPosOffset = pBindingPoint->GetAccessOffset(GetGlobalUniformAccessNameCameraPos());
		if (cameraPosOffset)
		{
			const auto& cameraPos = _pMainCamera->GetEntity()->GetPosition();
			_pGlobalUniformBuffer->WriteData(bindingPointOffset + *cameraPosOffset, cameraPos);
		}

		_pGlobalUniformBuffer->TransitionDataToGpu();
	}

	void RenderSystem::RenderForwardPass(VulkanCommandBuffer* pCommandBuffer)
	{
    	auto pForwardPass = GetRenderPass(RenderPassType::Forward);
    	if (pForwardPass == nullptr)
    		return;

		auto& renderInfo = _pIntermediateVariable->renderInfo.renderPassesMap[RenderPassType::Forward];
		if (renderInfo.materialsMap.empty())
			return;

    	// Prepare
		auto pVulkanSystem = Application::Get<VulkanSystem>();
    	auto pDescriptorPool = pVulkanSystem->GetFrameContext()->GetAllocatingDescriptorPool();

		pCommandBuffer->BeginRenderPass(pForwardPass->GetRHIRenderPass());

		for (auto& [materialId,  renderInfoPerMaterial] : renderInfo.materialsMap)
		{
			const auto pMaterial = renderInfoPerMaterial.pMaterial;
			const auto pDescriptorSetLayout = pMaterial->GetUniformSet(RenderPassType::Forward)->GetDescriptorSetLayout();

			// Allocate descriptor set
			auto descriptorSet = pDescriptorPool->AllocateDescriptorSet(pDescriptorSetLayout);

			for (auto& [vertexLayoutId, renderInfoPerVertexLayout] : renderInfoPerMaterial.vertexLayoutsMap)
			{
				// Create pipeline entry
				VulkanPipelineEntry pipelineEntry(RenderPassType::Forward, materialId, vertexLayoutId);
				const auto pPipeline = pVulkanSystem->GetPipelineManager()->GetPipeline(pipelineEntry);
				if (pPipeline == nullptr)
				{
					Logger::LogError("Pipeline not found for entry: {}",
						EnumReflection<RenderPassType>::ToString(pipelineEntry.renderPass));
					continue;
				}

				pCommandBuffer->BindPipeline(pPipeline);
				pCommandBuffer->SetViewportAndScissor();

				for (const auto& [pMesh, pEntity] : renderInfoPerVertexLayout.meshes)
				{
					std::vector<vk::DescriptorSet> descriptorSets;
					descriptorSets.push_back(descriptorSet);

					// Update descriptor set
					pCommandBuffer->BindDescriptorSet(pPipeline->GetPipelineLayout(), descriptorSets);

					// Push constant model matrix 
					pCommandBuffer->PushConstantModelMaterix(pPipeline, pEntity->GetModelMatrix());

					// Bind vertex buffer
					const auto pVertexBuffer = pMesh->GetVertexBuffer();
					pCommandBuffer->BindVertexBuffer(pVertexBuffer);

					// Bind index buffer and draw
					const auto pIndexBuffer = pMesh->GetIndexBuffer();
					if (pIndexBuffer != nullptr)
					{
						pCommandBuffer->BindIndexBuffer(pIndexBuffer);
						pCommandBuffer->DrawIndexed(pIndexBuffer->GetIndexCount());
					}
					else
					{
						pCommandBuffer->DrawNonIndexed(pMesh->GetVertexCount());
					}
				}
			}
		}

		pCommandBuffer->EndRenderPass();
	}
} // namespace Ailurus