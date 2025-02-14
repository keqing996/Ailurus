#pragma once

#include <memory>
#include <vulkan/vulkan.hpp>
#include "ShaderStage.h"
#include "../Context/VulkanContext.h"

namespace Ailurus
{
    class Shader
    {
    public:
        ~Shader();

    public:
        ShaderStage GetStage() const;
        vk::ShaderModule GetShaderModule() const;
        vk::PipelineShaderStageCreateInfo GeneratePipelineCreateInfo() const;

        static std::unique_ptr<Shader> Create(const VulkanContext* pContext, ShaderStage stage, const std::string& path);
        static std::unique_ptr<Shader> Create(const VulkanContext* pContext, ShaderStage stage, const char* binaryData, size_t size);

    private:
        Shader(const VulkanContext* pContext, ShaderStage stage, const char* binaryData, size_t size);

    private:
        const VulkanContext* _pContext;

        ShaderStage _stage;
        vk::ShaderModule _vkShaderModule = nullptr;
    };
}