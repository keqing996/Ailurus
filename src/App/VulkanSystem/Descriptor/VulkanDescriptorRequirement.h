#pragma once

#include <cstdint>
#include <array>
#include <vulkan/vulkan.hpp>

namespace Ailurus
{
	struct VulkanDescriptorRequirement
	{
		uint32_t uniformCount;
		uint32_t samplerCount;
	};
}