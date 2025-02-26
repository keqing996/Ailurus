#pragma once

#include <vulkan/vulkan.hpp>
#include "RenderPassType.h"
#include "Ailurus/Graphics/Airport/Flight.h"

namespace Ailurus
{
    class Renderer;
    class SwapChain;

    class RenderPass
    {
    public:
        explicit RenderPass(const Renderer* pRenderer) : _pRenderer(pRenderer) {}
        virtual ~RenderPass() = 0;

    public:
        virtual vk::RenderPass GetRenderPass() const = 0;
        virtual vk::RenderPassBeginInfo GetRenderPassBeginInfo(const Flight& flight) = 0;

    protected:
        const Renderer* _pRenderer;
    };
}