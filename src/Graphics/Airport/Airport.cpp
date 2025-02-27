#include "Ailurus/Graphics/Airport/Airport.h"
#include "Ailurus/Utility/Logger.h"
#include "Ailurus/Graphics/Renderer.h"

namespace Ailurus
{
    Airport::Airport(const Renderer* pRenderer)
        : _pRenderer(pRenderer)
    {
        vk::CommandBufferAllocateInfo allocInfo;
        allocInfo.setCommandPool(_pRenderer->GetCommandPool())
                .setLevel(vk::CommandBufferLevel::ePrimary)
                .setCommandBufferCount(MAX_FLIGHTS);

        _vkCommandBuffers = _pRenderer->GetLogicalDevice().allocateCommandBuffers(allocInfo);

        vk::SemaphoreCreateInfo semaphoreInfo;

        vk::FenceCreateInfo fenceInfo;
        fenceInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);

        for (size_t i = 0; i < MAX_FLIGHTS; i++)
        {
            _vkImageReadySemaphore.push_back(_pRenderer->GetLogicalDevice().createSemaphore(semaphoreInfo));
            _vkFinishRenderSemaphore.push_back(_pRenderer->GetLogicalDevice().createSemaphore(semaphoreInfo));
            _vkFences.push_back(_pRenderer->GetLogicalDevice().createFence(fenceInfo));
        }
    }

    Airport::~Airport()
    {
        _pRenderer->GetLogicalDevice().waitIdle();

        for (const auto& fence: _vkFences)
            _pRenderer->GetLogicalDevice().destroyFence(fence);

        for (const auto& sem: _vkImageReadySemaphore)
            _pRenderer->GetLogicalDevice().destroySemaphore(sem);

        for (const auto& sem: _vkFinishRenderSemaphore)
            _pRenderer->GetLogicalDevice().destroySemaphore(sem);

        _pRenderer->GetLogicalDevice().freeCommandBuffers(_pRenderer->GetCommandPool(), _vkCommandBuffers);
    }

    std::optional<Flight> Airport::WaitNextFlight(const SwapChain* pSwapChain, bool* needRebuild)
    {
        Flight flight(_pRenderer);
        flight.commandBuffer = _vkCommandBuffers[_currentFlightIndex];
        flight.imageReadySemaphore = _vkImageReadySemaphore[_currentFlightIndex];
        flight.renderFinishSemaphore = _vkFinishRenderSemaphore[_currentFlightIndex];
        flight.fence = _vkFences[_currentFlightIndex];

        // Acquire swap chain next image
        auto acquireImage = _pRenderer->GetLogicalDevice().acquireNextImageKHR(
            pSwapChain->GetSwapChain(), std::numeric_limits<uint64_t>::max(), flight.imageReadySemaphore);

        switch (acquireImage.result)
        {
            case vk::Result::eErrorOutOfDateKHR:
                *needRebuild = true;
                return std::nullopt;
            case vk::Result::eSuboptimalKHR:
                *needRebuild = true;
                break;
            case vk::Result::eSuccess:
                break;
            default:
                Logger::LogError("Fail to acquire next image, result = {}", static_cast<int>(acquireImage.result));
                return std::nullopt;
        }

        flight.imageIndex = acquireImage.value;

        // Wait fence
        auto waitFence = _pRenderer->GetLogicalDevice().waitForFences( flight.fence, true,
            std::numeric_limits<uint64_t>::max());
        if (waitFence != vk::Result::eSuccess)
        {
            Logger::LogError("Fail to wait fences, result = {}", static_cast<int>(waitFence));
            return std::nullopt;
        }

        _pRenderer->GetLogicalDevice().resetFences(flight.fence);

        return flight;
    }

    bool Airport::TakeOff(const Flight& flight, const SwapChain* pSwapChain, bool* needRebuild)
    {
        std::array<vk::PipelineStageFlags, 1> waitStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

        vk::SubmitInfo submitInfo;
        submitInfo.setWaitSemaphores(flight.imageReadySemaphore)
                .setSignalSemaphores(flight.renderFinishSemaphore)
                .setWaitDstStageMask(waitStages)
                .setCommandBuffers(flight.commandBuffer);

        _pRenderer->GetGraphicQueue().submit(submitInfo, flight.fence);

        vk::PresentInfoKHR presentInfo;
        presentInfo.setWaitSemaphores(flight.renderFinishSemaphore)
                .setSwapchains(pSwapChain->GetSwapChain())
                .setImageIndices(flight.imageIndex);

        auto present = _pRenderer->GetPresentQueue().presentKHR(presentInfo);
        switch (present)
        {
            case vk::Result::eErrorOutOfDateKHR:
                *needRebuild = true;
                return false;
            case vk::Result::eSuboptimalKHR:
                *needRebuild = true;
                break;
            case vk::Result::eSuccess:
                break;
            default:
                Logger::LogError("Fail to present, result = {}", static_cast<int>(present));
                return false;
        }

        _currentFlightIndex = (_currentFlightIndex + 1) % MAX_FLIGHTS;

        return true;
    }
}
