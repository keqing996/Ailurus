#include "Shader.h"
#include "Ailurus/Utility/File.h"
#include "Vulkan/Context/VulkanContext.h"

namespace Ailurus
{
    Shader::Shader(ShaderStage stage, const std::string& path)
        : _stage(stage)
    {
        auto binaryFile = File::LoadBinary(path);
        if (binaryFile.has_value())
            CreateShaderModule(binaryFile.value().data(), binaryFile.value().size());
    }

    Shader::Shader(ShaderStage stage, const char* binaryData, size_t size)
        : _stage(stage)
    {
        CreateShaderModule(binaryData, size);
    }

    Shader::~Shader()
    {
        VulkanContext::GetDevice().destroyShaderModule(_vkShaderModule);
    }

    ShaderStage Shader::GetStage() const
    {
        return _stage;
    }

    vk::ShaderModule Shader::GetShaderModule() const
    {
        return _vkShaderModule;
    }

    vk::PipelineShaderStageCreateInfo Shader::GeneratePipelineCreateInfo() const
    {
        vk::PipelineShaderStageCreateInfo createInfo;
        createInfo.setModule(_vkShaderModule)
                .setStage(ToVulkanEnum(_stage))
                .setPName("main");

        return createInfo;
    }

    vk::ShaderStageFlagBits Shader::ToVulkanEnum(ShaderStage stage)
    {
        vk::ShaderStageFlagBits vkStage = vk::ShaderStageFlagBits::eAll;
        switch (stage)
        {
            case ShaderStage::Vertex:
                vkStage = vk::ShaderStageFlagBits::eVertex;
            break;
            case ShaderStage::Fragment:
                vkStage = vk::ShaderStageFlagBits::eFragment;
            break;
        }

        return vkStage;
    }

    void Shader::CreateShaderModule(const char* binaryData, size_t size)
    {
        vk::ShaderModuleCreateInfo createInfo;
        createInfo.setPCode(reinterpret_cast<const uint32_t*>(binaryData))
                .setCodeSize(size);

        _vkShaderModule = VulkanContext::GetDevice().createShaderModule(createInfo);
    }
}
