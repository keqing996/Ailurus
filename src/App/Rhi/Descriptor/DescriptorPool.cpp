#include <array>
#include "Rhi/VulkanContext.h"
#include "DescriptorPool.h"

namespace Ailurus
{
	FrameDescriptorPool::FrameDescriptorPool(const DescriptorPoolCapacityConfig& capacityConfig)
	{
		// Uniform pool size
		vk::DescriptorPoolSize uniformPoolSize;
		uniformPoolSize.setType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(capacityConfig.uniformCapacity);

		// Sampler pool size
		vk::DescriptorPoolSize samplerPoolSize;
		samplerPoolSize.setType(vk::DescriptorType::eSampledImage)
			.setDescriptorCount(capacityConfig.samplerCapacity);

		// Pool size list
		std::array poolSizes = {
			uniformPoolSize, samplerPoolSize
		};

		// Create pool
		vk::DescriptorPoolCreateInfo poolCreateInfo;
		poolCreateInfo.setPoolSizes(poolSizes)
			.setMaxSets(capacityConfig.maxSets);

		_descriptorPool = VulkanContext::GetDevice().createDescriptorPool(poolCreateInfo);
	}

	FrameDescriptorPool::~FrameDescriptorPool()
	{
		VulkanContext::GetDevice().destroyDescriptorPool(_descriptorPool);
	}

	vk::DescriptorSet FrameDescriptorPool::AllocateDescriptorSet(const vk::DescriptorSetLayout& layout)
	{
		vk::DescriptorSetAllocateInfo allocateInfo;
		allocateInfo.setDescriptorPool(_descriptorPool)
			.setDescriptorSetCount(1)
			.setSetLayouts(layout);

		return VulkanContext::GetDevice().allocateDescriptorSets(allocateInfo)[0];
	}
} // namespace Ailurus