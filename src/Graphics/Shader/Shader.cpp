#include "Ailurus/Graphics/Shader/Shader.h"
#include "Ailurus/Utility/File.h"

namespace Ailurus
{
    Shader::Shader(vk::Device logicDevice, ShaderStage stage, const std::string& path)
        : _vkLogicDevice(logicDevice)
          , _stage(stage)
    {
        auto binaryFile = File::LoadBinary(path);
        if (binaryFile)
            Shader(logicDevice, stage, binaryFile.value().data(), binaryFile.value().size());
    }

    Shader::Shader(vk::Device logicDevice, ShaderStage stage, const char* binaryData, size_t size)
        : _vkLogicDevice(logicDevice)
          , _stage(stage)
    {
        vk::ShaderModuleCreateInfo createInfo;
        createInfo.setPCode(reinterpret_cast<const uint32_t*>(binaryData))
                .setCodeSize(size);

        _vkShaderModule = logicDevice.createShaderModule(createInfo);
    }

    Shader::~Shader()
    {
        _vkLogicDevice.destroyShaderModule(_vkShaderModule);
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
