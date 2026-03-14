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
    class RTHandle;

    class SSAOEffect : public PostProcessEffect
    {
    public:
        SSAOEffect();
        ~SSAOEffect() override;

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

        // Camera parameters needed for depth reconstruction
        void SetProjectionMatrix(const Matrix4x4f& projMatrix);
        void SetDepthImageViewOverride(vk::ImageView depthImageView) { _depthImageViewOverride = depthImageView; }

        void SetRadius(float radius) { _radius = radius; }
        void SetBias(float bias) { _bias = bias; }
        void SetPower(float power) { _power = power; }
        void SetKernelSize(int size) { _kernelSize = size; }
        void SetStrength(float strength) { _strength = strength; }

        float GetRadius() const { return _radius; }
        float GetBias() const { return _bias; }
        float GetPower() const { return _power; }
        int GetKernelSize() const { return _kernelSize; }
        float GetStrength() const { return _strength; }

    private:
        static const std::string EFFECT_NAME;
        static const char* GENERATE_SHADER_PATH;
        static const char* BLUR_SHADER_PATH;
        static const char* COMPOSITE_SHADER_PATH;

        // SSAO parameters
        float _radius = 0.5f;
        float _bias = 0.025f;
        float _power = 2.0f;
        int _kernelSize = 32;
        float _strength = 1.0f;

        // Camera projection matrix
        Matrix4x4f _projectionMatrix;
        vk::ImageView _depthImageViewOverride = nullptr;

        // Intermediate render targets
        RTHandle* _ssaoRT = nullptr;
        RTHandle* _blurRT = nullptr;

        // Descriptor set layouts
        std::unique_ptr<SamplerSchema> _singleSamplerLayout;
        std::unique_ptr<SamplerSchema> _dualSamplerLayout;

        // Pipelines
        std::unique_ptr<VulkanPipeline> _generatePipeline;
        std::unique_ptr<VulkanPipeline> _blurPipeline;
        std::unique_ptr<VulkanPipeline> _compositePipeline;

        // Sampler
        VulkanSampler* _sampler = nullptr;
        VulkanSampler* _depthSampler = nullptr;
    };
} // namespace Ailurus
