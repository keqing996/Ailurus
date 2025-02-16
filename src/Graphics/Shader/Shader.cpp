#include "Ailurus/Graphics/Shader/Shader.h"
#include "Ailurus/Utility/File.h"

namespace Ailurus
{
    Shader::~Shader()
    {
        _pContext->GetLogicalDevice().destroyShaderModule(_vkShaderModule);
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
                .setStage(ShaderStageUtil::ToVulkanEnum(_stage))
                .setPName("main");

        return createInfo;
    }

    std::unique_ptr<Shader> Shader::Create(const VulkanContext* pContext, ShaderStage stage, const std::string& path)
    {
        auto binaryFile = File::LoadBinary(path);
        if (!binaryFile)
            return nullptr;

        return Create(pContext, stage, binaryFile.value().data(), binaryFile.value().size());
    }

    std::unique_ptr<Shader> Shader::Create(const VulkanContext* pContext, ShaderStage stage, const char* binaryData,
        size_t size)
    {
        return std::unique_ptr<Shader>(new Shader(pContext, stage, binaryData, size));
    }

    Shader::Shader(const VulkanContext* pContext, ShaderStage stage, const char* binaryData, size_t size)
        : _pContext(pContext)
        , _stage(stage)
    {
        vk::ShaderModuleCreateInfo createInfo;
        createInfo.setPCode(reinterpret_cast<const uint32_t*>(binaryData))
                .setCodeSize(size);

        _vkShaderModule = pContext->GetLogicalDevice().createShaderModule(createInfo);
    }
}
