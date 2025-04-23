#pragma once

#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>
#include <vulkan/vulkan.hpp>

namespace Ailurus
{
	struct DescriptorPoolCapacityConfig
	{
		size_t maxSets;
		size_t uniformCapacity;
		size_t samplerCapacity;
	};

	class VulkanDescriptorPool : public NonCopyable, public NonMovable
	{
	public:
		explicit VulkanDescriptorPool(const DescriptorPoolCapacityConfig& capacity);
		~VulkanDescriptorPool();

	public:
		vk::DescriptorSet AllocateDescriptorSet(const vk::DescriptorSetLayout& layout);

	private:
		uint32_t _capacity;
		vk::DescriptorPool _descriptorPool;
	};
} // namespace Ailurus