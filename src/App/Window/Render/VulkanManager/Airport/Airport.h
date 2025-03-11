#pragma once

#include <cstdint>
#include <optional>
#include <vector>
#include "Flight.h"

namespace Ailurus
{
    class Render;
    class SwapChain;

    class Airport
    {
    public:
        explicit Airport(const Render* pRenderer);
        ~Airport();

    public:
        std::optional<Flight> WaitNextFlight(const SwapChain* pSwapChain, bool* needRebuild);
        bool TakeOff(const Flight& flight, const SwapChain* pSwapChain, bool* needRebuild);

        inline static uint32_t MAX_FLIGHTS = 2;

    private:
        const Render* _pRenderer;

        std::vector<vk::CommandBuffer> _vkCommandBuffers;
        std::vector<vk::Semaphore> _vkImageReadySemaphore;
        std::vector<vk::Semaphore> _vkFinishRenderSemaphore;
        std::vector<vk::Fence> _vkFences;

        uint32_t _currentFlightIndex = 0;
    };
}