#include <algorithm>
#include <cstdint>
#include <array>
#include <Ailurus/Utility/EnumReflection.h>
#include <Ailurus/Utility/Logger.h>
#include <Ailurus/Application/Application.h>
#include <Ailurus/Application/RenderSystem/RenderSystem.h>
#include <Ailurus/Application/RenderSystem/Uniform/UniformSet.h>
#include <Ailurus/Application/RenderSystem/Uniform/UniformBindingPoint.h>
#include <Ailurus/Application/RenderSystem/Uniform/UniformSetMemory.h>
#include <Ailurus/Application/AssetsSystem/Mesh/Mesh.h>
#include <Ailurus/Application/AssetsSystem/Model/Model.h>
#include <Ailurus/Application/SceneSystem/SceneSystem.h>
#include <Ailurus/Application/SceneSystem/Component/CompStaticMeshRender.h>
#include <VulkanContext/VulkanContext.h>
#include <VulkanContext/SwapChain/VulkanSwapChain.h>
#include <VulkanContext/RenderTarget/RenderTargetManager.h>
#include <VulkanContext/CommandBuffer/VulkanCommandBuffer.h>
#include <VulkanContext/Pipeline/VulkanPipelineManager.h>
#include <VulkanContext/DataBuffer/VulkanVertexBuffer.h>
#include <VulkanContext/DataBuffer/VulkanIndexBuffer.h>
#include <VulkanContext/DataBuffer/VulkanUniformBuffer.h>
#include <VulkanContext/Vertex/VulkanVertexLayoutManager.h>
#include <VulkanContext/Descriptor/VulkanDescriptorAllocator.h>
#include <VulkanContext/Descriptor/VulkanDescriptorSetLayout.h>
#include "Ailurus/Application/ImGuiSystem/ImGuiSystem.h"
#include "Ailurus/Application/RenderSystem/RenderPass/RenderPassType.h"
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
		_pIntermediateVariable->renderingDescriptorSets.fill(vk::DescriptorSet{});
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

	void RenderSystem::CheckRebuildSwapChain()
	{
		if (_needRebuildSwapChain)
			RebuildSwapChain();
	}

	void RenderSystem::RenderScene()
	{
		VulkanContext::RenderFrame(&_needRebuildSwapChain,
			[this](uint32_t swapChainImageIndex, VulkanCommandBuffer* pCommandBuffer, VulkanDescriptorAllocator* pDescriptorAllocator) -> void {
				RenderPrepare();

				CollectRenderingContext();
				UpdateGlobalUniformBuffer(pCommandBuffer, pDescriptorAllocator);
				UpdateMaterialInstanceUniformBuffer(pCommandBuffer, pDescriptorAllocator);

				// Get swap chain for image layout transitions
				auto pSwapChain = VulkanContext::GetSwapChain();
				const auto& swapChainImages = pSwapChain->GetSwapChainImages();
				vk::Image currentImage = swapChainImages[swapChainImageIndex];

				// Transition image to color attachment optimal before rendering
				pCommandBuffer->ImageMemoryBarrier(
					currentImage,
					vk::ImageLayout::eUndefined,
					vk::ImageLayout::eColorAttachmentOptimal,
					vk::AccessFlags{},
					vk::AccessFlagBits::eColorAttachmentWrite,
					vk::PipelineStageFlagBits::eTopOfPipe,
					vk::PipelineStageFlagBits::eColorAttachmentOutput);

				// Forward pass
				if (_enable3D)
					RenderPass(RenderPassType::Forward, swapChainImageIndex, pCommandBuffer);

				// ImGui pass
				if (_enableImGui)
					RenderImGuiPass(swapChainImageIndex, pCommandBuffer);

				// Transition image to present layout after rendering
				pCommandBuffer->ImageMemoryBarrier(
					currentImage,
					vk::ImageLayout::eColorAttachmentOptimal,
					vk::ImageLayout::ePresentSrcKHR,
					vk::AccessFlagBits::eColorAttachmentWrite,
					vk::AccessFlags{},
					vk::PipelineStageFlagBits::eColorAttachmentOutput,
					vk::PipelineStageFlagBits::eBottomOfPipe);
			});
	}

	void RenderSystem::UpdateGlobalUniformBuffer(VulkanCommandBuffer* pCommandBuffer, VulkanDescriptorAllocator* pDescriptorAllocator)
	{
		_pGlobalUniformMemory->SetUniformValue(
			{ 0, GetGlobalUniformAccessNameViewProjMat() },
			_pIntermediateVariable->viewProjectionMatrix);

		_pGlobalUniformMemory->SetUniformValue(
			{ 0, GetGlobalUniformAccessNameCameraPos() },
			_pMainCamera->GetEntity()->GetPosition());

		// Use cache to get or allocate descriptor set
		// Key: layout only (global uniform always uses same layout)
		auto globalUniformSetLayout = _pGlobalUniformSet->GetDescriptorSetLayout();
		
		VulkanDescriptorAllocator::CacheKey key;
		key.layout = globalUniformSetLayout->GetDescriptorSetLayout();
		key.bindingHash = 1; // Fixed hash for global uniform (to distinguish from material)
		
		auto globalDescriptorSet = pDescriptorAllocator->AllocateDescriptorSet(globalUniformSetLayout, &key);

		// Save set
		_pIntermediateVariable->renderingDescriptorSets[static_cast<int>(UniformSetUsage::General)] = globalDescriptorSet;

		// IMPORTANT: Always update descriptor set data (buffer content changes each frame)
		_pGlobalUniformMemory->UpdateToDescriptorSet(pCommandBuffer, globalDescriptorSet);
	}

	void RenderSystem::UpdateMaterialInstanceUniformBuffer(VulkanCommandBuffer* pCommandBuffer, VulkanDescriptorAllocator* pDescriptorAllocator)
	{
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

				// Use cache to get or allocate descriptor set
				auto* pDescriptorLayout = pMaterial->GetUniformSet(pass)->GetDescriptorSetLayout();
				
				// Create cache key based on layout and material instance
				VulkanDescriptorAllocator::CacheKey key;
				key.layout = pDescriptorLayout->GetDescriptorSetLayout();
				key.bindingHash = reinterpret_cast<size_t>(pMaterialInstance); // Use material instance pointer as hash
				
				auto descriptorSet = pDescriptorAllocator->AllocateDescriptorSet(pDescriptorLayout, &key);

				// Update material data to descriptor set (pass material instance for texture binding)
				pMaterialInstance->GetUniformSetMemory(pass)->UpdateToDescriptorSet(pCommandBuffer, descriptorSet, pMaterialInstance);

				// Record material instance descriptor set
				mapInstDescriptorSetMap[pMaterialInstance] = descriptorSet;
			}
		}
	}

	void RenderSystem::RenderPass(RenderPassType pass, uint32_t swapChainImageIndex, VulkanCommandBuffer* pCommandBuffer)
	{
		if (_pIntermediateVariable->renderingMeshes.empty())
			return;

		// Get swap chain image view and depth image view
		auto pSwapChain = VulkanContext::GetSwapChain();
		if (pSwapChain == nullptr)
		{
			Logger::LogError("SwapChain not found");
			return;
		}

		const auto& imageViews = pSwapChain->GetSwapChainImageViews();
		if (swapChainImageIndex >= imageViews.size())
		{
			Logger::LogError("Invalid swap chain image index");
			return;
		}

		vk::Extent2D extent = pSwapChain->GetConfig().extent;

		// Check if MSAA is enabled
		bool useMSAA = VulkanContext::GetMSAASamples() != vk::SampleCountFlagBits::e1;
		auto* pRenderTargetManager = VulkanContext::GetRenderTargetManager();

		if (useMSAA)
		{
			// With MSAA: render to MSAA attachments and resolve to swapchain image
			vk::ImageView msaaColorView = pRenderTargetManager->GetMSAAColorImageView();
			vk::ImageView msaaDepthView = pRenderTargetManager->GetMSAADepthImageView();
			vk::ImageView resolveView = imageViews[swapChainImageIndex];
			
			// For Forward pass, clear and use depth
			bool clearColor = (pass == RenderPassType::Forward);
			pCommandBuffer->BeginRendering(msaaColorView, msaaDepthView, resolveView, extent, clearColor, true);
		}
		else
		{
			// Without MSAA: render directly to swapchain image
			vk::ImageView colorImageView = imageViews[swapChainImageIndex];
			vk::ImageView depthImageView = pRenderTargetManager->GetDepthImageView();
			
			bool clearColor = (pass == RenderPassType::Forward);
			pCommandBuffer->BeginRendering(colorImageView, depthImageView, nullptr, extent, clearColor, true);
		}

		// Intermediate variables
		const Material* pCurrentMaterial = nullptr;
		const MaterialInstance* pCurrentMaterialInstance = nullptr;
		uint64_t currentVertexLayoutId = 0;

		// Pipeline variables
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
				pCurrentVkPipeline = VulkanContext::GetPipelineManager()->GetPipeline(pipelineEntry);
				if (pCurrentVkPipeline == nullptr)
				{
					Logger::LogError("Pipeline not found for entry: {}", EnumReflection<RenderPassType>::ToString(pipelineEntry.renderPass));
					continue;
				}

				pCommandBuffer->BindPipeline(pCurrentVkPipeline);
				pCommandBuffer->SetViewportAndScissor();

				std::vector<vk::DescriptorSet> descriptorSets(_pIntermediateVariable->renderingDescriptorSets.begin(),
				                                               _pIntermediateVariable->renderingDescriptorSets.end());

				pCommandBuffer->BindDescriptorSet(pCurrentVkPipeline->GetPipelineLayout(), descriptorSets);
			}

			// Push constant model matrix
			pCommandBuffer->PushConstantModelMatrix(pCurrentVkPipeline, renderingMesh.pEntity->GetModelMatrix());

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

		pCommandBuffer->EndRendering();
	}

	void RenderSystem::RenderImGuiPass(uint32_t swapChainImageIndex, VulkanCommandBuffer* pCommandBuffer)
	{
		auto pImGui = Application::Get<ImGuiSystem>();

		// Get swap chain image view and depth image view
		auto pSwapChain = VulkanContext::GetSwapChain();
		if (pSwapChain == nullptr)
		{
			Logger::LogError("SwapChain not found");
			return;
		}

		const auto& imageViews = pSwapChain->GetSwapChainImageViews();
		if (swapChainImageIndex >= imageViews.size())
		{
			Logger::LogError("Invalid swap chain image index");
			return;
		}

		vk::ImageView colorImageView = imageViews[swapChainImageIndex];
		vk::Extent2D extent = pSwapChain->GetConfig().extent;

		// ImGui doesn't need depth or MSAA, and should load existing content (clearColor=false)
		// Always render directly to swapchain image (no resolve needed)
		pCommandBuffer->BeginRendering(colorImageView, nullptr, nullptr, extent, false, false);
		pImGui->Render(pCommandBuffer);
		pCommandBuffer->EndRendering();
	}
} // namespace Ailurus