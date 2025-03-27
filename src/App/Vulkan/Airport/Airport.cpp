#include "Airport.h"
#include "Ailurus/Utility/Logger.h"
#include "Vulkan/Context/VulkanContext.h"
#include "Vulkan/SwapChain/SwapChain.h"

namespace Ailurus
{
    Airport::Airport()
    {
        vk::CommandBufferAllocateInfo allocInfo;
        allocInfo.setCommandPool(VulkanContext::GetCommandPool())
                .setLevel(vk::CommandBufferLevel::ePrimary)
                .setCommandBufferCount(MAX_FLIGHTS);

        _vkCommandBuffers = VulkanContext::GetDevice().allocateCommandBuffers(allocInfo);

        vk::SemaphoreCreateInfo semaphoreInfo;

        vk::FenceCreateInfo fenceInfo;
        fenceInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);

        for (size_t i = 0; i < MAX_FLIGHTS; i++)
        {
            _vkImageReadySemaphore.push_back(VulkanContext::GetDevice().createSemaphore(semaphoreInfo));
            _vkFinishRenderSemaphore.push_back(VulkanContext::GetDevice().createSemaphore(semaphoreInfo));
            _vkFences.push_back(VulkanContext::GetDevice().createFence(fenceInfo));
        }
    }

    Airport::~Airport()
    {
        VulkanContext::GetDevice().waitIdle();

        for (const auto& fence: _vkFences)
            VulkanContext::GetDevice().destroyFence(fence);

        for (const auto& sem: _vkImageReadySemaphore)
            VulkanContext::GetDevice().destroySemaphore(sem);

        for (const auto& sem: _vkFinishRenderSemaphore)
            VulkanContext::GetDevice().destroySemaphore(sem);

        VulkanContext::GetDevice().freeCommandBuffers(VulkanContext::GetCommandPool(), _vkCommandBuffers);
    }

    std::optional<Flight> Airport::WaitNextFlight(bool* needRebuild) const
	{
        Flight flight;
        flight.commandBuffer = _vkCommandBuffers[_currentFlightIndex];
        flight.imageReadySemaphore = _vkImageReadySemaphore[_currentFlightIndex];
        flight.renderFinishSemaphore = _vkFinishRenderSemaphore[_currentFlightIndex];
        flight.fence = _vkFences[_currentFlightIndex];

    	// Wait fence
    	auto waitFence = VulkanContext::GetDevice().waitForFences(flight.fence, true,
			std::numeric_limits<uint64_t>::max());
    	if (waitFence != vk::Result::eSuccess)
    	{
    		Logger::LogError("Fail to wait fences, result = {}", static_cast<int>(waitFence));
    		return std::nullopt;
    	}

        // Acquire swap chain next image
        auto acquireImage = VulkanContext::GetDevice().acquireNextImageKHR(
            VulkanContext::GetSwapChain()->GetSwapChain(), std::numeric_limits<uint64_t>::max(), flight.imageReadySemaphore);

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

    	VulkanContext::GetDevice().resetFences(flight.fence);

        return flight;
    }

    bool Airport::TakeOff(const Flight& flight, bool* needRebuild)
    {
        std::array<vk::PipelineStageFlags, 1> waitStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

        vk::SubmitInfo submitInfo;
        submitInfo.setWaitSemaphores(flight.imageReadySemaphore)
                .setSignalSemaphores(flight.renderFinishSemaphore)
                .setWaitDstStageMask(waitStages)
                .setCommandBuffers(flight.commandBuffer);

        VulkanContext::GetGraphicQueue().submit(submitInfo, flight.fence);

        vk::PresentInfoKHR presentInfo;
        presentInfo.setWaitSemaphores(flight.renderFinishSemaphore)
                .setSwapchains(VulkanContext::GetSwapChain()->GetSwapChain())
                .setImageIndices(flight.imageIndex);

        const auto present = VulkanContext::GetPresentQueue().presentKHR(presentInfo);
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
