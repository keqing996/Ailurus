#pragma once

#include <memory>
#include <unordered_map>
#include <vulkan/vulkan.hpp>
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
        void CreatePipelineIfNotExist(const class VulkanRenderPass* pRenderPass, const class Material* pMaterial, 
            uint64_t vertexLayoutId);

    private:
        PipelineMap _pipelinesMap;
    };
}