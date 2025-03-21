#pragma once

#include <unordered_map>
#include <vulkan/vulkan.hpp>
#include "Ailurus/Utility/NonCopyable.h"
#include "Vulkan/Airport/Flight.h"
#include "Vulkan/Pipeline/RHIPipeline.h"

namespace Ailurus
{
	class RHIRenderPass : public NonCopyable
	{
	public:
		virtual vk::RenderPass GetRenderPass() const = 0;
		virtual vk::RenderPassBeginInfo GetRenderPassBeginInfo(const Flight& flight) = 0;
		const Pipeline* GetPipeline(const PipelineConfig& config);

	private:
		std::unordered_map<PipelineConfig, std::unique_ptr<Pipeline>, PipelineConfig::Hash> _pipelineMap;
	};
} // namespace Ailurus