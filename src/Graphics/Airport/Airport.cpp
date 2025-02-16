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

        for (const auto& fence: _vkFences)
            _pRenderer->GetLogicalDevice().destroyFence(fence);

        _pRenderer->GetLogicalDevice().freeCommandBuffers(_pRenderer->GetCommandPool(), _vkCommandBuffers);
    }

    Flight Airport::GetNextFlight() const
    {
        return Flight{
            _vkCommandBuffers[_currentFlightIndex],
            _vkImageReadySemaphore[_currentFlightIndex],
            _vkFinishRenderSemaphore[_currentFlightIndex],
            _vkFences[_currentFlightIndex]
        };
    }

    void Airport::WaitFlightReady(const Flight& flight) const
    {
        auto waitFence = _pRenderer->GetLogicalDevice().waitForFences(
            flight.fence, true, std::numeric_limits<uint64_t>::max());
        if (waitFence != vk::Result::eSuccess)
        {
            Logger::LogError("Fail to wait fences, result = {}", static_cast<int>(waitFence));
            return;
        }

        _pRenderer->GetLogicalDevice().resetFences(flight.fence);
    }

    void Airport::UpdateFlightPlan()
    {
        _currentFlightIndex = (_currentFlightIndex + 1) % MAX_FLIGHTS;
    }
}
