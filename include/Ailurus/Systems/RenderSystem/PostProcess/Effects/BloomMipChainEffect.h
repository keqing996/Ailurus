#pragma once

#include <functional>
#include <string>
#include <memory>
#include <vulkan/vulkan.hpp>
#include <Ailurus/Systems/RenderSystem/PostProcess/PostProcessEffect.h>

namespace Ailurus
{
    class VulkanPipeline;
    class SamplerSchema;
    class VulkanSampler;
    class RTHandle;

    /// @brief Bloom post-process effect using a progressive mip-chain downsample/upsample approach.
    /// Uses a 13-tap downsample filter and a 9-tap tent filter upsample.
    /// Must be placed before ToneMappingEffect in the post-process chain (operates on HDR data).
    class BloomMipChainEffect : public PostProcessEffect
    {
    public:
        BloomMipChainEffect();
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

        void SetChangeCallback(const std::function<void()>& callback) { _onChanged = callback; }
        void SetThreshold(float threshold)
        {
            if (_threshold == threshold)
                return;

            _threshold = threshold;
            NotifyChanged();
        }
        void SetSoftKnee(float softKnee)
        {
            if (_softKnee == softKnee)
                return;

            _softKnee = softKnee;
            NotifyChanged();
        }
        void SetBloomIntensity(float intensity)
        {
            if (_bloomIntensity == intensity)
                return;

            _bloomIntensity = intensity;
            NotifyChanged();
        }
        void SetBlendFactor(float factor)
        {
            if (_blendFactor == factor)
                return;

            _blendFactor = factor;
            NotifyChanged();
        }

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
        std::function<void()> _onChanged;

        // Render target handles (stable pointers into the resource pool)
        RTHandle* _downMips[MIP_COUNT] = {};
        RTHandle* _upMips[MIP_COUNT - 1] = {};

        // Descriptor set layouts
        std::unique_ptr<SamplerSchema> _singleSamplerLayout;
        std::unique_ptr<SamplerSchema> _dualSamplerLayout;

        // Pipelines
        std::unique_ptr<VulkanPipeline> _thresholdPipeline;
        std::unique_ptr<VulkanPipeline> _downsamplePipeline;
        std::unique_ptr<VulkanPipeline> _upsamplePipeline;
        std::unique_ptr<VulkanPipeline> _compositePipeline;

        VulkanSampler* _sampler = nullptr;

        void NotifyChanged() const
        {
            if (_onChanged)
                _onChanged();
        }
    };
} // namespace Ailurus
