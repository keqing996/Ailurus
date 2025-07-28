#pragma once

#include <vulkan/vulkan.hpp>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>
#include "FrameContext.h"

namespace Ailurus
{
	class VulkanFlightManager : public NonCopyable, public NonMovable
	{
	public:
		VulkanFlightManager();
		~VulkanFlightManager();

	public:
	

	private:
		uint32_t _currentParallelFrameIndex = 0;
		std::vector<std::unique_ptr<FrameContext>> _frameContexts {};
	};
} // namespace Ailurus