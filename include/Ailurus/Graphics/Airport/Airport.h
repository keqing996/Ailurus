#pragma once

#include <cstdint>
#include <vector>
#include <optional>
#include "Flight.h"

namespace Ailurus
{
    class Renderer;

    class Airport
    {
    public:
        explicit Airport(const Renderer* pRenderer);
        ~Airport();

    public:
        Flight GetNextFlight() const;
        void WaitFlightReady(const Flight& flight) const;
        void UpdateFlightPlan();

        inline static uint32_t MAX_FLIGHTS = 2;

    private:
        const Renderer* _pRenderer;

        std::vector<vk::CommandBuffer> _vkCommandBuffers;
        std::vector<vk::Semaphore> _vkImageReadySemaphore;
        std::vector<vk::Semaphore> _vkFinishRenderSemaphore;
        std::vector<vk::Fence> _vkFences;

        uint32_t _currentFlightIndex = 0;
    };
}