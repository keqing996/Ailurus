#pragma once

#include <memory>
#include <vector>
#include <string>
#include <cstdint>
#include <vulkan/vulkan.hpp>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>
#include "PostProcessEffect.h"
#include "PostProcessResourcePool.h"
#include "PostProcessPipelineFactory.h"

namespace Ailurus
{
    class VulkanCommandBuffer;
    class VulkanDescriptorAllocator;
    class ShaderLibrary;

    /// @brief Manages an ordered chain of post-process effects.
    /// Orchestrates ping-pong execution flow between effects.
    class PostProcessChain : public NonCopyable, public NonMovable
    {
    public:
        /// @brief Initialize the chain. Must be called before any rendering.
        void Init(ShaderLibrary* pShaderLibrary, uint32_t width, uint32_t height, vk::Format format);

        /// @brief Execute all enabled effects in order.
        /// Reads from inputImageView (must be in ShaderReadOnly) and writes to outputImageView (must be in ColorAttachment).
        void Execute(VulkanCommandBuffer* pCmdBuffer,
            vk::Image inputImage, vk::ImageView inputImageView,
            vk::Image outputImage, vk::ImageView outputImageView,
            vk::Extent2D extent, VulkanDescriptorAllocator* pDescriptorAllocator);

        /// @brief Rebuild resources after swapchain resize.
        void OnResize(uint32_t width, uint32_t height, vk::Format format);

        /// @brief Shutdown and release all resources.
        void Shutdown();

        /// @brief Add an effect to the end of the chain.
        template <typename T, typename... Args>
        T* AddEffect(Args&&... args)
        {
            auto pEffect = std::make_unique<T>(std::forward<Args>(args)...);
            T* pRaw = pEffect.get();
            if (_initialized)
                pRaw->Init(_resourcePool, _factory, _width, _height, _format);
            _effects.push_back(std::move(pEffect));
            return pRaw;
        }

        /// @brief Insert an effect at a specific index.
        template <typename T, typename... Args>
        T* InsertEffect(size_t index, Args&&... args)
        {
            auto pEffect = std::make_unique<T>(std::forward<Args>(args)...);
            T* pRaw = pEffect.get();
            if (_initialized)
                pRaw->Init(_resourcePool, _factory, _width, _height, _format);
            _effects.insert(_effects.begin() + static_cast<ptrdiff_t>(index), std::move(pEffect));
            return pRaw;
        }

        /// @brief Remove an effect by name.
        void RemoveEffect(const std::string& name);

        /// @brief Remove an effect by index.
        void RemoveEffect(size_t index);

        /// @brief Get an effect by name. Returns nullptr if not found.
        PostProcessEffect* GetEffect(const std::string& name);

        /// @brief Returns true if there is at least one enabled effect in the chain.
        bool HasEnabledEffects() const;

    private:
        std::vector<std::unique_ptr<PostProcessEffect>> _effects;
        PostProcessResourcePool _resourcePool;
        PostProcessPipelineFactory _factory;

        uint32_t _width = 0;
        uint32_t _height = 0;
        vk::Format _format = vk::Format::eUndefined;
        bool _initialized = false;

        // Ping-pong intermediate render targets (registered with the resource pool)
        RTHandle* _pPingRT = nullptr;
        RTHandle* _pPongRT = nullptr;
    };
} // namespace Ailurus
