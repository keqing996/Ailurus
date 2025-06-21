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

// pass -> material(shader + uniform, descSetLayout) -> meshes

namespace Ailurus
{
	struct RenderIntermediateVariable
    {
		struct RenderInfoPerVertexLayout
		{
			std::vector<const Mesh*> meshes;
		};

		struct RenderInfoPerMaterial
		{
			Material* pMaterial;
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
            	auto& renderPassInfo = renderInfo.renderPassesMap[passType];

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

					auto& meshList = renderInfo[passType][pipelineEntry];
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

    	// Prepare
    	auto pDescriptorPool = Application::Get<VulkanSystem>()->GetFrameContext()->GetAllocatingDescriptorPool();

		pCommandBuffer->BeginRenderPass(pForwardPass->GetRHIRenderPass());

		for (auto& [pipelineEntry, pMeshList] : pipleMeshesMap)
		{
			const auto materialRef = Application::Get<AssetsSystem>()->GetAsset<Material>(pipelineEntry.materialAssetId);
			const auto pDescriptorSetLayout = materialRef->GetUniformSet(RenderPassType::Forward)->GetDescriptorSetLayout();

			auto descriptorSet = pDescriptorPool->AllocateDescriptorSet(pDescriptorSetLayout);

			RenderPipelineMeshes(pipelineEntry, pMeshList, pCommandBuffer);
		}

		pCommandBuffer->EndRenderPass();
	}

	void RenderSystem::RenderPipelineMeshes(const VulkanPipelineEntry& pipelineEntry, const std::vector<const Mesh*>& meshList, VulkanCommandBuffer* pCommandBuffer)
	{
		auto pVulkanSystem = Application::Get<VulkanSystem>();

		const auto pPipeline = pVulkanSystem->GetPipelineManager()->GetPipeline(pipelineEntry);
		if (pPipeline == nullptr)
		{
			Logger::LogError("Pipeline not found for entry: {}", pipelineEntry.renderPass);
			return;
		}

		pCommandBuffer->BindPipeline(pPipeline);
		pCommandBuffer->SetViewportAndScissor();



		for (const auto* pMesh: meshList)
		{
			// Prepare descriptor
			auto pDescriptorPool = pVulkanSystem->GetFrameContext()->GetAllocatingDescriptorPool();
			pDescriptorPool->AllocateDescriptorSet(pVertexLayout)

			// Draw
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
} // namespace Ailurus