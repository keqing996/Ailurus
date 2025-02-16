#include "Ailurus/Graphics/Shader/Shader.h"
#include "Ailurus/Utility/File.h"
#include "Ailurus/Graphics/Renderer.h"

namespace Ailurus
{
    Shader::Shader(const Renderer* pRenderer, ShaderStage stage, const std::string& path)
        : _pRenderer(pRenderer)
        , _stage(stage)
    {
        auto binaryFile = File::LoadBinary(path);
        if (binaryFile.has_value())
            Shader(pRenderer, stage, binaryFile.value().data(), binaryFile.value().size());
    }

    Shader::Shader(const Renderer* pRenderer, ShaderStage stage, const char* binaryData, size_t size)
        : _pRenderer(pRenderer)
        , _stage(stage)
    {
        vk::ShaderModuleCreateInfo createInfo;
        createInfo.setPCode(reinterpret_cast<const uint32_t*>(binaryData))
                .setCodeSize(size);

        _vkShaderModule = _pRenderer->GetLogicalDevice().createShaderModule(createInfo);
    }

    Shader::~Shader()
    {
        _pRenderer->GetLogicalDevice().destroyShaderModule(_vkShaderModule);
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
}
