#include "../Vulkan/VulkanManager.h"
#include "../../../../../include/App/Ailurus/Window/Window.h"

namespace Ailurus
{
    VulkanManager::VulkanManager()
    {
        CreateDynamicContext();
    }

    VulkanManager::~VulkanManager()
    {
        DestroyDynamicContext();
    }

    void VulkanManager::CreateDynamicContext()
    {
        Vector2i windowSize = Window::GetSize();
        _pSwapChain = std::make_unique<SwapChain>(this, windowSize.x(), windowSize.y());
        _pForwardPass = std::make_unique<RenderPassForward>(this);
        _pAirport = std::make_unique<Airport>(this);
    }

    void VulkanManager::DestroyDynamicContext()
    {
        _pAirport.reset();
        _pForwardPass.reset();
        _pSwapChain.reset();
    }
}
