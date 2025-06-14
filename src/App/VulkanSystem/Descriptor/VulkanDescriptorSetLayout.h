#pragma once

#include <vulkan/vulkan.hpp>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>

namespace Ailurus
{
	class VulkanDescriptorSetLayout: public NonCopyable, public NonMovable
	{
	public:
		explicit VulkanDescriptorSetLayout(class UniformSet* pUniformSet);
		~VulkanDescriptorSetLayout();

	private:
		vk::DescriptorSetLayout _descriptorSetLayout;
	};
}