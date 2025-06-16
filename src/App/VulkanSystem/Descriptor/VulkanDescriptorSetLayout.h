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

	public:
		vk::DescriptorSetLayout GetDescriptorSetLayout() const;

	private:
		vk::DescriptorSetLayout _descriptorSetLayout;
	};
}