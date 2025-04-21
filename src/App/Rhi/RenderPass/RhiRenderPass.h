#pragma once

#include <memory>
#include <unordered_map>
#include <vulkan/vulkan.hpp>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Application/RenderPass/RenderPassType.h"
#include "Rhi/Pipeline/RHIPipeline.h"

namespace Ailurus
{
	class RhiRenderPass : public NonCopyable
	{
	public:
		virtual RenderPassType GetRenderPassType() = 0;
		virtual vk::RenderPass GetRenderPass() const = 0;
		virtual vk::RenderPassBeginInfo GetRenderPassBeginInfo() const = 0;
		const RhiPipeline* GetPipeline(const RhiPipelineConfig& config);

	private:
		std::unordered_map<RhiPipelineConfig, std::unique_ptr<RhiPipeline>, RhiPipelineConfig::Hash> _pipelineMap;
	};
} // namespace Ailurus