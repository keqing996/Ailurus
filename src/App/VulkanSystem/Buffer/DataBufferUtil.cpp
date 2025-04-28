#include "DataBufferUtil.h"
#include "Ailurus/Application/Application.h"
#include "Ailurus/Utility/ScopeGuard.h"
#include "Ailurus/Utility/Logger.h"
#include "VulkanSystem/VulkanSystem.h"

namespace Ailurus
{
	

	

	void DataBufferUtil::CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
	{
		auto device = Application::Get<VulkanSystem>()->GetDevice();

		vk::CommandBufferAllocateInfo allocInfo;
		allocInfo.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandPool(Application::Get<VulkanSystem>()->GetCommandPool())
			.setCommandBufferCount(1);

		std::vector<vk::CommandBuffer> tempCmdBuffer = device.allocateCommandBuffers(allocInfo);
		ScopeGuard bufferReleaseGuard = [&]() -> void {
			Application::Get<VulkanSystem>()->GetDevice().freeCommandBuffers(Application::Get<VulkanSystem>()->GetCommandPool(), tempCmdBuffer);
		};

		vk::CommandBufferBeginInfo beginInfo;
		beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

		vk::CommandBuffer commandBuffer = tempCmdBuffer[0];
		commandBuffer.begin(beginInfo);
		{
			vk::BufferCopy copyRegion;
			copyRegion.setSize(size)
				.setSrcOffset(0)
				.setDstOffset(0);
			commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);
		}
		commandBuffer.end();

		vk::SubmitInfo submitInfo;
		submitInfo.setCommandBuffers(tempCmdBuffer);

		auto renderQueue = Application::Get<VulkanSystem>()->GetGraphicQueue();
		renderQueue.submit(submitInfo);
		renderQueue.waitIdle();
	}
} // namespace Ailurus
