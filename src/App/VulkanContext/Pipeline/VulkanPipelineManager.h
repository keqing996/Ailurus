#pragma once

#include "VulkanContext/VulkanPch.h"
#include <memory>
#include <unordered_map>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>
#include "VulkanPipelineEntry.h"
#include "VulkanPipeline.h"

namespace Ailurus
{
    class VulkanPipelineManager : public NonCopyable, public NonMovable
    {
        using PipelineMap = std::unordered_map<VulkanPipelineEntry, std::unique_ptr<VulkanPipeline>, 
            VulkanPipelineEntryHash, VulkanPipelineEntryEqual>;
    public:
		auto GetPipeline(const VulkanPipelineEntry& entry) -> VulkanPipeline*;
        auto ClearPipelines() -> void;

    private:
        PipelineMap _pipelinesMap;
    };
}