#pragma once

#include <string>
#include <memory>
#include <vulkan/vulkan.hpp>
#include <Ailurus/Systems/RenderSystem/PostProcess/PostProcessEffect.h>
#include <Ailurus/Math/Matrix4x4.hpp>

namespace Ailurus
{
    class VulkanPipeline;
    class SamplerSchema;
    class VulkanSampler;
    class ShaderLibrary;

    /// @brief Deferred lighting pass.
    ///
    /// Reads the G-Buffer (Normal+AO, Albedo+Roughness, Metallic) and the
    /// scene depth buffer, then computes full PBR lighting in a single
    /// fullscreen quad, writing to the offscreen HDR render target.
    ///
    /// The effect reuses the existing global descriptor set (set 0) so that
    /// shadow maps and IBL data are available without duplication.
    class DeferredLightingEffect : public PostProcessEffect
    {
    public:
        DeferredLightingEffect();
        ~DeferredLightingEffect() override;

    public:
        const std::string& GetName() const override;

        // PostProcessEffect interface (not used in the main pipeline — call InitDeferred instead)
        void Init(PostProcessResourcePool& pool, PostProcessPipelineFactory& factory,
            uint32_t width, uint32_t height, vk::Format format) override;

        void Render(VulkanCommandBuffer* pCmdBuffer, vk::ImageView inputImageView,
            vk::ImageView outputImageView, vk::Extent2D extent,
            VulkanDescriptorAllocator* pDescriptorAllocator) override;

        void OnResize(PostProcessResourcePool& pool, PostProcessPipelineFactory& factory,
            uint32_t width, uint32_t height, vk::Format format) override;

        void Shutdown() override;

        /// @brief Dedicated initializer called by RenderSystem directly (bypasses PostProcessChain).
        /// Must be called after the global uniform set has been built.
        void InitDeferred(ShaderLibrary* pShaderLibrary,
            vk::DescriptorSetLayout globalDescSetLayout,
            vk::Format outputFormat);

        /// @brief Rebuild the pipeline (e.g., after swapchain resize changes the output format).
        void RebuildPipeline(ShaderLibrary* pShaderLibrary,
            vk::DescriptorSetLayout globalDescSetLayout,
            vk::Format outputFormat);

        // ---- Per-frame data setters (called by RenderSystem before Render()) ----
        void SetGBufferViews(vk::ImageView normalAO, vk::ImageView albedoRoughness, vk::ImageView metallic);
        void SetDepthImageView(vk::ImageView depth);
        void SetGlobalDescriptorSet(vk::DescriptorSet globalSet);
        void SetInverseViewProjMatrix(const Matrix4x4f& mat);

    private:
        static const std::string EFFECT_NAME;
        static const char*       FRAG_SHADER_PATH;

        struct PushConstants
        {
            Matrix4x4f inverseViewProj;
        };

        // Per-frame inputs (set each frame by RenderSystem)
        vk::ImageView     _gBufferNormalAO        = nullptr;
        vk::ImageView     _gBufferAlbedoRoughness = nullptr;
        vk::ImageView     _gBufferMetallic        = nullptr;
        vk::ImageView     _depthImageView         = nullptr;
        vk::DescriptorSet _globalDescriptorSet    = nullptr;
        Matrix4x4f        _inverseViewProj;

        // Descriptor set layout for set 1 (G-Buffer samplers)
        std::unique_ptr<SamplerSchema> _gBufferSamplerSchema;

        // Sampler for G-Buffer textures
        VulkanSampler* _sampler = nullptr;

        // Pipeline: layout has set 0 (global) + set 1 (G-Buffer)
        std::unique_ptr<VulkanPipeline> _pipeline;

        void BuildPipelineInternal(ShaderLibrary* pShaderLibrary,
            vk::DescriptorSetLayout globalDescSetLayout,
            vk::Format outputFormat);
    };
} // namespace Ailurus
