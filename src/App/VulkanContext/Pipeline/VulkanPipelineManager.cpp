#include <Ailurus/Utility/Logger.h>
#include <Ailurus/Application/Application.h>
#include <Ailurus/Application/RenderSystem/RenderSystem.h>
#include <Ailurus/Application/RenderSystem/RenderPass/RenderPass.h>
#include <Ailurus/Application/AssetsSystem/AssetsSystem.h>
#include <Ailurus/Application/AssetsSystem/Material/Material.h>
#include "VulkanPipelineManager.h"
#include "VulkanContext/VulkanContext.h"
#include "VulkanContext/Vertex/VulkanVertexLayoutManager.h"

namespace Ailurus
{
	auto VulkanPipelineManager::GetPipeline(const VulkanPipelineEntry& entry) -> VulkanPipeline*
	{
		const auto it = _pipelinesMap.find(entry);
		if (it != _pipelinesMap.end())
			return it->second.get();

		// Prepare to create a new pipeline
		auto pRenderPass = Application::Get<RenderSystem>()->GetRenderPass(entry.renderPass);
		if (pRenderPass == nullptr)
		{
			Logger::LogError("VulkanPipelineManager::GetPipeline: Render pass not found for entry: {}",
				EnumReflection<RenderPassType>::ToString(entry.renderPass));
			return nullptr;
		}

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
			pRenderPass->GetRHIRenderPass(), 
			*pShaderArray,
			pVertexLayout, 
			uniformSets);

		_pipelinesMap[entry] = std::unique_ptr<VulkanPipeline>(pPipeline);

		return pPipeline;
	}
} // namespace Ailurus
