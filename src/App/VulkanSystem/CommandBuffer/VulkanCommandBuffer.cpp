#include "VulkanCommandBuffer.h"
#include "Ailurus/Application/Application.h"
#include "VulkanSystem/VulkanSystem.h"

namespace Ailurus
{
	VulkanCommandBuffer::VulkanCommandBuffer()
	{
		auto* vkSystem = Application::Get<VulkanSystem>();

		vk::CommandBufferAllocateInfo allocInfo;
		allocInfo.setCommandPool(vkSystem->GetCommandPool())
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(1);

		std::vector<vk::CommandBuffer> tempBuffers = vkSystem->GetDevice().allocateCommandBuffers(allocInfo);
		_vkCommandBuffer = tempBuffers[0];
	}

	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		auto* vkSystem = Application::Get<VulkanSystem>();
		vkSystem->GetDevice().freeCommandBuffers(vkSystem->GetCommandPool(), _vkCommandBuffer);
	}

	vk::CommandBuffer VulkanCommandBuffer::GetBuffer() const
	{
		return _vkCommandBuffer;
	}

	VulkanRecordCommandScope::VulkanRecordCommandScope(const VulkanCommandBuffer& commandBuffer)
		: buffer(commandBuffer.GetBuffer())
	{
		vk::CommandBufferBeginInfo beginInfo;
		beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		buffer.begin(beginInfo);
	}

	VulkanRecordCommandScope::~VulkanRecordCommandScope()
	{
		buffer.end();
	}
} // namespace Ailurus