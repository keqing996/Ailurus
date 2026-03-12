#pragma once

#include "VulkanContext/VulkanPch.h"
#include <cstdint>

namespace Ailurus
{
    struct SwapChainConfig
	{
		vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
		vk::SurfaceFormatKHR surfaceFormat = vk::Format::eUndefined;
		vk::Extent2D extent;
		uint32_t imageCount;
	};
}