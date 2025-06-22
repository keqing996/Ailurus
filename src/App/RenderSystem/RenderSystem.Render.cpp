#include <Ailurus/Application/RenderSystem/RenderPass/RenderPass.h>
#include <Ailurus/Utility/EnumReflection.h>
#include <Ailurus/Application/Application.h>
#include <Ailurus/Application/RenderSystem/RenderSystem.h>
#include <Ailurus/Application/AssetsSystem/Mesh/Mesh.h>
#include <Ailurus/Application/AssetsSystem/Model/Model.h>
#include <Ailurus/Application/SceneSystem/SceneSystem.h>
#include <Ailurus/Application/SceneSystem/Component/CompStaticMeshRender.h>
#include <VulkanSystem/VulkanSystem.h>
#include <vulkanSystem/CommandBuffer/VulkanCommandBuffer.h>
#include <VulkanSystem/Pipeline/VulkanPipelineManager.h>
#include <VulkanSystem/RenderPass/VulkanRenderPass.h>
#include <VulkanSystem/Buffer/VulkanVertexBuffer.h>
#include <VulkanSystem/Buffer/VulkanIndexBuffer.h>
#include <VulkanSystem/Vertex/VulkanVertexLayoutManager.h>
#include <Ailurus/Utility/Logger.h>
#include <Ailurus/Assert.h>

namespace Ailurus
{
	struct RenderIntermediateVariable
    {
		struct RenderInfoPerVertexLayout
		{
			const VulkanVertexLayout* pVertexLayout;
			std::vector<const Mesh*> meshes;
		};

		struct RenderInfoPerMaterial
		{
			const Material* pMaterial;
			std::unordered_map<uint64_t, RenderInfoPerVertexLayout> vertexLayoutsMap;
		};

		struct RenderInfoPerPass
		{
			std::unordered_map<uint32_t, RenderInfoPerMaterial> materialsMap;
		};

		struct RenderInfo
		{
			std::unordered_map<RenderPassType, RenderInfoPerPass> renderPassesMap;
		};

		// View and projection matrices
		Matrix4x4f projMatrix;
        Matrix4x4f viewMatrix;
		RenderInfo renderInfo;
    };

    void RenderSystem::CollectCameraViewProjectionMatrix()
    {
		_pIntermediateVariable->projMatrix = _pMainCamera->GetProjectionMatrix();
		_pIntermediateVariable->viewMatrix = _pMainCamera->GetEntity()->GetModelMatrix();
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

            const auto& materialRef = pMeshRender->GetMaterialAsset();
            if (!materialRef)
				continue;

        	uint32_t materialId = materialRef->GetAssetId();
			const auto* pMaterial = materialRef.Get()->GetTargetMaterial();
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

					auto vertexLayoutInfo = materialInfo.vertexLayoutsMap[vertexLayoutId];
					for (const auto& mesh : modelRef.Get()->GetMeshes())
						vertexLayoutInfo.meshes.push_back(mesh.get());
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
		RenderForwardPass(pCommandBuffer);

		Application::Get<VulkanSystem>()->RenderFrame(&_needRebuildSwapChain);
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
					Logger::LogError("Pipeline not found for entry: {}", pipelineEntry.renderPass);
					continue;
				}

				pCommandBuffer->BindPipeline(pPipeline);
				pCommandBuffer->SetViewportAndScissor();

				for (const auto& pMesh : renderInfoPerVertexLayout.meshes)
				{
					std::vector<vk::DescriptorSet> descriptorSets;
					descriptorSets.push_back(descriptorSet);

					pCommandBuffer->BindDescriptorSet(pPipeline->GetPipelineLayout(), descriptorSets);

					const auto pVertexBuffer = pMesh->GetVertexBuffer();
					pCommandBuffer->BindVertexBuffer(pVertexBuffer);

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