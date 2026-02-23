#pragma once

#include <string>
#include <memory>
#include <vulkan/vulkan.hpp>
#include <Ailurus/Application/RenderSystem/PostProcess/PostProcessEffect.h>

namespace Ailurus
{
    class VulkanPipeline;
    class VulkanDescriptorSetLayout;
    class VulkanSampler;
    class RTHandle;

    /// @brief Bloom post-process effect using a progressive mip-chain downsample/upsample approach.
    /// Uses a 13-tap downsample filter and a 9-tap tent filter upsample.
    /// Must be placed before ToneMappingEffect in the post-process chain (operates on HDR data).
    class BloomMipChainEffect : public PostProcessEffect
    {
    public:
        BloomMipChainEffect() = default;
        ~BloomMipChainEffect() override;

    public:
        const std::string& GetName() const override;

        void Init(PostProcessResourcePool& pool, PostProcessPipelineFactory& factory,
            uint32_t width, uint32_t height, vk::Format format) override;

        void Render(VulkanCommandBuffer* pCmdBuffer, vk::ImageView inputImageView,
            vk::ImageView outputImageView, vk::Extent2D extent,
            VulkanDescriptorAllocator* pDescriptorAllocator) override;

        void OnResize(PostProcessResourcePool& pool, PostProcessPipelineFactory& factory,
            uint32_t width, uint32_t height, vk::Format format) override;

        void Shutdown() override;

        void SetThreshold(float threshold) { _threshold = threshold; }
        void SetSoftKnee(float softKnee) { _softKnee = softKnee; }
        void SetBloomIntensity(float intensity) { _bloomIntensity = intensity; }
        void SetBlendFactor(float factor) { _blendFactor = factor; }

        float GetThreshold() const { return _threshold; }
        float GetSoftKnee() const { return _softKnee; }
        float GetBloomIntensity() const { return _bloomIntensity; }
        float GetBlendFactor() const { return _blendFactor; }

    private:
        static const std::string EFFECT_NAME;
        static const char* THRESHOLD_SHADER_PATH;
        static const char* DOWNSAMPLE_SHADER_PATH;
        static const char* UPSAMPLE_SHADER_PATH;
        static const char* COMPOSITE_SHADER_PATH;

        static constexpr int MIP_COUNT = 5;

        float _threshold = 1.0f;
        float _softKnee = 0.5f;
        float _bloomIntensity = 0.5f;
        float _blendFactor = 0.7f;

        // Render target handles (stable pointers into the resource pool)
        RTHandle* _downMips[MIP_COUNT] = {};
        RTHandle* _upMips[MIP_COUNT - 1] = {};

        // Descriptor set layouts
        std::unique_ptr<VulkanDescriptorSetLayout> _singleSamplerLayout;
        std::unique_ptr<VulkanDescriptorSetLayout> _dualSamplerLayout;

        // Pipelines
        std::unique_ptr<VulkanPipeline> _thresholdPipeline;
        std::unique_ptr<VulkanPipeline> _downsamplePipeline;
        std::unique_ptr<VulkanPipeline> _upsamplePipeline;
        std::unique_ptr<VulkanPipeline> _compositePipeline;

        VulkanSampler* _sampler = nullptr;
    };
} // namespace Ailurus
