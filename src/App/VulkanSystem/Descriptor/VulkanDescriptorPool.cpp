#include <array>
#include "VulkanSystem/VulkanSystem.h"
#include "VulkanDescriptorPool.h"
#include "Ailurus/Application/Application.h"

namespace Ailurus
{
	bool VulkanDescriptorPool::DescriptorCapacity::IsEnoughFor(const DescriptorCapacity& requirement) const
	{
		return uniformNum >= requirement.uniformNum && samplerNum >= requirement.samplerNum;
	}

	void VulkanDescriptorPool::DescriptorCapacity::Allocate(const DescriptorCapacity& requirement)
	{
		uniformNum -= requirement.uniformNum;
		samplerNum -= requirement.samplerNum;
	}

	bool VulkanDescriptorPool::PoolCapacity::IsEnoughFor(const DescriptorCapacity& requirement) const
	{
		return setsNum > 0 && descriptorCapacity.IsEnoughFor(requirement);
	}

	void VulkanDescriptorPool::PoolCapacity::Allocate(const DescriptorCapacity& requirement)
	{
		descriptorCapacity.Allocate(requirement);
		setsNum--;
	}

	VulkanDescriptorPool::Entry::Entry(PoolCapacity poolConfig)
		: originalCapacity(poolConfig)
		, currentCapacity(poolConfig)
	{
		// Uniform pool size
		vk::DescriptorPoolSize uniformPoolSize;
		uniformPoolSize.setType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(poolConfig.descriptorCapacity.uniformNum);

		// Sampler pool size
		vk::DescriptorPoolSize samplerPoolSize;
		samplerPoolSize.setType(vk::DescriptorType::eSampledImage)
			.setDescriptorCount(poolConfig.descriptorCapacity.samplerNum);

		// Pool size list
		std::array poolSizes = {
			uniformPoolSize, samplerPoolSize
		};

		// Create pool
		vk::DescriptorPoolCreateInfo poolCreateInfo;
		poolCreateInfo.setPoolSizes(poolSizes)
			.setMaxSets(poolConfig.setsNum);

		pool = Application::Get<VulkanSystem>()->GetDevice().createDescriptorPool(poolCreateInfo);
	}

	VulkanDescriptorPool::Entry::~Entry()
	{
		Application::Get<VulkanSystem>()->GetDevice().destroyDescriptorPool(pool);
	}

	bool VulkanDescriptorPool::Entry::IsEnoughFor(const DescriptorCapacity& requirement) const
	{
		return currentCapacity.IsEnoughFor(requirement);
	}

	vk::DescriptorSet VulkanDescriptorPool::Entry::Allocate(const vk::DescriptorSetLayout& layout)
	{

	}

	VulkanDescriptorPool::VulkanDescriptorPool(const DescriptorPoolCapacityConfig& capacityConfig)
	{

	}

	VulkanDescriptorPool::~VulkanDescriptorPool()
	{
		Application::Get<VulkanSystem>()->GetDevice().destroyDescriptorPool(_descriptorPool);
	}

	vk::DescriptorSet VulkanDescriptorPool::AllocateDescriptorSet()
	{
		vk::DescriptorSetAllocateInfo allocateInfo;
		allocateInfo.setDescriptorPool(_descriptorPool)
			.setDescriptorSetCount(1)
			.setSetLayouts(layout);

		return Application::Get<VulkanSystem>()->GetDevice().allocateDescriptorSets(allocateInfo)[0];
	}
} // namespace Ailurus