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

    /// @brief Post-process effect that applies HDR Tone Mapping (ACES) and Gamma Correction.
    class ToneMappingEffect : public PostProcessEffect
    {
    public:
        ToneMappingEffect();
        ~ToneMappingEffect() override;

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
        void SetExposure(float exposure)
        {
            if (_exposure == exposure)
                return;

            _exposure = exposure;
            NotifyChanged();
        }
        void SetGamma(float gamma)
        {
            if (_gamma == gamma)
                return;

            _gamma = gamma;
            NotifyChanged();
        }
        float GetExposure() const { return _exposure; }
        float GetGamma() const { return _gamma; }

    private:
        static const std::string EFFECT_NAME;
        static const char* FRAG_SHADER_PATH;

        struct PushConstants
        {
            float exposure = 1.0f;
            float gamma = 2.2f;
        };

        float _exposure = 1.0f;
        float _gamma = 2.2f;
        std::function<void()> _onChanged;

        std::unique_ptr<SamplerSchema> _descriptorSetLayout;
        std::unique_ptr<VulkanPipeline> _pipeline;
        VulkanSampler* _sampler = nullptr;

        void NotifyChanged() const
        {
            if (_onChanged)
                _onChanged();
        }
    };
} // namespace Ailurus
