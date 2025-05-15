#include <vulkan/vulkan.hpp>
#include "VulkanPool.hpp"
#include "Ailurus/Application/Application.h"
#include "VulkanSystem/VulkanSystem.h"

namespace Ailurus
{
	template <>
	vk::CommandBuffer VulkanPool<vk::CommandBuffer>::Create()
	{
		auto* vkSystem = Application::Get<VulkanSystem>();

		vk::CommandBufferAllocateInfo allocInfo;
		allocInfo.setCommandPool(vkSystem->GetCommandPool())
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(1);

		const std::vector<vk::CommandBuffer> tempBuffers = vkSystem->GetDevice().allocateCommandBuffers(allocInfo);
		return tempBuffers[0];
	}

	template <>
	void VulkanPool<vk::CommandBuffer>::Destroy(vk::CommandBuffer res)
	{
		auto* vkSystem = Application::Get<VulkanSystem>();
		vkSystem->GetDevice().freeCommandBuffers(vkSystem->GetCommandPool(), res);
	}

	template <>
	vk::Fence VulkanPool<vk::Fence>::Create()
	{
		vk::FenceCreateInfo fenceInfo;
		return Application::Get<VulkanSystem>()->GetDevice().createFence(fenceInfo);
	}

	template <>
	void VulkanPool<vk::Fence>::Destroy(vk::Fence res)
	{
		Application::Get<VulkanSystem>()->GetDevice().destroyFence(res);
	}

	template <>
	vk::Semaphore VulkanPool<vk::Semaphore>::Create()
	{
		auto device = Application::Get<VulkanSystem>()->GetDevice();

		vk::SemaphoreCreateInfo semaphoreInfo;
		return device.createSemaphore(semaphoreInfo);
	}

	template <>
	void VulkanPool<vk::Semaphore>::Destroy(vk::Semaphore res)
	{
		Application::Get<VulkanSystem>()->GetDevice().destroySemaphore(res);
	}
}