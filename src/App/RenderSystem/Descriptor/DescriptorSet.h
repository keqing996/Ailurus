#pragma once

#include <vulkan/vulkan.hpp>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>

namespace Ailurus
{
	class DescriptorSet: public NonCopyable, public NonMovable
	{
	public:
		DescriptorSet()
		{

		}

	private:
		vk::DescriptorSet _descriptorSet;
	};
}