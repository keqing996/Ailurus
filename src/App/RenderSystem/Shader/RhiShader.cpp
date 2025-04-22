#include "RhiShader.h"
#include "Ailurus/Utility/File.h"
#include "VulkanSystem/VulkanSystem.h"
#include "Ailurus/Application/Application.h"

namespace Ailurus
{
    RhiShader::RhiShader(const std::string& path)
    {
        auto binaryFile = File::LoadBinary(path);
        if (binaryFile.has_value())
            CreateShaderModule(binaryFile.value().data(), binaryFile.value().size());
    }

    RhiShader::RhiShader(const char* binaryData, size_t size)
    {
        CreateShaderModule(binaryData, size);
    }

    RhiShader::~RhiShader()
    {
        Application::Get<VulkanSystem>()->GetDevice().destroyShaderModule(_vkShaderModule);
    }

    vk::ShaderModule RhiShader::GetShaderModule() const
    {
        return _vkShaderModule;
    }

    vk::PipelineShaderStageCreateInfo RhiShader::GeneratePipelineCreateInfo(ShaderStage stage) const
    {
        vk::PipelineShaderStageCreateInfo createInfo;
        createInfo.setModule(_vkShaderModule)
                .setStage(ToVulkanEnum(stage))
                .setPName("main");

        return createInfo;
    }

    vk::ShaderStageFlagBits RhiShader::ToVulkanEnum(ShaderStage stage)
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

    void RhiShader::CreateShaderModule(const char* binaryData, size_t size)
    {
        vk::ShaderModuleCreateInfo createInfo;
        createInfo.setPCode(reinterpret_cast<const uint32_t*>(binaryData))
                .setCodeSize(size);

        _vkShaderModule = Application::Get<VulkanSystem>()->GetDevice().createShaderModule(createInfo);
    }
}
