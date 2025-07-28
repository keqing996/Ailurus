#include "VulkanContext/VulkanContext.h"
#include "VulkanDescriptorSetLayout.h"
#include "VulkanDescriptorAllocator.h"

namespace Ailurus
{
	VulkanDescriptorAllocator::PoolCapacity VulkanDescriptorAllocator::_defaultPoolCapacity = {
		.setsNum = 200,
		.descriptorCount = {
			{ vk::DescriptorType::eUniformBuffer, 400 },
			{ vk::DescriptorType::eSampledImage, 400 }
		}
	};

	VulkanDescriptorAllocator::VulkanDescriptorAllocator()
	{
		_pools.push_back(CreatePoolItem());
	}

	VulkanDescriptorAllocator::~VulkanDescriptorAllocator()
	{
		for (auto& poolItem : _pools)
			VulkanContext::GetDevice().destroyDescriptorPool(poolItem.pool);
	}

	void VulkanDescriptorAllocator::ResetPool()
	{
		for (auto& poolItem : _pools)
		{
			VulkanContext::GetDevice().resetDescriptorPool(poolItem.pool);
			poolItem.currentCapacity = poolItem.originalCapacity;
		}
	}

	VulkanDescriptorSet VulkanDescriptorAllocator::AllocateDescriptorSet(const VulkanDescriptorSetLayout* pSetLayout)
	{
		for (auto& poolItem : _pools)
		{
			if (CanPoolItemAllocateNewSet(poolItem, pSetLayout))
				return VulkanDescriptorSet{ PoolItemAllocateNewSet(poolItem, pSetLayout) };
		}

		_pools.push_back(CreatePoolItem());
		return VulkanDescriptorSet{ PoolItemAllocateNewSet(_pools.back(), pSetLayout) };
	}

	VulkanDescriptorAllocator::PoolItem VulkanDescriptorAllocator::CreatePoolItem()
	{
		std::vector<vk::DescriptorPoolSize> poolSizes;
		for (const auto [type, size] : _defaultPoolCapacity.descriptorCount) 
			poolSizes.push_back(vk::DescriptorPoolSize(type, size));

		vk::DescriptorPoolCreateInfo poolCreateInfo;
		poolCreateInfo.setPoolSizes(poolSizes)
			.setMaxSets(_defaultPoolCapacity.setsNum);

		auto pool = VulkanContext::GetDevice().createDescriptorPool(poolCreateInfo);

		return PoolItem{ pool, _defaultPoolCapacity, _defaultPoolCapacity };
	}

	auto VulkanDescriptorAllocator::CanPoolItemAllocateNewSet(const PoolItem& poolItem, const class VulkanDescriptorSetLayout* pSetLayout) -> bool
	{
		// Check sets enough
		if (poolItem.currentCapacity.setsNum <= 0)
			return false;

		// Check descriptor count
		auto& currentCapacity = poolItem.currentCapacity;
		for (const auto [type, needCount] : pSetLayout->GetRequirement())
		{
			auto it = currentCapacity.descriptorCount.find(type);
			if (it == poolItem.currentCapacity.descriptorCount.end() || it->second < needCount)
				return false;
		}

		return true;
	}

	vk::DescriptorSet VulkanDescriptorAllocator::PoolItemAllocateNewSet(PoolItem& poolItem, const class VulkanDescriptorSetLayout* pSetLayout) 
	{
		vk::DescriptorSetLayout layout = pSetLayout->GetDescriptorSetLayout();
		vk::DescriptorSetAllocateInfo allocateInfo;
		allocateInfo.setDescriptorPool(poolItem.pool)
			.setDescriptorSetCount(1)
			.setSetLayouts(layout);

		auto descriptorSets = VulkanContext::GetDevice().allocateDescriptorSets(allocateInfo);
		poolItem.currentCapacity.setsNum--;

		for (const auto& [type, needCount] : pSetLayout->GetRequirement())
			poolItem.currentCapacity.descriptorCount[type] -= needCount;

		return descriptorSets[0];
	}
} // namespace Ailurus