#include "Ailurus/Application/RenderSystem/PostProcess/RTHandle.h"
#include "VulkanContext/RenderTarget/RenderTarget.h"

namespace Ailurus
{
    vk::Image RTHandle::GetImage() const
    {
        return _rt ? _rt->GetImage() : nullptr;
    }

    vk::ImageView RTHandle::GetImageView() const
    {
        return _rt ? _rt->GetImageView() : nullptr;
    }

    uint32_t RTHandle::GetWidth() const
    {
        return _width;
    }

    uint32_t RTHandle::GetHeight() const
    {
        return _height;
    }

    bool RTHandle::IsValid() const
    {
        return _rt != nullptr && _rt->IsValid();
    }
} // namespace Ailurus
