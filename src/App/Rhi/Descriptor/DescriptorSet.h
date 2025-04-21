#pragma once

#include <vulkan/vulkan.hpp>
#include <Ailurus/Utility/NonCopyable.h>

namespace Ailurus
{
	class DescriptorSet: public NonCopyable
	{
	public:
		DescriptorSet()
		{

		}

	private:
		vk::DescriptorSet _descriptorSet;
	};
}