#pragma once

#include <memory>
#include <unordered_map>
#include <vulkan/vulkan.hpp>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"
#include "Ailurus/Application/RenderSystem/RenderPass/RenderPassType.h"
#include "../Pipeline/VulkanPipeline.h"

namespace Ailurus
{
	class VulkanRenderPass : public NonCopyable, public NonMovable
	{
	public:
		virtual ~VulkanRenderPass();

	public:
		virtual RenderPassType GetRenderPassType() = 0;
		virtual vk::RenderPass GetRenderPass() const = 0;
		virtual vk::RenderPassBeginInfo GetRenderPassBeginInfo() const = 0;
		const VulkanPipeline* GetPipeline(const VulkanPipelineConfig& config);

	private:
		std::unordered_map<VulkanPipelineConfig, std::unique_ptr<VulkanPipeline>, VulkanPipelineConfig::Hash> _pipelineMap;
	};
} // namespace Ailurus