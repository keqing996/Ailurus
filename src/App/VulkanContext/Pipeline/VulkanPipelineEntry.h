#pragma once

#include <cstdint>
#include <Ailurus/Application/RenderSystem/RenderPass/RenderPassType.h>

namespace Ailurus
{
    struct VulkanPipelineEntry
    {
        RenderPassType renderPass;
        uint32_t materialAssetId;
        uint64_t vertexLayoutId;

		VulkanPipelineEntry(RenderPassType renderPassType, uint32_t materialId, uint64_t layoutId);
	};

    struct VulkanPipelineEntryHash
    {
		std::size_t operator()(const VulkanPipelineEntry& entry) const;
	};

    struct VulkanPipelineEntryEqual
    {
		bool operator()(const VulkanPipelineEntry& lhs, const VulkanPipelineEntry& rhs) const;
	};
}