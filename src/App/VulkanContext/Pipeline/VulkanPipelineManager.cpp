#include <Ailurus/Utility/Logger.h>
#include <Ailurus/Application/Application.h>
#include <Ailurus/Application/RenderSystem/RenderSystem.h>
#include <Ailurus/Application/AssetsSystem/AssetsSystem.h>
#include <Ailurus/Application/AssetsSystem/Material/Material.h>
#include "VulkanPipelineManager.h"
#include "VulkanContext/VulkanContext.h"
#include "VulkanContext/Vertex/VulkanVertexLayoutManager.h"
#include "VulkanContext/SwapChain/VulkanSwapChain.h"

namespace Ailurus
{
	auto VulkanPipelineManager::GetPipeline(const VulkanPipelineEntry& entry) -> VulkanPipeline*
	{
		const auto it = _pipelinesMap.find(entry);
		if (it != _pipelinesMap.end())
			return it->second.get();

		// Get formats from swap chain
		auto pSwapChain = VulkanContext::GetSwapChain();
		if (pSwapChain == nullptr)
		{
			Logger::LogError("VulkanPipelineManager::GetPipeline: SwapChain not found");
			return nullptr;
		}

		const vk::Format colorFormat = pSwapChain->GetConfig().surfaceFormat.format;
		const vk::Format depthFormat = vk::Format::eD32Sfloat; // Standard depth format

		auto refMaterial = Application::Get<AssetsSystem>()->GetAsset<Material>(entry.materialAssetId);
		if (!refMaterial)
		{
			Logger::LogError("VulkanPipelineManager::GetPipeline: Material not found for entry: {}", entry.materialAssetId);
			return nullptr;
		}

		auto pShaderArray = refMaterial->GetPassShaderArray(entry.renderPass);
		if (pShaderArray == nullptr)
		{
			Logger::LogError("VulkanPipelineManager::GetPipeline: Shader array not found for material {} in render pass {}", 
				entry.materialAssetId, EnumReflection<RenderPassType>::ToString(entry.renderPass));
			return nullptr;
		}

		auto pVertexLayout = VulkanContext::GetVertexLayoutManager()->GetLayout(entry.vertexLayoutId);
		if (pVertexLayout == nullptr)
		{
			Logger::LogError("VulkanPipelineManager::GetPipeline: Vertex layout not found for entry: {}", entry.vertexLayoutId);
			return nullptr;
		}

		// Create the pipeline
		std::vector<const UniformSet*> uniformSets;
		uniformSets.push_back(Application::Get<RenderSystem>()->GetGlobalUniformSet());
		uniformSets.push_back(refMaterial->GetUniformSet(entry.renderPass));

		const auto pPipeline = new VulkanPipeline(
			colorFormat,
			depthFormat,
			*pShaderArray,
			pVertexLayout, 
			uniformSets);

		_pipelinesMap[entry] = std::unique_ptr<VulkanPipeline>(pPipeline);

		return pPipeline;
	}
} // namespace Ailurus
