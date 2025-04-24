#pragma once

#include <vulkan/vulkan.hpp>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"

namespace Ailurus
{
	class VulkanCommandBuffer : public NonCopyable, public NonMovable
	{
	public:
		VulkanCommandBuffer();
		~VulkanCommandBuffer();

	public:
		vk::CommandBuffer GetBuffer() const;

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