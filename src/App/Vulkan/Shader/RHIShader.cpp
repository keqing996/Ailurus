#include "RHIShader.h"
#include "Ailurus/Utility/File.h"
#include "Vulkan/Context/VulkanContext.h"

namespace Ailurus
{
    RHIShader::RHIShader(const std::string& path)
    {
        auto binaryFile = File::LoadBinary(path);
        if (binaryFile.has_value())
            CreateShaderModule(binaryFile.value().data(), binaryFile.value().size());
    }

    RHIShader::RHIShader(const char* binaryData, size_t size)
    {
        CreateShaderModule(binaryData, size);
    }

    RHIShader::~RHIShader()
    {
        VulkanContext::GetDevice().destroyShaderModule(_vkShaderModule);
    }

    vk::ShaderModule RHIShader::GetShaderModule() const
    {
        return _vkShaderModule;
    }

    vk::PipelineShaderStageCreateInfo RHIShader::GeneratePipelineCreateInfo(ShaderStage stage) const
    {
        vk::PipelineShaderStageCreateInfo createInfo;
        createInfo.setModule(_vkShaderModule)
                .setStage(ToVulkanEnum(stage))
                .setPName("main");

        return createInfo;
    }

    vk::ShaderStageFlagBits RHIShader::ToVulkanEnum(ShaderStage stage)
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

    void RHIShader::CreateShaderModule(const char* binaryData, size_t size)
    {
        vk::ShaderModuleCreateInfo createInfo;
        createInfo.setPCode(reinterpret_cast<const uint32_t*>(binaryData))
                .setCodeSize(size);

        _vkShaderModule = VulkanContext::GetDevice().createShaderModule(createInfo);
    }
}
