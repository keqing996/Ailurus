#include "Ailurus/Systems/RenderSystem/PostProcess/Effects/ToneMappingEffect.h"
#include "Ailurus/Systems/RenderSystem/PostProcess/PostProcessResourcePool.h"
#include "Ailurus/Systems/RenderSystem/PostProcess/PostProcessPipelineFactory.h"
#include "VulkanContext/Pipeline/VulkanPipeline.h"
#include "Ailurus/Systems/RenderSystem/Descriptor/SamplerSchema.h"
#include "Ailurus/Systems/RenderSystem/Descriptor/SamplerSetData.h"
#include "VulkanContext/CommandBuffer/VulkanCommandBuffer.h"
#include "VulkanContext/Resource/VulkanResourceManager.h"
#include "VulkanContext/Resource/Image/VulkanSampler.h"
#include "VulkanContext/VulkanContext.h"
#include "VulkanContext/SwapChain/VulkanSwapChain.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
    const std::string ToneMappingEffect::EFFECT_NAME = "ToneMapping";
    const char* ToneMappingEffect::FRAG_SHADER_PATH = "./Assets/ShaderBin/PostProcess/tonemapping.frag.spv";

    ToneMappingEffect::ToneMappingEffect() = default;

    ToneMappingEffect::~ToneMappingEffect()
    {
        Shutdown();
    }

    const std::string& ToneMappingEffect::GetName() const
    {
        return EFFECT_NAME;
    }

    void ToneMappingEffect::Init(PostProcessResourcePool& pool, PostProcessPipelineFactory& factory,
        uint32_t width, uint32_t height, vk::Format format)
    {
        // Create descriptor set layout: binding 0 = combined image sampler (fragment stage)
        _descriptorSetLayout = std::make_unique<SamplerSchema>(
            std::vector<SamplerBindingDesc>{ {0} });

        // Determine output format: swapchain format (the final output is the swapchain image)
        const auto& swapChainConfig = VulkanContext::GetSwapChain()->GetConfig();
        vk::Format outputFormat = swapChainConfig.surfaceFormat.format;

        // Create pipeline
        PostProcessPipelineDesc pipelineDesc;
        pipelineDesc.outputFormat = outputFormat;
        pipelineDesc.fragShaderPath = FRAG_SHADER_PATH;
        pipelineDesc.pSchema = _descriptorSetLayout.get();
        pipelineDesc.pushConstantSize = static_cast<uint32_t>(sizeof(PushConstants));
        pipelineDesc.blendEnabled = false;

        _pipeline = factory.CreatePipeline(pipelineDesc);

        // Create sampler for input texture
        _sampler = VulkanContext::GetResourceManager()->CreateSampler();
    }

    void ToneMappingEffect::Render(VulkanCommandBuffer* pCmdBuffer, vk::ImageView inputImageView,
        vk::ImageView outputImageView, vk::Extent2D extent,
        VulkanDescriptorAllocator* pDescriptorAllocator)
    {
        if (!_pipeline || !_descriptorSetLayout || !_sampler)
            return;

        // Begin rendering to output image (no depth, loads existing alpha is fine; clear=true for color)
        pCmdBuffer->BeginRendering(outputImageView, nullptr, nullptr, extent, true, false);

        // Bind pipeline
        pCmdBuffer->BindPipeline(_pipeline.get());

        // Set viewport and scissor
        pCmdBuffer->SetViewportAndScissor(extent.width, extent.height);

        // Allocate and write descriptor set
        SamplerSetData samplerData;
        samplerData.SetImage(0, inputImageView, _sampler->GetSampler());
        auto descriptorSet = samplerData.AllocateAndWrite(pDescriptorAllocator, _descriptorSetLayout.get());

        // Bind descriptor set
        std::vector<vk::DescriptorSet> sets{ descriptorSet };
        pCmdBuffer->BindDescriptorSet(_pipeline->GetPipelineLayout(), sets);

        // Push exposure/gamma constants
        PushConstants pushConstants{ _exposure, _gamma };
        pCmdBuffer->PushConstants(_pipeline.get(),
            vk::ShaderStageFlagBits::eFragment,
            0,
            static_cast<uint32_t>(sizeof(PushConstants)),
            &pushConstants);

        // Draw full-screen triangle (3 vertices, no vertex buffer)
        pCmdBuffer->DrawNonIndexed(3);

        pCmdBuffer->EndRendering();
    }

    void ToneMappingEffect::OnResize(PostProcessResourcePool& pool, PostProcessPipelineFactory& factory,
        uint32_t width, uint32_t height, vk::Format format)
    {
        // Pipeline output format may change with swapchain; recreate pipeline
        if (_descriptorSetLayout)
        {
            const auto& swapChainConfig = VulkanContext::GetSwapChain()->GetConfig();
            vk::Format outputFormat = swapChainConfig.surfaceFormat.format;

            PostProcessPipelineDesc pipelineDesc;
            pipelineDesc.outputFormat = outputFormat;
            pipelineDesc.fragShaderPath = FRAG_SHADER_PATH;
            pipelineDesc.pSchema = _descriptorSetLayout.get();
            pipelineDesc.pushConstantSize = static_cast<uint32_t>(sizeof(PushConstants));
            pipelineDesc.blendEnabled = false;

            _pipeline = factory.CreatePipeline(pipelineDesc);
        }
    }

    void ToneMappingEffect::Shutdown()
    {
        _pipeline = nullptr;
        _descriptorSetLayout = nullptr;
        _sampler = nullptr;
    }
} // namespace Ailurus
