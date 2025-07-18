#include "Ailurus/Application/Application.h"
#include "VulkanSystem/VulkanSystem.h"
#include "VulkanDescriptorSetLayout.h"
#include "VulkanDescriptorPool.h"

namespace Ailurus
{
	VulkanDescriptorPool::PoolCapacity VulkanDescriptorPool::_defaultPoolCapacity = {
		.setsNum = 200,
		.descriptorCount = {
			{ vk::DescriptorType::eUniformBuffer, 400 },
			{ vk::DescriptorType::eSampledImage, 400 }
		}
	};

	VulkanDescriptorPool::VulkanDescriptorPool()
	{
		_pools.push_back(CreatePoolItem());
	}

	VulkanDescriptorPool::~VulkanDescriptorPool()
	{
		for (auto& poolItem : _pools)
			Application::Get<VulkanSystem>()->GetDevice().destroyDescriptorPool(poolItem.pool);
	}

	void VulkanDescriptorPool::ResetPool()
	{
		for (auto& poolItem : _pools)
		{
			Application::Get<VulkanSystem>()->GetDevice().resetDescriptorPool(poolItem.pool);
			poolItem.currentCapacity = poolItem.originalCapacity;
		}
	}

	VulkanDescriptorSet VulkanDescriptorPool::AllocateDescriptorSet(const VulkanDescriptorSetLayout* pSetLayout)
	{
		for (auto& poolItem : _pools)
		{
			if (CanPoolItemAllocateNewSet(poolItem, pSetLayout))
				return VulkanDescriptorSet{ PoolItemAllocateNewSet(poolItem, pSetLayout) };
		}

		_pools.push_back(CreatePoolItem());
		return VulkanDescriptorSet{ PoolItemAllocateNewSet(_pools.back(), pSetLayout) };
	}

	VulkanDescriptorPool::PoolItem VulkanDescriptorPool::CreatePoolItem()
	{
		std::vector<vk::DescriptorPoolSize> poolSizes;
		for (const auto [type, size] : _defaultPoolCapacity.descriptorCount) 
			poolSizes.push_back(vk::DescriptorPoolSize(type, size));

		vk::DescriptorPoolCreateInfo poolCreateInfo;
		poolCreateInfo.setPoolSizes(poolSizes)
			.setMaxSets(_defaultPoolCapacity.setsNum);

		auto pool = Application::Get<VulkanSystem>()->GetDevice().createDescriptorPool(poolCreateInfo);

		return PoolItem{ pool, _defaultPoolCapacity, _defaultPoolCapacity };
	}

	auto VulkanDescriptorPool::CanPoolItemAllocateNewSet(const PoolItem& poolItem, const class VulkanDescriptorSetLayout* pSetLayout) -> bool
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

	vk::DescriptorSet VulkanDescriptorPool::PoolItemAllocateNewSet(PoolItem& poolItem, const class VulkanDescriptorSetLayout* pSetLayout) 
	{
		vk::DescriptorSetLayout layout = pSetLayout->GetDescriptorSetLayout();
		vk::DescriptorSetAllocateInfo allocateInfo;
		allocateInfo.setDescriptorPool(poolItem.pool)
			.setDescriptorSetCount(1)
			.setSetLayouts(layout);

		auto descriptorSets = Application::Get<VulkanSystem>()->GetDevice().allocateDescriptorSets(allocateInfo);
		poolItem.currentCapacity.setsNum--;

		for (const auto& [type, needCount] : pSetLayout->GetRequirement())
			poolItem.currentCapacity.descriptorCount[type] -= needCount;

		return descriptorSets[0];
	}
} // namespace Ailurus