#pragma once

#include <cstdint>
#include <unordered_set>
#include <vulkan/vulkan.hpp>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"

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
		void MarkDelete();
		bool IsValid() const;
		virtual uint32_t GetHash() = 0;

	protected:
		VulkanResource();

	private:
		bool _markDeleted = false;
		std::unordered_set<vk::CommandBuffer> _referencedCommandBuffer;
	};
} // namespace Ailurus