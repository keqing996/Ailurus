#include "VulkanContext/VulkanContext.h"
#include "VulkanDescriptorSetLayout.h"
#include "VulkanDescriptorAllocator.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
	VulkanDescriptorAllocator::PoolCapacity VulkanDescriptorAllocator::_defaultPoolCapacity = []() {
		PoolCapacity capacity;
		capacity.setsNum = 200;
		capacity.SetCount(vk::DescriptorType::eUniformBuffer, 400);
		capacity.SetCount(vk::DescriptorType::eSampledImage, 400);
		capacity.SetCount(vk::DescriptorType::eCombinedImageSampler, 400);
		return capacity;
	}();

	VulkanDescriptorAllocator::VulkanDescriptorAllocator()
	{
		_pools.push_back(CreatePoolItem());
		_poolStats.peakPoolCount = 1;
	}

	VulkanDescriptorAllocator::~VulkanDescriptorAllocator()
	{
		try
		{
			for (auto& poolItem : _pools)
				VulkanContext::GetDevice().destroyDescriptorPool(poolItem.pool);
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to destroy descriptor pool: {}", e.what());
		}
	}

	void VulkanDescriptorAllocator::ResetPools()
	{
		try
		{
			// Update statistics before reset
			_poolStats.totalResets++;
			
			// Clear cache before resetting pools (descriptor sets will be invalid)
			_cache.clear();
			_cacheStats.cachedSets = 0;
			
			// Reset all pools and mark them as active
			for (auto& poolItem : _pools)
			{
				VulkanContext::GetDevice().resetDescriptorPool(poolItem.pool);
				poolItem.currentCapacity = poolItem.originalCapacity;
				poolItem.isActive = true;
			}

			// Reset per-frame statistics
			_poolStats.frameAllocations = 0;
			_poolStats.frameDescriptorUsage.clear();
			_currentPoolIndex = 0;
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to reset descriptor pool: {}", e.what());
		}
	}

	VulkanDescriptorSet VulkanDescriptorAllocator::AllocateDescriptorSet(const VulkanDescriptorSetLayout* pSetLayout, const CacheKey* cacheKey)
	{
		_cacheStats.totalRequests++;
		
		// Check cache if key is provided
		if (cacheKey != nullptr)
		{
			auto it = _cache.find(*cacheKey);
			if (it != _cache.end())
			{
				_cacheStats.cacheHits++;
				return it->second;
			}
			_cacheStats.cacheMisses++;
		}
		
		// Find or create an available pool
		PoolItem* pPool = GetOrCreateAvailablePool(pSetLayout);
		
		if (pPool == nullptr)
		{
			Logger::LogError("Failed to get available descriptor pool");
			return vk::DescriptorSet{};
		}

		// Allocate from the pool
		vk::DescriptorSet set = PoolItemAllocateNewSet(*pPool, pSetLayout);
		
		// Update statistics
		UpdatePoolStatistics(pSetLayout);
		
		// Cache the allocated set if key is provided
		if (cacheKey != nullptr)
		{
			_cache[*cacheKey] = set;
			_cacheStats.cachedSets = _cache.size();
		}

		return set;
	}
	
	size_t VulkanDescriptorAllocator::HashBuffers(const std::vector<vk::Buffer>& buffers)
	{
		size_t hash = 0;
		for (const auto& buffer : buffers)
		{
			size_t bufferHash = std::hash<VkBuffer>{}(static_cast<VkBuffer>(buffer));
			hash ^= bufferHash + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		}
		return hash;
	}

	void VulkanDescriptorAllocator::LogStatistics() const
	{
		Logger::LogInfo("=== Descriptor Pool Statistics ===");
		Logger::LogInfo("Total Allocations: {}", _poolStats.totalAllocations);
		Logger::LogInfo("Total Resets: {}", _poolStats.totalResets);
		Logger::LogInfo("Peak Pool Count: {}", _poolStats.peakPoolCount);
		Logger::LogInfo("Peak Sets Usage: {}", _poolStats.peakSetsUsage);
		Logger::LogInfo("Active Pools: {}", _pools.size());
		
		if (!_poolStats.peakDescriptorUsage.empty())
		{
			Logger::LogInfo("Peak Descriptor Usage:");
			for (const auto& [type, count] : _poolStats.peakDescriptorUsage)
			{
				Logger::LogInfo("  Type {}: {}", static_cast<int>(type), count);
			}
		}
		
		Logger::LogInfo("=== Descriptor Cache Statistics ===");
		Logger::LogInfo("Total Requests: {}", _cacheStats.totalRequests);
		Logger::LogInfo("Cache Hits: {}", _cacheStats.cacheHits);
		Logger::LogInfo("Cache Misses: {}", _cacheStats.cacheMisses);
		Logger::LogInfo("Hit Rate: {:.2f}%", _cacheStats.GetHitRate() * 100.0f);
		Logger::LogInfo("Cached Sets: {}", _cacheStats.cachedSets);
	}

	VulkanDescriptorAllocator::PoolItem VulkanDescriptorAllocator::CreatePoolItem()
	{
		std::vector<vk::DescriptorPoolSize> poolSizes;
		
		// Iterate through all descriptor types in the capacity array
		for (size_t i = 0; i < DESCRIPTOR_TYPE_COUNT; ++i)
		{
			uint32_t count = _defaultPoolCapacity.descriptorCount[i];
			if (count > 0)
			{
				vk::DescriptorType type = static_cast<vk::DescriptorType>(i);
				poolSizes.push_back(vk::DescriptorPoolSize(type, count));
			}
		}

		vk::DescriptorPoolCreateInfo poolCreateInfo;
		poolCreateInfo.setPoolSizes(poolSizes)
			.setMaxSets(_defaultPoolCapacity.setsNum)
			.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet); // Allow freeing individual sets

		auto pool = VulkanContext::GetDevice().createDescriptorPool(poolCreateInfo);

		return PoolItem{ pool, _defaultPoolCapacity, _defaultPoolCapacity, true };
	}

	auto VulkanDescriptorAllocator::GetOrCreateAvailablePool(const VulkanDescriptorSetLayout* pSetLayout) -> PoolItem*
	{
		// Try to find an existing pool with enough capacity
		for (size_t i = _currentPoolIndex; i < _pools.size(); ++i)
		{
			if (_pools[i].isActive && CanPoolItemAllocateNewSet(_pools[i], pSetLayout))
			{
				_currentPoolIndex = i;
				return &_pools[i];
			}
		}

		// Try from the beginning if we haven't checked all pools
		for (size_t i = 0; i < _currentPoolIndex; ++i)
		{
			if (_pools[i].isActive && CanPoolItemAllocateNewSet(_pools[i], pSetLayout))
			{
				_currentPoolIndex = i;
				return &_pools[i];
			}
		}

		// No available pool found, create a new one
		_pools.push_back(CreatePoolItem());
		_currentPoolIndex = _pools.size() - 1;
		
		// Update peak pool count
		if (_pools.size() > _poolStats.peakPoolCount)
			_poolStats.peakPoolCount = _pools.size();

		return &_pools.back();
	}

	auto VulkanDescriptorAllocator::CanPoolItemAllocateNewSet(const PoolItem& poolItem, const class VulkanDescriptorSetLayout* pSetLayout) -> bool
	{
		// Check if pool is active
		if (!poolItem.isActive)
			return false;

		// Check sets enough
		if (poolItem.currentCapacity.setsNum <= 0)
			return false;

		// Check descriptor count using the new helper methods
		const auto& currentCapacity = poolItem.currentCapacity;
		for (const auto& [type, needCount] : pSetLayout->GetRequirement())
		{
			uint32_t available = currentCapacity.GetCount(type);
			if (available < needCount)
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

		// Update capacity counts using helper method
		for (const auto& [type, needCount] : pSetLayout->GetRequirement())
		{
			size_t index = static_cast<size_t>(type);
			if (index < DESCRIPTOR_TYPE_COUNT)
				poolItem.currentCapacity.descriptorCount[index] -= needCount;
		}

		return descriptorSets[0];
	}

	void VulkanDescriptorAllocator::UpdatePoolStatistics(const VulkanDescriptorSetLayout* pSetLayout)
	{
		// Update total and frame allocations
		_poolStats.totalAllocations++;
		_poolStats.frameAllocations++;

		// Update frame descriptor usage
		for (const auto& [type, count] : pSetLayout->GetRequirement())
		{
			_poolStats.frameDescriptorUsage[type] += count;
		}

		// Calculate current total usage across all pools
		size_t currentSetsUsage = 0;
		std::unordered_map<vk::DescriptorType, uint32_t> currentDescriptorUsage;
		
		for (const auto& pool : _pools)
		{
			currentSetsUsage += (pool.originalCapacity.setsNum - pool.currentCapacity.setsNum);
			
			// Iterate through descriptor type array
			for (size_t i = 0; i < DESCRIPTOR_TYPE_COUNT; ++i)
			{
				uint32_t originalCount = pool.originalCapacity.descriptorCount[i];
				uint32_t currentCount = pool.currentCapacity.descriptorCount[i];
				
				if (originalCount > 0)
				{
					uint32_t used = originalCount - currentCount;
					if (used > 0)
					{
						vk::DescriptorType type = static_cast<vk::DescriptorType>(i);
						currentDescriptorUsage[type] += used;
					}
				}
			}
		}

		// Update peak usage
		if (currentSetsUsage > _poolStats.peakSetsUsage)
			_poolStats.peakSetsUsage = currentSetsUsage;

		for (const auto& [type, usage] : currentDescriptorUsage)
		{
			if (usage > _poolStats.peakDescriptorUsage[type])
				_poolStats.peakDescriptorUsage[type] = usage;
		}
	}
} // namespace Ailurus