#pragma once

#include <Ailurus/Utility/NonCopyable.h>
#include <vulkan/vulkan.hpp>

namespace Ailurus
{
	struct DescriptorPoolCapacityConfig
	{
		size_t maxSets;
		size_t uniformCapacity;
		size_t samplerCapacity;
	};

	class FrameDescriptorPool : public NonCopyable
	{
	public:
		explicit FrameDescriptorPool(const DescriptorPoolCapacityConfig& capacity);
		~FrameDescriptorPool() override;

	public:
		vk::DescriptorSet AllocateDescriptorSet(const vk::DescriptorSetLayout& layout);

	private:
		uint32_t _capacity;
		vk::DescriptorPool _descriptorPool;
	};
} // namespace Ailurus