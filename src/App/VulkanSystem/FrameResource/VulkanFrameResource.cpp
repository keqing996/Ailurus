#include "VulkanFrameResource.h"
#include "Ailurus/Application/Application.h"
#include "VulkanSystem/VulkanSystem.h"

namespace Ailurus
{

	FrameResource::FrameResource()
	{
		const auto& vkDevice = Application::Get<VulkanSystem>()->GetDevice();

		// Create command pool
		vk::CommandPoolCreateInfo poolInfo;
		poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
			.setQueueFamilyIndex(Application::Get<VulkanSystem>()->GetGraphicQueueIndex());

		_commandPool = vkDevice.createCommandPool(poolInfo);

		// Create semaphore
		vk::SemaphoreCreateInfo semaphoreInfo;
		_imageAvailableSemaphore = vkDevice.createSemaphore(semaphoreInfo);
		_renderFinishedSemaphore = vkDevice.createSemaphore(semaphoreInfo);

		// Create fence
		vk::FenceCreateInfo fenceInfo;
		fenceInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
		_inFlightFence = vkDevice.createFence(fenceInfo);

		// Create command buffer
		vk::CommandBufferAllocateInfo allocInfo;
		allocInfo.setCommandPool(_commandPool)
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(1);

		auto createdCmdBuffers = vkDevice.allocateCommandBuffers(allocInfo);
		_commandBuffer = createdCmdBuffers[0];
	}

	FrameResource::~FrameResource()
	{
		const auto& vkDevice = Application::Get<VulkanSystem>()->GetDevice();

		// Destroy command buffer
		vkDevice.freeCommandBuffers(_commandPool, _commandBuffer);

		// Destroy fence
		vkDevice.destroyFence(_inFlightFence);

		// Destroy synchronization objects
		vkDevice.destroySemaphore(_renderFinishedSemaphore);
		vkDevice.destroySemaphore(_imageAvailableSemaphore);

		// Destroy command pool
		vkDevice.destroyCommandPool(_commandPool);
	}

	vk::Semaphore FrameResource::GetSemaphoreImageAvailable() const
	{
		return _imageAvailableSemaphore;
	}

	vk::Semaphore FrameResource::GetSemaphoreRenderFinished() const
	{
		return _renderFinishedSemaphore;
	}

	vk::Fence FrameResource::GetInFlightFence() const
	{
		return _inFlightFence;
	}

	vk::CommandBuffer FrameResource::GetCommandBuffer() const
	{
		return _commandBuffer;
	}
} // namespace Ailurus