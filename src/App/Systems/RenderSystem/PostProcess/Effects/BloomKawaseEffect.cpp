#include "Ailurus/Application/RenderSystem/PostProcess/Effects/BloomKawaseEffect.h"
#include "Ailurus/Application/RenderSystem/PostProcess/PostProcessResourcePool.h"
#include "Ailurus/Application/RenderSystem/PostProcess/PostProcessPipelineFactory.h"
#include "VulkanContext/Pipeline/VulkanPipeline.h"
#include "VulkanContext/Descriptor/VulkanDescriptorSetLayout.h"
#include "VulkanContext/Descriptor/VulkanDescriptorAllocator.h"
#include "VulkanContext/Descriptor/VulkanDescriptorWriter.h"
#include "VulkanContext/CommandBuffer/VulkanCommandBuffer.h"
#include "VulkanContext/Resource/VulkanResourceManager.h"
#include "VulkanContext/Resource/Image/VulkanSampler.h"
#include "VulkanContext/VulkanContext.h"

namespace Ailurus
{
    const std::string BloomKawaseEffect::EFFECT_NAME = "BloomKawase";
    const char* BloomKawaseEffect::THRESHOLD_SHADER_PATH  = "./Assets/ShaderBin/PostProcess/bloom_threshold.frag.spv";
    const char* BloomKawaseEffect::DOWNSAMPLE_SHADER_PATH = "./Assets/ShaderBin/PostProcess/bloom_kawase_down.frag.spv";
    const char* BloomKawaseEffect::UPSAMPLE_SHADER_PATH   = "./Assets/ShaderBin/PostProcess/bloom_kawase_up.frag.spv";
    const char* BloomKawaseEffect::COMPOSITE_SHADER_PATH  = "./Assets/ShaderBin/PostProcess/bloom_composite.frag.spv";

    BloomKawaseEffect::~BloomKawaseEffect()
    {
        Shutdown();
    }

    const std::string& BloomKawaseEffect::GetName() const
    {
        return EFFECT_NAME;
    }

    void BloomKawaseEffect::Init(PostProcessResourcePool& pool, PostProcessPipelineFactory& factory,
        uint32_t width, uint32_t height, vk::Format format)
    {
        // Single-sampler descriptor set layout (binding 0)
        vk::DescriptorSetLayoutBinding singleBinding;
        singleBinding.setBinding(0)
            .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
            .setDescriptorCount(1)
            .setStageFlags(vk::ShaderStageFlagBits::eFragment);

        _singleSamplerLayout = std::make_unique<VulkanDescriptorSetLayout>(
            std::vector<vk::DescriptorSetLayoutBinding>{ singleBinding });

        // Dual-sampler descriptor set layout (bindings 0 and 1)
        vk::DescriptorSetLayoutBinding dualBinding0;
        dualBinding0.setBinding(0)
            .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
            .setDescriptorCount(1)
            .setStageFlags(vk::ShaderStageFlagBits::eFragment);

        vk::DescriptorSetLayoutBinding dualBinding1;
        dualBinding1.setBinding(1)
            .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
            .setDescriptorCount(1)
            .setStageFlags(vk::ShaderStageFlagBits::eFragment);

        _dualSamplerLayout = std::make_unique<VulkanDescriptorSetLayout>(
            std::vector<vk::DescriptorSetLayoutBinding>{ dualBinding0, dualBinding1 });

        // Register mip RT handles with the pool
        // widthScale/heightScale: 1/2, 1/4, 1/8, 1/16, 1/32
        static constexpr float MIP_SCALES[MIP_COUNT] = { 0.5f, 0.25f, 0.125f, 0.0625f, 0.03125f };
        for (int i = 0; i < MIP_COUNT; ++i)
        {
            RTSpec spec;
            spec.widthScale = MIP_SCALES[i];
            spec.heightScale = MIP_SCALES[i];
            spec.format = vk::Format::eR16G16B16A16Sfloat;
            _downMips[i] = pool.RegisterRT(spec);
        }
        for (int i = 0; i < MIP_COUNT - 1; ++i)
        {
            RTSpec spec;
            spec.widthScale = MIP_SCALES[i];
            spec.heightScale = MIP_SCALES[i];
            spec.format = vk::Format::eR16G16B16A16Sfloat;
            _upMips[i] = pool.RegisterRT(spec);
        }

        // Create pipelines
        constexpr vk::Format HDR_FORMAT = vk::Format::eR16G16B16A16Sfloat;

        PostProcessPipelineDesc thresholdDesc;
        thresholdDesc.outputFormat = HDR_FORMAT;
        thresholdDesc.fragShaderPath = THRESHOLD_SHADER_PATH;
        thresholdDesc.pDescriptorSetLayout = _singleSamplerLayout.get();
        thresholdDesc.pushConstantSize = sizeof(float) * 2; // threshold + softKnee
        thresholdDesc.blendEnabled = false;
        _thresholdPipeline = factory.CreatePipeline(thresholdDesc);

        PostProcessPipelineDesc downsampleDesc;
        downsampleDesc.outputFormat = HDR_FORMAT;
        downsampleDesc.fragShaderPath = DOWNSAMPLE_SHADER_PATH;
        downsampleDesc.pDescriptorSetLayout = _singleSamplerLayout.get();
        downsampleDesc.pushConstantSize = sizeof(float) * 2; // vec2 texelSize
        downsampleDesc.blendEnabled = false;
        _downsamplePipeline = factory.CreatePipeline(downsampleDesc);

        PostProcessPipelineDesc upsampleDesc;
        upsampleDesc.outputFormat = HDR_FORMAT;
        upsampleDesc.fragShaderPath = UPSAMPLE_SHADER_PATH;
        upsampleDesc.pDescriptorSetLayout = _dualSamplerLayout.get();
        upsampleDesc.pushConstantSize = sizeof(float) * 3; // vec2 texelSize + float blendFactor
        upsampleDesc.blendEnabled = false;
        _upsamplePipeline = factory.CreatePipeline(upsampleDesc);

        PostProcessPipelineDesc compositeDesc;
        compositeDesc.outputFormat = HDR_FORMAT;
        compositeDesc.fragShaderPath = COMPOSITE_SHADER_PATH;
        compositeDesc.pDescriptorSetLayout = _dualSamplerLayout.get();
        compositeDesc.pushConstantSize = sizeof(float); // bloomIntensity
        compositeDesc.blendEnabled = false;
        _compositePipeline = factory.CreatePipeline(compositeDesc);

        // Create linear sampler
        _sampler = VulkanContext::GetResourceManager()->CreateSampler();
    }

    void BloomKawaseEffect::Render(VulkanCommandBuffer* pCmdBuffer, vk::ImageView inputImageView,
        vk::ImageView outputImageView, vk::Extent2D extent,
        VulkanDescriptorAllocator* pDescriptorAllocator)
    {
        if (!_thresholdPipeline || !_sampler)
            return;

        // --- Step 1: Bright extract (input → downMips[0]) ---
        {
            RTHandle* dst = _downMips[0];
            pCmdBuffer->ImageMemoryBarrier(
                dst->GetImage(),
                vk::ImageLayout::eUndefined,
                vk::ImageLayout::eColorAttachmentOptimal,
                vk::AccessFlags{},
                vk::AccessFlagBits::eColorAttachmentWrite,
                vk::PipelineStageFlagBits::eTopOfPipe,
                vk::PipelineStageFlagBits::eColorAttachmentOutput);

            vk::Extent2D mipExtent{ dst->GetWidth(), dst->GetHeight() };
            pCmdBuffer->BeginRendering(dst->GetImageView(), nullptr, nullptr, mipExtent, true, false);
            pCmdBuffer->BindPipeline(_thresholdPipeline.get());
            pCmdBuffer->SetViewportAndScissor(mipExtent.width, mipExtent.height);

            VulkanDescriptorAllocator::CacheKey key;
            key.layout = _singleSamplerLayout->GetDescriptorSetLayout();
            key.bindingHash = reinterpret_cast<size_t>(static_cast<VkImageView>(inputImageView));
            auto descriptorSet = pDescriptorAllocator->AllocateDescriptorSet(_singleSamplerLayout.get(), &key);

            VulkanDescriptorWriter writer;
            writer.WriteImage(0, inputImageView, _sampler->GetSampler());
            writer.UpdateSet(descriptorSet);

            std::vector<vk::DescriptorSet> sets{ descriptorSet };
            pCmdBuffer->BindDescriptorSet(_thresholdPipeline->GetPipelineLayout(), sets);

            struct ThresholdPushConstants { float threshold; float softKnee; };
            ThresholdPushConstants pc{ _threshold, _softKnee };
            pCmdBuffer->GetBuffer().pushConstants(
                _thresholdPipeline->GetPipelineLayout(),
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

        // --- Step 2: Downsample chain (downMips[i-1] → downMips[i]) ---
        for (int i = 1; i < MIP_COUNT; ++i)
        {
            RTHandle* src = _downMips[i - 1];
            RTHandle* dst = _downMips[i];

            pCmdBuffer->ImageMemoryBarrier(
                dst->GetImage(),
                vk::ImageLayout::eUndefined,
                vk::ImageLayout::eColorAttachmentOptimal,
                vk::AccessFlags{},
                vk::AccessFlagBits::eColorAttachmentWrite,
                vk::PipelineStageFlagBits::eTopOfPipe,
                vk::PipelineStageFlagBits::eColorAttachmentOutput);

            vk::Extent2D mipExtent{ dst->GetWidth(), dst->GetHeight() };
            pCmdBuffer->BeginRendering(dst->GetImageView(), nullptr, nullptr, mipExtent, true, false);
            pCmdBuffer->BindPipeline(_downsamplePipeline.get());
            pCmdBuffer->SetViewportAndScissor(mipExtent.width, mipExtent.height);

            VulkanDescriptorAllocator::CacheKey key;
            key.layout = _singleSamplerLayout->GetDescriptorSetLayout();
            key.bindingHash = reinterpret_cast<size_t>(static_cast<VkImageView>(src->GetImageView()));
            auto descriptorSet = pDescriptorAllocator->AllocateDescriptorSet(_singleSamplerLayout.get(), &key);

            VulkanDescriptorWriter writer;
            writer.WriteImage(0, src->GetImageView(), _sampler->GetSampler());
            writer.UpdateSet(descriptorSet);

            std::vector<vk::DescriptorSet> sets{ descriptorSet };
            pCmdBuffer->BindDescriptorSet(_downsamplePipeline->GetPipelineLayout(), sets);

            struct DownsamplePushConstants { float texelSizeX; float texelSizeY; };
            DownsamplePushConstants pc{
                1.0f / static_cast<float>(src->GetWidth()),
                1.0f / static_cast<float>(src->GetHeight())
            };
            pCmdBuffer->GetBuffer().pushConstants(
                _downsamplePipeline->GetPipelineLayout(),
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

        // --- Step 3: Upsample chain ---
        // previousUpsample starts as downMips[MIP_COUNT-1] (smallest mip)
        // For i = MIP_COUNT-2 downto 0: upMips[i] = downMips[i] + upsample(previousUpsample)
        RTHandle* previousUpsample = _downMips[MIP_COUNT - 1];

        for (int i = MIP_COUNT - 2; i >= 0; --i)
        {
            RTHandle* coarser = previousUpsample;
            RTHandle* current = _downMips[i];
            RTHandle* dst = _upMips[i];

            pCmdBuffer->ImageMemoryBarrier(
                dst->GetImage(),
                vk::ImageLayout::eUndefined,
                vk::ImageLayout::eColorAttachmentOptimal,
                vk::AccessFlags{},
                vk::AccessFlagBits::eColorAttachmentWrite,
                vk::PipelineStageFlagBits::eTopOfPipe,
                vk::PipelineStageFlagBits::eColorAttachmentOutput);

            vk::Extent2D mipExtent{ dst->GetWidth(), dst->GetHeight() };
            pCmdBuffer->BeginRendering(dst->GetImageView(), nullptr, nullptr, mipExtent, true, false);
            pCmdBuffer->BindPipeline(_upsamplePipeline.get());
            pCmdBuffer->SetViewportAndScissor(mipExtent.width, mipExtent.height);

            VulkanDescriptorAllocator::CacheKey key;
            key.layout = _dualSamplerLayout->GetDescriptorSetLayout();
            key.bindingHash = reinterpret_cast<size_t>(static_cast<VkImageView>(coarser->GetImageView()))
                ^ (reinterpret_cast<size_t>(static_cast<VkImageView>(current->GetImageView())) << 1);
            auto descriptorSet = pDescriptorAllocator->AllocateDescriptorSet(_dualSamplerLayout.get(), &key);

            VulkanDescriptorWriter writer;
            writer.WriteImage(0, coarser->GetImageView(), _sampler->GetSampler());
            writer.WriteImage(1, current->GetImageView(), _sampler->GetSampler());
            writer.UpdateSet(descriptorSet);

            std::vector<vk::DescriptorSet> sets{ descriptorSet };
            pCmdBuffer->BindDescriptorSet(_upsamplePipeline->GetPipelineLayout(), sets);

            struct UpsamplePushConstants { float texelSizeX; float texelSizeY; float blendFactor; };
            UpsamplePushConstants pc{
                1.0f / static_cast<float>(coarser->GetWidth()),
                1.0f / static_cast<float>(coarser->GetHeight()),
                _blendFactor
            };
            pCmdBuffer->GetBuffer().pushConstants(
                _upsamplePipeline->GetPipelineLayout(),
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

            previousUpsample = dst;
        }

        // --- Step 4: Composite (upMips[0] + original → outputImageView) ---
        {
            RTHandle* bloomResult = _upMips[0];

            pCmdBuffer->BeginRendering(outputImageView, nullptr, nullptr, extent, true, false);
            pCmdBuffer->BindPipeline(_compositePipeline.get());
            pCmdBuffer->SetViewportAndScissor(extent.width, extent.height);

            VulkanDescriptorAllocator::CacheKey key;
            key.layout = _dualSamplerLayout->GetDescriptorSetLayout();
            key.bindingHash = reinterpret_cast<size_t>(static_cast<VkImageView>(bloomResult->GetImageView()))
                ^ (reinterpret_cast<size_t>(static_cast<VkImageView>(inputImageView)) << 1);
            auto descriptorSet = pDescriptorAllocator->AllocateDescriptorSet(_dualSamplerLayout.get(), &key);

            VulkanDescriptorWriter writer;
            writer.WriteImage(0, bloomResult->GetImageView(), _sampler->GetSampler());
            writer.WriteImage(1, inputImageView, _sampler->GetSampler());
            writer.UpdateSet(descriptorSet);

            std::vector<vk::DescriptorSet> sets{ descriptorSet };
            pCmdBuffer->BindDescriptorSet(_compositePipeline->GetPipelineLayout(), sets);

            pCmdBuffer->GetBuffer().pushConstants(
                _compositePipeline->GetPipelineLayout(),
                vk::ShaderStageFlagBits::eFragment,
                0, sizeof(float), &_bloomIntensity);

            pCmdBuffer->DrawNonIndexed(3);
            pCmdBuffer->EndRendering();
        }
    }

    void BloomKawaseEffect::OnResize(PostProcessResourcePool& pool, PostProcessPipelineFactory& factory,
        uint32_t width, uint32_t height, vk::Format format)
    {
        // RT handles are managed by the pool (it rebuilds them on resize).
        // Pipelines do not depend on resolution, so nothing to recreate.
    }

    void BloomKawaseEffect::Shutdown()
    {
        _thresholdPipeline = nullptr;
        _downsamplePipeline = nullptr;
        _upsamplePipeline = nullptr;
        _compositePipeline = nullptr;
        _singleSamplerLayout = nullptr;
        _dualSamplerLayout = nullptr;
        _sampler = nullptr;
    }
} // namespace Ailurus
