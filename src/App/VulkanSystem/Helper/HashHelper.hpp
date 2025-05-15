#pragma once

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_hash.hpp>
#include <unordered_map>
#include <unordered_set>

namespace Ailurus
{
	template <typename T>
	struct VulkanObjectHash
	{
		std::size_t operator()(const T& object) const
		{
			return std::hash<T>()(object);
		}
	};

	template <typename T>
	struct VulkanObjectEqual
	{
		bool operator()(const T& lhs, const T& rhs) const
		{
			return static_cast<T>(lhs) == static_cast<T>(rhs);
		}
	};

	template <typename T>
	using VkObjectSet = std::unordered_set<T, VulkanObjectHash<T>, VulkanObjectEqual<T>>;

	template <typename T, typename F>
	using VkObjectMap = std::unordered_map<T, F, VulkanObjectHash<T>, VulkanObjectEqual<T>>;
} // namespace Ailurus