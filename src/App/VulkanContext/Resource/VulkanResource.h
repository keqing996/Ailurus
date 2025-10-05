#pragma once

#include "VulkanContext/VulkanPch.h"
#include <cstdint>
#include <unordered_set>
#include <memory>
#include <functional>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"
#include "VulkanContext/Helper/HashHelper.hpp"

namespace Ailurus
{
	class VulkanResource: public NonCopyable, public NonMovable
	{
	public:
		VulkanResource();
		virtual ~VulkanResource();

	public:
		auto AddRef(const class VulkanCommandBuffer& pCommandBuffer) -> void;
		auto RemoveRef(const class VulkanCommandBuffer& pCommandBuffer) -> void;
		auto GetRefCount() const -> size_t;
		auto MarkDelete() -> void;
		auto IsMarkDeleted() const -> bool;
		auto GetDebugName() const -> const std::string&;
		auto SetDebugName(const std::string& name) -> void;

	private:
		bool _markDeleted = false;
		VkObjectSet<vk::CommandBuffer> _referencedCommandBuffer;
		std::string _debugName;
	};

	using VulkanResourcePtr = std::unique_ptr<VulkanResource, std::function<void(VulkanResource*)>>;
} // namespace Ailurus