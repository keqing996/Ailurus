#pragma once

#include "VulkanContext/VulkanPch.h"
#include <cstdint>
#include <unordered_set>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"
#include "VulkanContext/Helper/HashHelper.hpp"

namespace Ailurus
{
	class VulkanResource: public NonCopyable, public NonMovable
	{
		friend class VulkanResourceManager;
	public:
		virtual ~VulkanResource();

	public:
		void AddRef(const class VulkanCommandBuffer& pCommandBuffer);
		void RemoveRef(const class VulkanCommandBuffer& pCommandBuffer);
		size_t GetRefCount() const;
		void MarkDelete();
		bool IsMarkDeleted() const;

	protected:
		VulkanResource();

	private:
		bool _markDeleted = false;
		VkObjectSet<vk::CommandBuffer> _referencedCommandBuffer;
	};
} // namespace Ailurus