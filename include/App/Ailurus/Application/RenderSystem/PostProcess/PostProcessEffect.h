#pragma once

#include <string>
#include <cstdint>
#include <vulkan/vulkan.hpp>

namespace Ailurus
{
    class VulkanCommandBuffer;
    class VulkanDescriptorAllocator;
    class PostProcessResourcePool;
    class PostProcessPipelineFactory;

    /// @brief Abstract base class for individual post-process effects.
    /// Each effect is an independent class that owns its own Pipeline(s) and DescriptorSetLayout.
    class PostProcessEffect
    {
    public:
        virtual ~PostProcessEffect() = default;

        /// @brief Get the unique name of this effect.
        virtual const std::string& GetName() const = 0;

        /// @brief Initialize the effect. Called once after registration.
        virtual void Init(PostProcessResourcePool& pool, PostProcessPipelineFactory& factory,
            uint32_t width, uint32_t height, vk::Format format) = 0;

        /// @brief Render this effect.
        /// @param pCmdBuffer Command buffer to record into
        /// @param inputImageView Input image view (in ShaderReadOnly layout)
        /// @param outputImageView Output image view (in ColorAttachment layout)
        /// @param extent Render extent
        /// @param pDescriptorAllocator Per-frame descriptor allocator
        virtual void Render(VulkanCommandBuffer* pCmdBuffer, vk::ImageView inputImageView,
            vk::ImageView outputImageView, vk::Extent2D extent,
            VulkanDescriptorAllocator* pDescriptorAllocator) = 0;

        /// @brief Rebuild resources when the swapchain is resized.
        virtual void OnResize(PostProcessResourcePool& pool, PostProcessPipelineFactory& factory,
            uint32_t width, uint32_t height, vk::Format format) = 0;

        /// @brief Release all GPU resources.
        virtual void Shutdown() = 0;

        bool IsEnabled() const { return _enabled; }
        void SetEnabled(bool enabled) { _enabled = enabled; }

    protected:
        bool _enabled = true;
    };
} // namespace Ailurus
