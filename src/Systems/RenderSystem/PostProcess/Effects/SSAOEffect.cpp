#include "Ailurus/Systems/RenderSystem/PostProcess/Effects/SSAOEffect.h"
#include "Ailurus/Systems/RenderSystem/PostProcess/PostProcessResourcePool.h"
#include "Ailurus/Systems/RenderSystem/PostProcess/PostProcessPipelineFactory.h"
#include "VulkanContext/Pipeline/VulkanPipeline.h"
#include "Ailurus/Systems/RenderSystem/Descriptor/SamplerSchema.h"
#include "Ailurus/Systems/RenderSystem/Descriptor/SamplerSetData.h"
#include "VulkanContext/CommandBuffer/VulkanCommandBuffer.h"
#include "VulkanContext/Resource/VulkanResourceManager.h"
#include "VulkanContext/Resource/Image/VulkanSampler.h"
#include "VulkanContext/RenderTarget/RenderTargetManager.h"
#include "VulkanContext/VulkanContext.h"
#include <cstring>

namespace Ailurus
{
    const std::string SSAOEffect::EFFECT_NAME = "SSAO";
    const char* SSAOEffect::GENERATE_SHADER_PATH = "./Assets/ShaderBin/PostProcess/ssao_generate.frag.spv";
    const char* SSAOEffect::BLUR_SHADER_PATH = "./Assets/ShaderBin/PostProcess/ssao_blur.frag.spv";
    const char* SSAOEffect::COMPOSITE_SHADER_PATH = "./Assets/ShaderBin/PostProcess/ssao_composite.frag.spv";

    SSAOEffect::SSAOEffect() = default;

    SSAOEffect::~SSAOEffect()
    {
        Shutdown();
    }

    const std::string& SSAOEffect::GetName() const
    {
        return EFFECT_NAME;
    }

    void SSAOEffect::SetProjectionMatrix(const Matrix4x4f& projMatrix)
    {
        _projectionMatrix = projMatrix;
    }

    void SSAOEffect::Init(PostProcessResourcePool& pool, PostProcessPipelineFactory& factory,
        uint32_t width, uint32_t height, vk::Format format)
    {
        // Single-sampler descriptor set layout (binding 0)
        _singleSamplerLayout = std::make_unique<SamplerSchema>(
            std::vector<SamplerBindingDesc>{ {0} });

        // Dual-sampler descriptor set layout (bindings 0 and 1)
        _dualSamplerLayout = std::make_unique<SamplerSchema>(
            std::vector<SamplerBindingDesc>{ {0}, {1} });

        // Register intermediate RTs (single-channel AO)
        RTSpec ssaoSpec;
        ssaoSpec.widthScale = 1.0f;
        ssaoSpec.heightScale = 1.0f;
        ssaoSpec.format = vk::Format::eR8Unorm;
        _ssaoRT = pool.RegisterRT(ssaoSpec);

        RTSpec blurSpec;
        blurSpec.widthScale = 1.0f;
        blurSpec.heightScale = 1.0f;
        blurSpec.format = vk::Format::eR8Unorm;
        _blurRT = pool.RegisterRT(blurSpec);

        // Create pipelines
        constexpr vk::Format AO_FORMAT = vk::Format::eR8Unorm;

        // SSAO generation: depth -> AO
        // Push constants: mat4(64) + float radius(4) + float bias(4) + float power(4) + int kernelSize(4) = 80 bytes
        PostProcessPipelineDesc generateDesc;
        generateDesc.outputFormat = AO_FORMAT;
        generateDesc.fragShaderPath = GENERATE_SHADER_PATH;
        generateDesc.pSchema = _singleSamplerLayout.get();
        generateDesc.pushConstantSize = 80;
        generateDesc.blendEnabled = false;
        _generatePipeline = factory.CreatePipeline(generateDesc);

        // Blur: AO -> blurred AO
        PostProcessPipelineDesc blurDesc;
        blurDesc.outputFormat = AO_FORMAT;
        blurDesc.fragShaderPath = BLUR_SHADER_PATH;
        blurDesc.pSchema = _singleSamplerLayout.get();
        blurDesc.pushConstantSize = 0;
        blurDesc.blendEnabled = false;
        _blurPipeline = factory.CreatePipeline(blurDesc);

        // Composite: scene color + blurred AO -> final color
        PostProcessPipelineDesc compositeDesc;
        compositeDesc.outputFormat = format;
        compositeDesc.fragShaderPath = COMPOSITE_SHADER_PATH;
        compositeDesc.pSchema = _dualSamplerLayout.get();
        compositeDesc.pushConstantSize = sizeof(float);
        compositeDesc.blendEnabled = false;
        _compositePipeline = factory.CreatePipeline(compositeDesc);

        // Create samplers
        _sampler = VulkanContext::GetResourceManager()->CreateSampler();

        VulkanSamplerCreateConfig depthSamplerConfig;
        depthSamplerConfig.addressModeU = vk::SamplerAddressMode::eClampToEdge;
        depthSamplerConfig.addressModeV = vk::SamplerAddressMode::eClampToEdge;
        depthSamplerConfig.addressModeW = vk::SamplerAddressMode::eClampToEdge;
        _depthSampler = VulkanContext::GetResourceManager()->CreateSampler(depthSamplerConfig);
    }

    void SSAOEffect::Render(VulkanCommandBuffer* pCmdBuffer, vk::ImageView inputImageView,
        vk::ImageView outputImageView, vk::Extent2D extent,
        VulkanDescriptorAllocator* pDescriptorAllocator)
    {
        if (!_generatePipeline || !_blurPipeline || !_compositePipeline)
            return;

        // Get depth image view from render target manager
        auto* pRenderTargetManager = VulkanContext::GetRenderTargetManager();
        vk::ImageView depthImageView = _depthImageViewOverride ? _depthImageViewOverride : pRenderTargetManager->GetDepthImageView();
        if (!depthImageView)
            return;

        // === Pass 1: Generate SSAO ===
        {
            RTHandle* dst = _ssaoRT;
            pCmdBuffer->ImageMemoryBarrier(
                dst->GetImage(),
                vk::ImageLayout::eUndefined,
                vk::ImageLayout::eColorAttachmentOptimal,
                vk::AccessFlags{},
                vk::AccessFlagBits::eColorAttachmentWrite,
                vk::PipelineStageFlagBits::eTopOfPipe,
                vk::PipelineStageFlagBits::eColorAttachmentOutput);

            vk::Extent2D aoExtent{ dst->GetWidth(), dst->GetHeight() };
            pCmdBuffer->BeginRendering(dst->GetImageView(), nullptr, nullptr, aoExtent, true, false);
            pCmdBuffer->BindPipeline(_generatePipeline.get());
            pCmdBuffer->SetViewportAndScissor(aoExtent.width, aoExtent.height);

            SamplerSetData samplerData;
            samplerData.SetImage(0, depthImageView, _depthSampler->GetSampler());
            auto descriptorSet = samplerData.AllocateAndWrite(pDescriptorAllocator, _singleSamplerLayout.get());

            std::vector<vk::DescriptorSet> sets{ descriptorSet };
            pCmdBuffer->BindDescriptorSet(_generatePipeline->GetPipelineLayout(), sets);

            // Push constants: mat4 projection + radius + bias + power + kernelSize
            struct GeneratePushConstants {
                float projection[16];
                float radius;
                float bias;
                float power;
                int kernelSize;
            };
            GeneratePushConstants pc;
            std::memcpy(pc.projection, _projectionMatrix.GetDataPtr(), sizeof(float) * 16);
            pc.radius = _radius;
            pc.bias = _bias;
            pc.power = _power;
            pc.kernelSize = _kernelSize;

            pCmdBuffer->PushConstants(_generatePipeline.get(),
                vk::ShaderStageFlagBits::eFragment,
                0, sizeof(pc), &pc);

            pCmdBuffer->DrawNonIndexed(3);
            pCmdBuffer->EndRendering();

            pCmdBuffer->ImageMemoryBarrier(
                dst->GetImage(),
                vk::ImageLayout::eColorAttachmentOptimal,
                vk::ImageLayout::eShaderReadOnlyOptimal,
                vk::AccessFlagBits::eColorAttachmentWrite,
                vk::AccessFlagBits::eShaderRead,
                vk::PipelineStageFlagBits::eColorAttachmentOutput,
                vk::PipelineStageFlagBits::eFragmentShader);
        }

        // === Pass 2: Blur SSAO ===
        {
            RTHandle* dst = _blurRT;
            pCmdBuffer->ImageMemoryBarrier(
                dst->GetImage(),
                vk::ImageLayout::eUndefined,
                vk::ImageLayout::eColorAttachmentOptimal,
                vk::AccessFlags{},
                vk::AccessFlagBits::eColorAttachmentWrite,
                vk::PipelineStageFlagBits::eTopOfPipe,
                vk::PipelineStageFlagBits::eColorAttachmentOutput);

            vk::Extent2D blurExtent{ dst->GetWidth(), dst->GetHeight() };
            pCmdBuffer->BeginRendering(dst->GetImageView(), nullptr, nullptr, blurExtent, true, false);
            pCmdBuffer->BindPipeline(_blurPipeline.get());
            pCmdBuffer->SetViewportAndScissor(blurExtent.width, blurExtent.height);

            SamplerSetData samplerData;
            samplerData.SetImage(0, _ssaoRT->GetImageView(), _sampler->GetSampler());
            auto descriptorSet = samplerData.AllocateAndWrite(pDescriptorAllocator, _singleSamplerLayout.get());

            std::vector<vk::DescriptorSet> sets{ descriptorSet };
            pCmdBuffer->BindDescriptorSet(_blurPipeline->GetPipelineLayout(), sets);

            pCmdBuffer->DrawNonIndexed(3);
            pCmdBuffer->EndRendering();

            pCmdBuffer->ImageMemoryBarrier(
                dst->GetImage(),
                vk::ImageLayout::eColorAttachmentOptimal,
                vk::ImageLayout::eShaderReadOnlyOptimal,
                vk::AccessFlagBits::eColorAttachmentWrite,
                vk::AccessFlagBits::eShaderRead,
                vk::PipelineStageFlagBits::eColorAttachmentOutput,
                vk::PipelineStageFlagBits::eFragmentShader);
        }

        // === Pass 3: Composite (scene color * AO -> output) ===
        {
            pCmdBuffer->BeginRendering(outputImageView, nullptr, nullptr, extent, true, false);
            pCmdBuffer->BindPipeline(_compositePipeline.get());
            pCmdBuffer->SetViewportAndScissor(extent.width, extent.height);

            SamplerSetData samplerData;
            samplerData.SetImage(0, inputImageView, _sampler->GetSampler());
            samplerData.SetImage(1, _blurRT->GetImageView(), _sampler->GetSampler());
            auto descriptorSet = samplerData.AllocateAndWrite(pDescriptorAllocator, _dualSamplerLayout.get());

            std::vector<vk::DescriptorSet> sets{ descriptorSet };
            pCmdBuffer->BindDescriptorSet(_compositePipeline->GetPipelineLayout(), sets);

            float ssaoStrength = _strength;
            pCmdBuffer->PushConstants(_compositePipeline.get(),
                vk::ShaderStageFlagBits::eFragment,
                0, sizeof(float), &ssaoStrength);

            pCmdBuffer->DrawNonIndexed(3);
            pCmdBuffer->EndRendering();
        }
    }

    void SSAOEffect::OnResize(PostProcessResourcePool& pool, PostProcessPipelineFactory& factory,
        uint32_t width, uint32_t height, vk::Format format)
    {
        // Composite pipeline uses the chain format which could change on resize
        if (_dualSamplerLayout)
        {
            PostProcessPipelineDesc compositeDesc;
            compositeDesc.outputFormat = format;
            compositeDesc.fragShaderPath = COMPOSITE_SHADER_PATH;
            compositeDesc.pSchema = _dualSamplerLayout.get();
            compositeDesc.pushConstantSize = sizeof(float);
            compositeDesc.blendEnabled = false;
            _compositePipeline = factory.CreatePipeline(compositeDesc);
        }
    }

    void SSAOEffect::Shutdown()
    {
        _generatePipeline.reset();
        _blurPipeline.reset();
        _compositePipeline.reset();
        _singleSamplerLayout.reset();
        _dualSamplerLayout.reset();
        // Samplers are managed by VulkanResourceManager
        _sampler = nullptr;
        _depthSampler = nullptr;
        _ssaoRT = nullptr;
        _blurRT = nullptr;
        _depthImageViewOverride = nullptr;
    }
} // namespace Ailurus
