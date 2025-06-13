#include "Ailurus/Utility/EnumReflection.h"
#include <Ailurus/Application/Application.h>
#include <Ailurus/Application/RenderSystem/RenderSystem.h>
#include <Ailurus/Application/SceneSystem/SceneSystem.h>
#include <Ailurus/Application/SceneSystem/Component/CompStaticMeshRender.h>
#include <VulkanSystem/VulkanSystem.h>
#include <VulkanSystem/Pipeline/VulkanPipelineConfig.h>
#include <VulkanSystem/RenderPass/VulkanRenderPass.h>
#include <Ailurus/Utility/Logger.h>
#include <Ailurus/Assert.h>

namespace Ailurus
{
    struct RenderIntermidiateVariable
    {
        using MaterialMeshMap = std::unordered_map<const MaterialInstance*, std::vector<const Mesh*>>;
        using RenderPassObjectMap = std::unordered_map<RenderPassType, MaterialMeshMap>;

        Matrix4x4f projMatrix;
        Matrix4x4f viewMatrix;
        RenderPassObjectMap renderPassObjectMap;
    };

    void RenderSystem::CollectCameraViewProjectionMatrix()
    {
		_pIntermidiateVariable->projMatrix = _pMainCamera->GetProjectionMatrix();
		_pIntermidiateVariable->viewMatrix = _pMainCamera->GetEntity()->GetModelMatrix();
    }

    void RenderSystem::CollectMaterialMeshMap()
    {
        auto allEntities = Application::Get<SceneSystem>()->GetAllRawEntities();
        auto& renderPassObjMap = _pIntermidiateVariable->renderPassObjectMap;
        for (const auto pEntity : allEntities)
        {
            auto pMeshRender = pEntity->GetComponent<CompStaticMeshRender>();
            if (pMeshRender == nullptr)
                continue;

            const auto& modelRef = pMeshRender->GetModelAsset();
            if (!modelRef)
                continue;

            const auto& materialRef = pMeshRender->GetMaterialAsset();
            if (!materialRef)
                continue;

            auto* pMaterial = materialRef.Get()->GetTargetMaterial();
            for (auto i = 0; i < EnumReflection<RenderPassType>::Size(); i++)
            {
                auto passType = static_cast<RenderPassType>(i);
                if (!pMaterial->HasRenderPass(passType))
                    continue;

                auto& meshList  = renderPassObjMap[passType][materialRef.Get()];
                for (const auto& mesh : modelRef.Get()->GetMeshes())
                    meshList.push_back(mesh.get());
            }
        }
    }

	void RenderSystem::CreateIntermidiateVariable()
    {
        _pIntermidiateVariable = std::make_unique<RenderIntermidiateVariable>();
    }

	void RenderSystem::RenderScene()
	{
		if (_needRebuildSwapChain)
			ReBuildSwapChain();

        CollectCameraViewProjectionMatrix();
        CollectMaterialMeshMap();

		RenderForwardPass();

		Application::Get<VulkanSystem>()->RenderFrame(&_needRebuildSwapChain);
	}

	void RenderSystem::RenderForwardPass()
	{
        ASSERT_MSG(_pCurrentRenderPass == nullptr, "Render pass already exists, cannot render again");

        auto& meshRenderList = _pIntermidiateVariable->renderPassObjectMap[RenderPassType::Forward];
        if (meshRenderList.empty())
            return;

		_pCurrentRenderPass = _renderPassMap[RenderPassType::Forward].get();

		{
			VulkanCommandBuffer* pCommandBuffer = Application::Get<VulkanSystem>()->GetFrameContext()->GetRecordingCommandBuffer();
			VulkanCommandBufferRenderPassRecordScope renderPassRecordScope(pCommandBuffer, _pCurrentRenderPass);

			for (const auto pMeshRender : meshRenderList)
				RenderMesh(pMeshRender, pCommandBuffer);
		}

		_pCurrentRenderPass = nullptr;
	}

	void RenderSystem::RenderMesh(const CompStaticMeshRender* pMeshRender, VulkanCommandBuffer* pCommandBuffer) const
	{
		if (pMeshRender == nullptr)
			return;

		if (_pCurrentRenderPass == nullptr)
		{
			Logger::LogError("Command buffer draw object but not in any render pass");
			return;
		}

		const auto commandBuffer = pCommandBuffer->GetBuffer();
		const auto& modelRef = pMeshRender->GetModelAsset();
		if (modelRef == nullptr)
			return;

		const auto& allModelMeshs = modelRef.Get()->GetMeshes();
		if (allModelMeshs.empty())
			return;

		const auto pMaterial = pMeshRender->GetMaterial();

		if (pMesh == nullptr || pMaterial == nullptr)
			return;

		auto optStageShaders = pMaterial->GetStageShaderArray(_pCurrentRenderPass->GetRenderPassType());
		if (!optStageShaders.has_value())
			return; // This object should not be drawn under this pass;

		// Bind pipeline
		VulkanPipelineConfig pipelineConfig;
		pipelineConfig.pMesh = pMesh;
		pipelineConfig.shaderStages = optStageShaders.value();
		vk::Pipeline pipeline = _pCurrentRenderPass->GetRHIRenderPass()->GetPipeline(pipelineConfig)->GetPipeline();
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

		// Set viewport & scissor
		auto extent = Application::Get<VulkanSystem>()->GetSwapChainConfig().extent;
		vk::Viewport viewport(0.0f, 0.0f, extent.width, extent.height, 0.0f, 1.0f);
		vk::Rect2D scissor(vk::Offset2D{ 0, 0 }, extent);
		commandBuffer.setViewport(0, 1, &viewport);
		commandBuffer.setScissor(0, 1, &scissor);

		// Bind vertex buffer
		vk::Buffer vertexBuffers[] = { pMesh->GetVertexBuffer()->GetBuffer() };
		vk::DeviceSize offsets[] = { 0 };
		commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

		// Draw by index
		auto pIndexBuffer = pMesh->GetIndexBuffer();
		if (pIndexBuffer != nullptr)
		{
			commandBuffer.bindIndexBuffer(pIndexBuffer->GetBuffer(), 0, pIndexBuffer->GetIndexType());
			commandBuffer.drawIndexed(pIndexBuffer->GetIndexCount(), 1, 0, 0, 0);
		}
		// Draw by vertex
		else
		{
			commandBuffer.draw(pMesh->GetVertexCount(), 1, 0, 0);
		}
	}
} // namespace Ailurus