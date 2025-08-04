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
#include <VulkanContext/VulkanContext.h>
#include <VulkanContext/CommandBuffer/VulkanCommandBuffer.h>
#include <VulkanContext/Pipeline/VulkanPipelineManager.h>
#include <VulkanContext/RenderPass/VulkanRenderPass.h>
#include <VulkanContext/DataBuffer/VulkanVertexBuffer.h>
#include <VulkanContext/DataBuffer/VulkanIndexBuffer.h>
#include <VulkanContext/DataBuffer/VulkanUniformBuffer.h>
#include <VulkanContext/Vertex/VulkanVertexLayoutManager.h>
#include <Ailurus/Utility/Logger.h>
#include "Ailurus/Application/RenderSystem/Uniform/UniformSet.h"
#include "Detail/RenderIntermediateVariable.h"

namespace Ailurus
{
	void RenderSystem::RenderPrepare()
	{
		const Matrix4x4f projMat = _pMainCamera->GetProjectionMatrix();
		const Matrix4x4f viewMat = _pMainCamera->GetViewMatrix();
		_pIntermediateVariable->viewProjectionMatrix = projMat * viewMat;
		_pIntermediateVariable->renderingMeshes.clear();
		_pIntermediateVariable->materialInstanceDescriptorsMap.clear();
		_pIntermediateVariable->renderingDescriptorSets.fill(VulkanDescriptorSet{});
	}

	void RenderSystem::CollectRenderingContext()
	{
		auto& renderingMeshesMap = _pIntermediateVariable->renderingMeshes;
		renderingMeshesMap.clear();

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

					renderingMeshesMap[passType].push_back(RenderingMesh{
						pMaterial,
						pMaterialInstance,
						vertexLayoutId,
						pMesh.get(),
						pEntity });
				}
			}
		}

		// Opaque meshes sorted by material & material instance
		auto& forwardPassMeshes = renderingMeshesMap[RenderPassType::Forward];
		std::sort(forwardPassMeshes.begin(), forwardPassMeshes.end(),
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

		CollectRenderingContext();
		UpdateGlobalUniformBuffer();
		UpdateMaterialInstanceUniformBuffer();

		auto* pCommandBuffer = Application::Get<VulkanSystem>()->GetFrameContext()->GetRecordingCommandBuffer();

		RenderSpecificPass(RenderPassType::Forward, pCommandBuffer);

		Application::Get<VulkanSystem>()->RenderFrame(&_needRebuildSwapChain);
	}

	void RenderSystem::UpdateGlobalUniformBuffer()
	{
		_pGlobalUniformMemory->SetUniformValue(
			{ 0, GetGlobalUniformAccessNameViewProjMat() },
			_pIntermediateVariable->viewProjectionMatrix);

		_pGlobalUniformMemory->SetUniformValue(
			{ 0, GetGlobalUniformAccessNameCameraPos() },
			_pMainCamera->GetEntity()->GetPosition());

		// Allocate descriptor set
		auto globalUniformSetLayout = _pGlobalUniformSet->GetDescriptorSetLayout();
		auto globalDescriptorSet = Application::Get<VulkanSystem>()->GetFrameContext()->GetAllocatingDescriptorPool()->AllocateDescriptorSet(globalUniformSetLayout);

		// Save set
		_pIntermediateVariable->renderingDescriptorSets[static_cast<int>(UniformSetUsage::General)] = globalDescriptorSet;

		// Write the descriptor set
		_pGlobalUniformMemory->UpdateToDescriptorSet(globalDescriptorSet);
	}

	void RenderSystem::UpdateMaterialInstanceUniformBuffer()
	{
		auto* pDescriptorPool = Application::Get<VulkanSystem>()->GetFrameContext()->GetAllocatingDescriptorPool();

		auto& opaqueMeshes = _pIntermediateVariable->renderingMeshes;
		for (auto& [pass, meshes] : opaqueMeshes)
		{
			auto& mapInstDescriptorSetMap = _pIntermediateVariable->materialInstanceDescriptorsMap[pass];
			for (auto& pRenderingMesh : meshes)
			{
				const auto* pMaterial = pRenderingMesh.pMaterial;
				const auto* pMaterialInstance = pRenderingMesh.pMaterialInstance;

				if (mapInstDescriptorSetMap.contains(pMaterialInstance))
					continue;

				// Allocate a set by layout.
				auto* pDescriptorLayout = pMaterial->GetUniformSet(pass)->GetDescriptorSetLayout();
				auto descriptorSet = pDescriptorPool->AllocateDescriptorSet(pDescriptorLayout);

				// Update material data to descriptor set
				pMaterialInstance->GetUniformSetMemory(pass)->UpdateToDescriptorSet(descriptorSet);

				// Record material instance descriptor set
				mapInstDescriptorSetMap[pMaterialInstance] = descriptorSet;
			}
		}
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

		// Intermidiate variables
		const Material* pCurrentMaterial = nullptr;
		const MaterialInstance* pCurrentMaterialInstance = nullptr;
		uint64_t currentVertexLayoutId = 0;

		// Pipeline variables
		VulkanDescriptorSetLayout* pCurrentVkSetLayout;
		VulkanPipeline* pCurrentVkPipeline;

		for (const auto& renderingMesh : _pIntermediateVariable->renderingMeshes[pass])
		{
			if (renderingMesh.pMaterial != pCurrentMaterial)
			{
				pCurrentMaterial = renderingMesh.pMaterial;

				// Reset the material instance and the vertex layout
				pCurrentMaterialInstance = nullptr;
				currentVertexLayoutId = 0;
			}

			if (renderingMesh.pMaterialInstance != pCurrentMaterialInstance)
			{
				pCurrentMaterialInstance = renderingMesh.pMaterialInstance;

				_pIntermediateVariable->renderingDescriptorSets[static_cast<int>(UniformSetUsage::MaterialCustom)] =
					_pIntermediateVariable->materialInstanceDescriptorsMap[pass][pCurrentMaterialInstance];
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