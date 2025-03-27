#pragma once

#include <cstdint>
#include <optional>
#include <vector>
#include "Flight.h"

namespace Ailurus
{
	class Airport
	{
	public:
		explicit Airport();
		~Airport();

	public:
		std::optional<Flight> WaitNextFlight(bool* needRebuild) const;
		bool TakeOff(const Flight& flight, bool* needRebuild);

		inline static uint32_t MAX_FLIGHTS = 2;

	private:
		std::vector<vk::CommandBuffer> _vkCommandBuffers;
		std::vector<vk::Semaphore> _vkImageReadySemaphore;
		std::vector<vk::Semaphore> _vkFinishRenderSemaphore;
		std::vector<vk::Fence> _vkFences;

		uint32_t _currentFlightIndex = 0;
	};
} // namespace Ailurus