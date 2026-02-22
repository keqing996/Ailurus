#include "Ailurus/Application/RenderSystem/PostProcess/PostProcessResourcePool.h"
#include "VulkanContext/RenderTarget/RenderTarget.h"
#include "Ailurus/Utility/Logger.h"
#include <cmath>

namespace Ailurus
{
    RTHandle* PostProcessResourcePool::RegisterRT(const RTSpec& spec)
    {
        auto handle = std::make_unique<RTHandle>();
        handle->_spec = spec;
        RTHandle* pRaw = handle.get();
        _handles.push_back(std::move(handle));
        return pRaw;
    }

    void PostProcessResourcePool::Build(uint32_t baseWidth, uint32_t baseHeight)
    {
        for (auto& handle : _handles)
            BuildHandle(*handle, baseWidth, baseHeight);
    }

    void PostProcessResourcePool::Rebuild(uint32_t baseWidth, uint32_t baseHeight)
    {
        for (auto& handle : _handles)
        {
            handle->_rt = nullptr; // Destroy old RT
            BuildHandle(*handle, baseWidth, baseHeight);
        }
    }

    void PostProcessResourcePool::Shutdown()
    {
        for (auto& handle : _handles)
            handle->_rt = nullptr;
        _handles.clear();
    }

    void PostProcessResourcePool::BuildHandle(RTHandle& handle, uint32_t baseWidth, uint32_t baseHeight)
    {
        const uint32_t w = static_cast<uint32_t>(std::max(1.0f, std::round(static_cast<float>(baseWidth) * handle._spec.widthScale)));
        const uint32_t h = static_cast<uint32_t>(std::max(1.0f, std::round(static_cast<float>(baseHeight) * handle._spec.heightScale)));

        handle._width = w;
        handle._height = h;

        RenderTargetConfig config;
        config.width = w;
        config.height = h;
        config.format = handle._spec.format;
        config.samples = vk::SampleCountFlagBits::e1;
        config.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
        config.aspectMask = vk::ImageAspectFlagBits::eColor;
        config.transient = false;

        try
        {
            handle._rt = std::make_unique<RenderTarget>(config);
        }
        catch (const std::exception& e)
        {
            Logger::LogError("PostProcessResourcePool: Failed to create RT: {}", e.what());
        }
    }
} // namespace Ailurus
