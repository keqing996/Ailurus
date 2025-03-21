#pragma once

#include <vulkan/vulkan.hpp>
#include "Ailurus/Application/Shader/ShaderStage.h"

namespace Ailurus
{
    class RHIShader
    {
    public:
        RHIShader(const std::string& path);
        RHIShader(const char* binaryData, size_t size);
        ~RHIShader();

    public:
        vk::ShaderModule GetShaderModule() const;
        vk::PipelineShaderStageCreateInfo GeneratePipelineCreateInfo(ShaderStage stage) const;

    public:
        static vk::ShaderStageFlagBits ToVulkanEnum(ShaderStage stage);

    private:
        void CreateShaderModule(const char* binaryData, size_t size);

    private:
        vk::ShaderModule _vkShaderModule = nullptr;
    };

}