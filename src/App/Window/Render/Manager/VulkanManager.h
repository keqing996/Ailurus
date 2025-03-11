#pragma once

#include <functional>
#include <memory>
#include <vulkan/vulkan.hpp>
#include "Ailurus/Math/Vector.hpp"
#include "SwapChain/SwapChain.h"

namespace Ailurus
{
    class VulkanManager
    {
    public:
        VulkanManager();
        ~VulkanManager();

    public:
        void RenderScene();
        void NeedRecreateSwapChain();

    private:
        void CreateDynamicContext();
        void DestroyDynamicContext();

    private:
        bool _needRebuildSwapChain;

        std::unique_ptr<SwapChain> _pSwapChain = nullptr;
        std::unique_ptr<class RenderPassForward> _pForwardPass = nullptr;
        std::unique_ptr<class Airport> _pAirport = nullptr;
    };
}