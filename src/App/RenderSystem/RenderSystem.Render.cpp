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
#include <Ailurus/Utility/Logger.h>
#include <Ailurus/Assert.h>

namespace Ailurus
{
	struct RenderIntermediateVariable
    {
		using PipelineMeshMap = std::unordered_map<VulkanPipelineEntry, std::vector<const Mesh*>,
			VulkanPipelineEntryHash, VulkanPipelineEntryEqual>;

		using RenderPassPipelineMeshMap = std::unordered_map<RenderPassType, PipelineMeshMap>;

		// View and projection matrices
		Matrix4x4f projMatrix;
        Matrix4x4f viewMatrix;
        RenderPassPipelineMeshMap renderPassPipelineMeshesMap;
    };

    void RenderSystem::CollectCameraViewProjectionMatrix()
    {
		_pIntermediateVariable->projMatrix = _pMainCamera->GetProjectionMatrix();
		_pIntermediateVariable->viewMatrix = _pMainCamera->GetEntity()->GetModelMatrix();
    }

    void RenderSystem::CollectPipelineMeshMap()
    {
		const auto allEntities = Application::Get<SceneSystem>()->GetAllRawEntities();
        auto& renderPassMap = _pIntermediateVariable->renderPassPipelineMeshesMap;
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

			const auto* pMaterial = materialRef.Get()->GetTargetMaterial();
            for (auto i = 0; i < EnumReflection<RenderPassType>::Size(); i++)
            {
                auto passType = static_cast<RenderPassType>(i);
                if (!pMaterial->HasRenderPass(passType))
                    continue;

				const auto& allMeshes = modelRef->GetMeshes();
				for (const auto& pMesh: allMeshes)
				{
					// Create a pipeline entry
					VulkanPipelineEntry pipelineEntry(
						passType,
						pMaterial->GetAssetId(),
						pMesh->GetVertexLayoutId());

					auto& meshList = renderPassMap[passType][pipelineEntry];
					for (const auto& mesh : modelRef.Get()->GetMeshes())
						meshList.push_back(mesh.get());
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

		auto& pipleMeshesMap = _pIntermediateVariable->renderPassPipelineMeshesMap[RenderPassType::Forward];
		if (pipleMeshesMap.empty())
			return;

		pCommandBuffer->BeginRenderPass(pForwardPass->GetRHIRenderPass());

		for (auto& [pipelienEntry, pMeshList] : pipleMeshesMap)
			RenderPipelineMeshes(pipelienEntry, pMeshList, pCommandBuffer);

		pCommandBuffer->EndRenderPass();
	}

	void RenderSystem::RenderPipelineMeshes(const VulkanPipelineEntry& pipelineEntry, const std::vector<const Mesh*>& meshList, VulkanCommandBuffer* pCommandBuffer)
	{
		auto pPipeline = Application::Get<VulkanSystem>()->GetPipelineManager()->GetPipeline(pipelineEntry);
		if (pPipeline == nullptr)
		{
			Logger::LogError("Pipeline not found for entry: {}", pipelineEntry.renderPass);
			return;
		}

		pCommandBuffer->BindPipeline(pPipeline);
		pCommandBuffer->SetViewportAndScissor();

		for (const auto* pMesh: meshList)
		{
			auto pVertexBuffer = pMesh->GetVertexBuffer();
			pCommandBuffer->BindVertexBuffer(pVertexBuffer);

			auto pIndexBuffer = pMesh->GetIndexBuffer();
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
} // namespace Ailurus