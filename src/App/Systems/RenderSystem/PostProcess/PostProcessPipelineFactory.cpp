#include "Ailurus/Application/RenderSystem/PostProcess/PostProcessPipelineFactory.h"
#include "Ailurus/Application/RenderSystem/Shader/ShaderLibrary.h"
#include "Ailurus/Application/RenderSystem/Shader/ShaderStage.h"
#include "Ailurus/Application/RenderSystem/Shader/Shader.h"
#include "VulkanContext/Pipeline/VulkanPipeline.h"
#include "VulkanContext/Descriptor/VulkanDescriptorSetLayout.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
    const char* PostProcessPipelineFactory::FULLSCREEN_VERT_PATH =
        "./Assets/ShaderBin/PostProcess/fullscreen.vert.spv";

    void PostProcessPipelineFactory::Init(ShaderLibrary* pShaderLibrary)
    {
        if (pShaderLibrary == nullptr)
        {
            Logger::LogError("PostProcessPipelineFactory::Init: ShaderLibrary is null");
            return;
        }

        _pShaderLibrary = pShaderLibrary;
        _pFullscreenVertShader = pShaderLibrary->GetShader(ShaderStage::Vertex, FULLSCREEN_VERT_PATH);
        if (_pFullscreenVertShader == nullptr)
            Logger::LogError("PostProcessPipelineFactory::Init: Failed to load fullscreen vertex shader");
    }

    std::unique_ptr<VulkanPipeline> PostProcessPipelineFactory::CreatePipeline(const PostProcessPipelineDesc& desc)
    {
        if (_pFullscreenVertShader == nullptr)
        {
            Logger::LogError("PostProcessPipelineFactory::CreatePipeline: Fullscreen vertex shader not initialized");
            return nullptr;
        }

        if (desc.pDescriptorSetLayout == nullptr)
        {
            Logger::LogError("PostProcessPipelineFactory::CreatePipeline: Descriptor set layout is null");
            return nullptr;
        }

        if (desc.fragShaderPath.empty())
        {
            Logger::LogError("PostProcessPipelineFactory::CreatePipeline: Fragment shader path is empty");
            return nullptr;
        }

        // Load fragment shader
        Shader* pFragShader = _pShaderLibrary->GetShader(ShaderStage::Fragment, desc.fragShaderPath);
        if (pFragShader == nullptr)
        {
            Logger::LogError("PostProcessPipelineFactory::CreatePipeline: Failed to load fragment shader: {}", desc.fragShaderPath);
            return nullptr;
        }

        // Build shader array
        StageShaderArray shaderArray{};
        shaderArray[ShaderStage::Vertex] = _pFullscreenVertShader;
        shaderArray[ShaderStage::Fragment] = pFragShader;

        // Build descriptor set layout vector
        std::vector<vk::DescriptorSetLayout> layouts;
        layouts.push_back(desc.pDescriptorSetLayout->GetDescriptorSetLayout());

        return std::make_unique<VulkanPipeline>(
            desc.outputFormat,
            shaderArray,
            layouts,
            desc.pushConstantSize,
            desc.blendEnabled);
    }

    Shader* PostProcessPipelineFactory::GetFullscreenVertexShader() const
    {
        return _pFullscreenVertShader;
    }
} // namespace Ailurus
