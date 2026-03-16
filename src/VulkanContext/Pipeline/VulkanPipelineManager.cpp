#include <Ailurus/Utility/Logger.h>
#include <Ailurus/Application.h>
#include <Ailurus/Systems/RenderSystem/RenderSystem.h>
#include <Ailurus/Systems/AssetsSystem/AssetsSystem.h>
#include <Ailurus/Systems/AssetsSystem/Material/Material.h>
#include <Ailurus/Math/Matrix4x4.hpp>
#include "VulkanPipelineManager.h"
#include "VulkanContext/VulkanContext.h"
#include "VulkanContext/Vertex/VulkanVertexLayoutManager.h"
#include "VulkanContext/SwapChain/VulkanSwapChain.h"
#include "VulkanContext/RenderTarget/RenderTargetManager.h"

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

		const bool isShadowPass   = (entry.renderPass == RenderPassType::Shadow);
		const bool isGBufferPass  = (entry.renderPass == RenderPassType::GBuffer);
		const bool isTransparent  = (entry.renderPass == RenderPassType::Transparent);

		const vk::Format depthFormat = vk::Format::eD32Sfloat;

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

		// Collect uniform sets
		std::vector<const UniformSet*> uniformSets;
		uniformSets.push_back(Application::Get<RenderSystem>()->GetGlobalUniformSet());

		auto* materialUniformSet = refMaterial->GetUniformSet(entry.renderPass);
		if (materialUniformSet != nullptr)
			uniformSets.push_back(materialUniformSet);

		VulkanPipeline* pPipeline = nullptr;

		if (isGBufferPass)
		{
			// G-Buffer pipeline: 3 color attachments (Normal+AO, Albedo+Roughness, Metallic)
			std::vector<vk::Format> colorFormats = {
				RenderTargetManager::GetGBufferNormalFormat(),
				RenderTargetManager::GetGBufferAlbedoFormat(),
				RenderTargetManager::GetGBufferMetallicFormat()
			};
			pPipeline = new VulkanPipeline(colorFormats, depthFormat, *pShaderArray, pVertexLayout, uniformSets);
		}
		else if (isShadowPass)
		{
			// Depth-only pipeline
			const uint32_t pushConstantSize = static_cast<uint32_t>(sizeof(Matrix4x4f) + sizeof(uint32_t));
			pPipeline = new VulkanPipeline(vk::Format::eUndefined, depthFormat, *pShaderArray, pVertexLayout, uniformSets, pushConstantSize);
		}
		else if (isTransparent)
		{
			// Transparent pipeline: alpha blending, depth test (read-only)
			const vk::Format colorFormat = vk::Format::eR16G16B16A16Sfloat;
			pPipeline = new VulkanPipeline(colorFormat, depthFormat, *pShaderArray, pVertexLayout, uniformSets,
				static_cast<uint32_t>(sizeof(Matrix4x4f)),
				/*blendEnabled=*/true,
				/*depthWriteEnabled=*/false);
		}
		else
		{
			// Standard forward pass pipeline
			const vk::Format colorFormat = vk::Format::eR16G16B16A16Sfloat;
			pPipeline = new VulkanPipeline(colorFormat, depthFormat, *pShaderArray, pVertexLayout, uniformSets);
		}

		_pipelinesMap[entry] = std::unique_ptr<VulkanPipeline>(pPipeline);
		return pPipeline;
	}
} // namespace Ailurus
