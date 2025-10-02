#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>
#include <array>
#include <unordered_map>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>
#include "VulkanDescriptorSet.h"

namespace Ailurus
{
	class VulkanDescriptorAllocator : public NonCopyable, public NonMovable
	{
	public:
		/// @brief Cache key for descriptor set lookup
		struct CacheKey
		{
			vk::DescriptorSetLayout layout;
			size_t bindingHash = 0;

			bool operator==(const CacheKey& other) const
			{
				return layout == other.layout && bindingHash == other.bindingHash;
			}
		};

		/// @brief Hash function for CacheKey
		struct CacheKeyHash
		{
			size_t operator()(const CacheKey& key) const
			{
				size_t h1 = std::hash<VkDescriptorSetLayout>{}(static_cast<VkDescriptorSetLayout>(key.layout));
				size_t h2 = key.bindingHash;
				return h1 ^ (h2 << 1);
			}
		};

		/// @brief Cache statistics
		struct CacheStatistics
		{
			size_t totalRequests = 0;
			size_t cacheHits = 0;
			size_t cacheMisses = 0;
			size_t cachedSets = 0;
			
			float GetHitRate() const
			{
				return totalRequests > 0 ? static_cast<float>(cacheHits) / totalRequests : 0.0f;
			}
		};

	private:
		static constexpr size_t DESCRIPTOR_TYPE_COUNT = 11;
		
		struct PoolCapacity
		{
			size_t setsNum;
			std::array<uint32_t, DESCRIPTOR_TYPE_COUNT> descriptorCount{};
			
			void SetCount(vk::DescriptorType type, uint32_t count)
			{
				size_t index = static_cast<size_t>(type);
				if (index < DESCRIPTOR_TYPE_COUNT)
					descriptorCount[index] = count;
			}

			uint32_t GetCount(vk::DescriptorType type) const
			{
				size_t index = static_cast<size_t>(type);
				return (index < DESCRIPTOR_TYPE_COUNT) ? descriptorCount[index] : 0;
			}

			bool HasType(vk::DescriptorType type) const
			{
				return GetCount(type) > 0;
			}
		};

		struct PoolItem
		{
			vk::DescriptorPool pool;
			PoolCapacity originalCapacity;
			PoolCapacity currentCapacity;
			bool isActive = true; // Track if pool is currently in use
		};

		struct PoolStatistics
		{
			size_t totalAllocations = 0;         // Total descriptor sets allocated
			size_t totalResets = 0;              // Number of times pools have been reset
			size_t peakPoolCount = 0;            // Maximum number of pools created
			size_t peakSetsUsage = 0;            // Peak number of sets allocated at once
			std::unordered_map<vk::DescriptorType, uint32_t> peakDescriptorUsage; // Peak usage per type
			
			// Per-frame statistics (reset each frame)
			size_t frameAllocations = 0;
			std::unordered_map<vk::DescriptorType, uint32_t> frameDescriptorUsage;
		};

	public:
		VulkanDescriptorAllocator();
		~VulkanDescriptorAllocator();

	public:
		/// @brief Reset all pools and cache for new frame
		auto ResetPools() -> void;
		
		/// @brief Allocate or retrieve cached descriptor set
		/// @param pSetLayout Layout for the descriptor set
		/// @param cacheKey Optional cache key for reuse (if not provided, always allocates new)
		auto AllocateDescriptorSet(const class VulkanDescriptorSetLayout* pSetLayout, const CacheKey* cacheKey = nullptr) -> VulkanDescriptorSet;
		
		/// @brief Create a hash for buffer bindings (for cache key)
		static auto HashBuffers(const std::vector<vk::Buffer>& buffers) -> size_t;
		
		// Statistics access
		auto GetPoolStatistics() const -> const PoolStatistics& { return _poolStats; }
		auto GetCacheStatistics() const -> const CacheStatistics& { return _cacheStats; }
		auto LogStatistics() const -> void;

	private:
		auto CreatePoolItem() -> PoolItem;
		auto GetOrCreateAvailablePool(const class VulkanDescriptorSetLayout* pSetLayout) -> PoolItem*;
		auto CanPoolItemAllocateNewSet(const PoolItem& poolItem, const class VulkanDescriptorSetLayout* pSetLayout) -> bool;
		auto PoolItemAllocateNewSet(PoolItem& poolItem, const class VulkanDescriptorSetLayout* pSetLayout) -> vk::DescriptorSet;
		auto UpdatePoolStatistics(const class VulkanDescriptorSetLayout* pSetLayout) -> void;

	private:
		// Pool management
		std::vector<PoolItem> _pools;
		size_t _currentPoolIndex = 0;
		static PoolCapacity _defaultPoolCapacity;
		PoolStatistics _poolStats;
		
		// Cache management
		std::unordered_map<CacheKey, VulkanDescriptorSet, CacheKeyHash> _cache;
		CacheStatistics _cacheStats;
	};

} // namespace Ailurus