#pragma once

#include "VulkanContext/VulkanPch.h"
#include "Ailurus/Application/RenderSystem/Shader/ShaderStage.h"

namespace Ailurus
{
    class VulkanShader
    {
    public:
        VulkanShader(const std::string& path);
        VulkanShader(const char* binaryData, size_t size);
        ~VulkanShader();

    public:
        vk::ShaderModule GetShaderModule() const;
        vk::PipelineShaderStageCreateInfo GeneratePipelineCreateInfo(ShaderStage stage) const;

    private:
        void CreateShaderModule(const char* binaryData, size_t size);

    private:
        vk::ShaderModule _vkShaderModule = nullptr;
    };

}