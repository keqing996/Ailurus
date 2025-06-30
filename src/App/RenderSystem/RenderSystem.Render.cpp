#include <algorithm>
#include <cstdint>
#include <array>
#include <Ailurus/Application/RenderSystem/RenderPass/RenderPass.h>
#include <Ailurus/Utility/EnumReflection.h>
#include <Ailurus/Application/Application.h>
#include <Ailurus/Application/RenderSystem/RenderSystem.h>
#include <Ailurus/Application/RenderSystem/Uniform/UniformBindingPoint.h>
#include <Ailurus/Application/RenderSystem/Uniform/UniformSetMemory.h>
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
	void RenderSystem::RenderPrepare()
	{
		const auto& projMat = _pMainCamera->GetProjectionMatrix();
		const auto& viewMat = _pMainCamera->GetEntity()->GetModelMatrix();
		_pIntermediateVariable->viewProjectionMatrix = viewMat * projMat;
		_pIntermediateVariable->renderingMeshes.clear();
		_pIntermediateVariable->renderingDescriptorSets.fill(VulkanDescriptorSet{});
	}

	void RenderSystem::CollectOpaqueRenderingObject()
	{
		auto& renderingMeshes = _pIntermediateVariable->renderingMeshes;
		renderingMeshes.clear();

		const auto allEntities = Application::Get<SceneSystem>()->GetAllRawEntities();
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
			const auto* pMaterialInstance = materialInstRef.Get();
			for (auto i = 0; i < EnumReflection<RenderPassType>::Size(); i++)
			{
				auto passType = static_cast<RenderPassType>(i);
				if (!pMaterial->HasRenderPass(passType))
					continue;

				const auto& allMeshes = modelRef->GetMeshes();
				for (const auto& pMesh : allMeshes)
				{
					const auto vertexLayoutId = pMesh->GetVertexLayoutId();

					renderingMeshes.push_back(RenderingMesh{
						pMaterial,
						pMaterialInstance,
						vertexLayoutId,
						pMesh.get(),
						pEntity });
				}
			}
		}

		std::sort(renderingMeshes.begin(), renderingMeshes.end(),
			[](const RenderingMesh& lhs, const RenderingMesh& rhs) -> bool {
				// Compare by Material
				if (lhs.pMaterial != rhs.pMaterial)
					return lhs.pMaterial < rhs.pMaterial;

				// Compare by MaterialInstance
				if (lhs.pMaterialInstance != rhs.pMaterialInstance)
					return lhs.pMaterialInstance < rhs.pMaterialInstance;

				// Compare by vertexLayoutId
				return lhs.vertexLayoutId < rhs.vertexLayoutId;
			});
	}

	void RenderSystem::CreateIntermediateVariable()
	{
		_pIntermediateVariable = std::make_unique<RenderIntermediateVariable>();
	}

	void RenderSystem::RenderScene()
	{
		if (_needRebuildSwapChain)
			ReBuildSwapChain();

		RenderPrepare();

		UpdateGlobalUniformBuffer();

		auto* pCommandBuffer = Application::Get<VulkanSystem>()->GetFrameContext()->GetRecordingCommandBuffer();

		CollectOpaqueRenderingObject();
		RenderSpecificPass(RenderPassType::Forward, pCommandBuffer);

		Application::Get<VulkanSystem>()->RenderFrame(&_needRebuildSwapChain);
	}

	void RenderSystem::UpdateGlobalUniformBuffer()
	{
		_pGlobalUniformMemory->SetUniformValue(
			{ 0, GetGlobalUniformAccessNameViewProjMat() },
			_pIntermediateVariable->viewProjectionMatrix);

		_pGlobalUniformMemory->SetUniformValue(
			{ 1, GetGlobalUniformAccessNameCameraPos() },
			_pMainCamera->GetEntity()->GetPosition());

		// Allocate descriptor set
		auto globalUniformSetLayout = _pGlobalUniformSet->GetDescriptorSetLayout();
		auto globalDescriptorSet = Application::Get<VulkanSystem>()->GetFrameContext()->GetAllocatingDescriptorPool()->AllocateDescriptorSet(globalUniformSetLayout);

		// Save set
		_pIntermediateVariable->renderingDescriptorSets[static_cast<int>(UniformSetUsage::General)] = globalDescriptorSet;

		// Write descriptor set
		_pGlobalUniformMemory->UpdateToDescriptorSet(globalDescriptorSet);
	}

	void RenderSystem::RenderSpecificPass(RenderPassType pass, VulkanCommandBuffer* pCommandBuffer)
	{
		if (_pIntermediateVariable->renderingMeshes.empty())
			return;

		auto pRenderPass = GetRenderPass(pass);
		if (pRenderPass == nullptr)
			return;

		pCommandBuffer->BeginRenderPass(pRenderPass->GetRHIRenderPass());

		// Prepare
		auto pVulkanSystem = Application::Get<VulkanSystem>();
		auto pDescriptorPool = pVulkanSystem->GetFrameContext()->GetAllocatingDescriptorPool();

		// Intermidiate variables
		const Material* pCurrentMaterial = nullptr;
		const MaterialInstance* pCurrentMaterialInstance = nullptr;
		uint64_t currentVertexLayoutId = 0;

		// Pipeline variables
		VulkanDescriptorSetLayout* pCurrentVkSetLayout;
		VulkanPipeline* pCurrentVkPipeline;

		for (const auto& renderingMesh : _pIntermediateVariable->renderingMeshes)
		{
			if (renderingMesh.pMaterial != pCurrentMaterial)
			{
				pCurrentMaterial = renderingMesh.pMaterial;

				// Reset material instace and vertex layout
				pCurrentMaterialInstance = nullptr;
				currentVertexLayoutId = 0;

				// Allocate descriptor set
				auto pUniformSet = pCurrentMaterial->GetUniformSet(pass);
				pCurrentVkSetLayout = pUniformSet->GetDescriptorSetLayout();
				_pIntermediateVariable->renderingDescriptorSets[static_cast<int>(UniformSetUsage::MaterialCustom)] = pDescriptorPool->AllocateDescriptorSet(pCurrentVkSetLayout);
			}

			if (renderingMesh.pMaterialInstance != pCurrentMaterialInstance)
			{
				pCurrentMaterialInstance = renderingMesh.pMaterialInstance;

				// Update descriptor set memory
				pCurrentMaterialInstance->GetUniformSetMemory(pass)->UpdateToDescriptorSet(
					_pIntermediateVariable->renderingDescriptorSets[static_cast<int>(UniformSetUsage::MaterialCustom)]);
			}

			if (currentVertexLayoutId != renderingMesh.vertexLayoutId)
			{
				currentVertexLayoutId = renderingMesh.vertexLayoutId;

				// Get vulkan pipeline
				VulkanPipelineEntry pipelineEntry(pass, pCurrentMaterial->GetAssetId(), currentVertexLayoutId);
				pCurrentVkPipeline = pVulkanSystem->GetPipelineManager()->GetPipeline(pipelineEntry);
				if (pCurrentVkPipeline == nullptr)
				{
					Logger::LogError("Pipeline not found for entry: {}", EnumReflection<RenderPassType>::ToString(pipelineEntry.renderPass));
					continue;
				}

				pCommandBuffer->BindPipeline(pCurrentVkPipeline);
				pCommandBuffer->SetViewportAndScissor();

				std::vector<vk::DescriptorSet> descriptorSets;
				for (const auto& descriptorSet : _pIntermediateVariable->renderingDescriptorSets)
					descriptorSets.push_back(descriptorSet.descriptorSet);

				pCommandBuffer->BindDescriptorSet(pCurrentVkPipeline->GetPipelineLayout(), descriptorSets);
			}

			// Push constant model matrix
			pCommandBuffer->PushConstantModelMaterix(pCurrentVkPipeline, renderingMesh.pEntity->GetModelMatrix());

			// Bind vertex buffer
			const auto pVertexBuffer = renderingMesh.pTargetMesh->GetVertexBuffer();
			pCommandBuffer->BindVertexBuffer(pVertexBuffer);

			// Bind index buffer and draw
			const auto pIndexBuffer = renderingMesh.pTargetMesh->GetIndexBuffer();
			if (pIndexBuffer != nullptr)
			{
				pCommandBuffer->BindIndexBuffer(pIndexBuffer);
				pCommandBuffer->DrawIndexed(pIndexBuffer->GetIndexCount());
			}
			else
			{
				pCommandBuffer->DrawNonIndexed(renderingMesh.pTargetMesh->GetVertexCount());
			}
		}

		pCommandBuffer->EndRenderPass();
	}
} // namespace Ailurus