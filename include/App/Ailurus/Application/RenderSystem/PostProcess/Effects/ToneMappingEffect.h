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

    /// @brief Post-process effect that applies HDR Tone Mapping (ACES) and Gamma Correction.
    class ToneMappingEffect : public PostProcessEffect
    {
    public:
        ToneMappingEffect() = default;
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

        void SetExposure(float exposure) { _exposure = exposure; }
        void SetGamma(float gamma) { _gamma = gamma; }
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

        std::unique_ptr<VulkanDescriptorSetLayout> _descriptorSetLayout;
        std::unique_ptr<VulkanPipeline> _pipeline;
        VulkanSampler* _sampler = nullptr;
    };
} // namespace Ailurus
