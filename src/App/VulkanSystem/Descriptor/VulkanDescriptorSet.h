#pragma once

#include <vulkan/vulkan.hpp>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>

namespace Ailurus
{
	class VulkanDescriptorSet: public NonCopyable, public NonMovable
	{
	public:
		VulkanDescriptorSet()
		{

		}

	private:
		vk::DescriptorSet _descriptorSet;
	};
}