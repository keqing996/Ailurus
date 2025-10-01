#include "VulkanShader.h"
#include "Ailurus/Utility/File.h"
#include "Ailurus/Application/Application.h"
#include "VulkanContext/VulkanContext.h"
#include "VulkanContext/Helper/VulkanHelper.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
    VulkanShader::VulkanShader(const std::string& path)
    {
        auto binaryFile = File::LoadBinary(path);
        if (binaryFile.has_value())
            CreateShaderModule(binaryFile.value().data(), binaryFile.value().size());
    }

    VulkanShader::VulkanShader(const char* binaryData, size_t size)
    {
        CreateShaderModule(binaryData, size);
    }

    VulkanShader::~VulkanShader()
    {
        try
        {
            VulkanContext::GetDevice().destroyShaderModule(_vkShaderModule);
        }
        catch (const vk::SystemError& e)
        {
            Logger::LogError("Failed to destroy shader module: {}", e.what());
        }
    }

    vk::ShaderModule VulkanShader::GetShaderModule() const
    {
        return _vkShaderModule;
    }

    vk::PipelineShaderStageCreateInfo VulkanShader::GeneratePipelineCreateInfo(ShaderStage stage) const
    {
        vk::PipelineShaderStageCreateInfo createInfo;
        createInfo.setModule(_vkShaderModule)
                .setStage(VulkanHelper::GetShaderStage(stage))
                .setPName("main");

        return createInfo;
    }

    void VulkanShader::CreateShaderModule(const char* binaryData, size_t size)
    {
        vk::ShaderModuleCreateInfo createInfo;
        createInfo.setPCode(reinterpret_cast<const uint32_t*>(binaryData))
                .setCodeSize(size);

        try
        {
            _vkShaderModule = VulkanContext::GetDevice().createShaderModule(createInfo);
        }
        catch (const vk::SystemError& e)
        {
            Logger::LogError("Failed to create shader module: {}", e.what());
        }
    }
}
