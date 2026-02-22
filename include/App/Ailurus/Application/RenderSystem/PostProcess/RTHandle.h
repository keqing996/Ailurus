#pragma once

#include <memory>
#include <cstdint>
#include <vulkan/vulkan.hpp>
#include "RTSpec.h"

namespace Ailurus
{
    class RenderTarget;

    /// @brief Stable handle to a RenderTarget managed by PostProcessResourcePool.
    /// The internal RenderTarget pointer is updated transparently on resize while
    /// the RTHandle* pointer itself remains stable.
    class RTHandle
    {
    public:
        vk::Image GetImage() const;
        vk::ImageView GetImageView() const;
        uint32_t GetWidth() const;
        uint32_t GetHeight() const;
        bool IsValid() const;

    private:
        friend class PostProcessResourcePool;
        RTSpec _spec;
        std::unique_ptr<RenderTarget> _rt;
        uint32_t _width = 0;
        uint32_t _height = 0;
    };
} // namespace Ailurus
