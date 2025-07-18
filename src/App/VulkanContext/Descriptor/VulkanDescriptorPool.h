#pragma once

#include <vector>
#include <unordered_map>
#include <vulkan/vulkan.hpp>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>
#include "VulkanDescriptorSet.h"

namespace Ailurus
{
	class VulkanDescriptorPool : public NonCopyable, public NonMovable
	{
		struct PoolCapacity
		{
			size_t setsNum;
			std::unordered_map<vk::DescriptorType, uint32_t> descriptorCount;
		};

		struct PoolItem
		{
			vk::DescriptorPool pool;
			PoolCapacity originalCapacity;
			PoolCapacity currentCapacity;
		};

	public:
		VulkanDescriptorPool();
		~VulkanDescriptorPool();

	public:
		auto ResetPool() -> void;
		auto AllocateDescriptorSet(const class VulkanDescriptorSetLayout* pSetLayout) -> VulkanDescriptorSet;

	private:
		auto CreatePoolItem() -> PoolItem;
		auto CanPoolItemAllocateNewSet(const PoolItem& poolItem, const class VulkanDescriptorSetLayout* pSetLayout) -> bool;
		auto PoolItemAllocateNewSet(PoolItem& poolItem, const class VulkanDescriptorSetLayout* pSetLayout) -> vk::DescriptorSet;

	private:
		std::vector<PoolItem> _pools;
		static PoolCapacity _defaultPoolCapacity;
	};

} // namespace Ailurus