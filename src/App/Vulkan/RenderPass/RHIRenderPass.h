#pragma once

#include <memory>
#include <unordered_map>
#include <vulkan/vulkan.hpp>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Application/RenderPass/RenderPassType.h"
#include "Vulkan/Airport/Flight.h"
#include "Vulkan/Pipeline/RHIPipeline.h"

namespace Ailurus
{
	class RHIRenderPass : public NonCopyable
	{
	public:
		virtual RenderPassType GetRenderPassType() = 0;
		virtual vk::RenderPass GetRenderPass() const = 0;
		virtual vk::RenderPassBeginInfo GetRenderPassBeginInfo(const Flight& flight) const = 0;
		const Pipeline* GetPipeline(const PipelineConfig& config);

	private:
		std::unordered_map<PipelineConfig, std::unique_ptr<Pipeline>, PipelineConfig::Hash> _pipelineMap;
	};
} // namespace Ailurus