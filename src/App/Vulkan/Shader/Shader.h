#pragma once

#include <vulkan/vulkan.hpp>
#include "Ailurus/Application/Render/Shader/ShaderStage.h"

namespace Ailurus
{
    class Shader
    {
    public:
        Shader(ShaderStage stage, const std::string& path);
        Shader(ShaderStage stage, const char* binaryData, size_t size);
        ~Shader();

    public:
        ShaderStage GetStage() const;
        vk::ShaderModule GetShaderModule() const;
        vk::PipelineShaderStageCreateInfo GeneratePipelineCreateInfo() const;

    public:
        static vk::ShaderStageFlagBits ToVulkanEnum(ShaderStage stage);

    private:
        void CreateShaderModule(const char* binaryData, size_t size);

    private:
        ShaderStage _stage;
        vk::ShaderModule _vkShaderModule = nullptr;
    };

    using PipelineShaderStages = std::array<const Shader*, EnumReflection<ShaderStage>::Size()>;

}