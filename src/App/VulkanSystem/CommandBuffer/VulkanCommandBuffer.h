#pragma once

#include <vulkan/vulkan.hpp>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"

namespace Ailurus
{
	class VulkanCommandBuffer
	{
	public:
		VulkanCommandBuffer();
		~VulkanCommandBuffer();

	public:
		const vk::CommandBuffer& GetBuffer() const;

	private:
		vk::CommandBuffer _vkCommandBuffer;
	};

	struct VulkanRecordCommandScope: public NonCopyable, public NonMovable
	{
		explicit VulkanRecordCommandScope(const VulkanCommandBuffer& commandBuffer);
		~VulkanRecordCommandScope();

		vk::CommandBuffer buffer;
	};
} // namespace Ailurus