#include <vulkan/vulkan.hpp>
#include "VulkanObjectPool.hpp"
#include "Ailurus/Application/Application.h"
#include "VulkanSystem/VulkanSystem.h"

namespace Ailurus
{
	template <>
	vk::CommandBuffer VulkanObjectPool<vk::CommandBuffer>::Create()
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
	void VulkanObjectPool<vk::CommandBuffer>::Destroy(vk::CommandBuffer res)
	{
		auto* vkSystem = Application::Get<VulkanSystem>();
		vkSystem->GetDevice().freeCommandBuffers(vkSystem->GetCommandPool(), res);
	}

	template <>
	vk::Fence VulkanObjectPool<vk::Fence>::Create()
	{
		vk::FenceCreateInfo fenceInfo;
		return Application::Get<VulkanSystem>()->GetDevice().createFence(fenceInfo);
	}

	template <>
	void VulkanObjectPool<vk::Fence>::Destroy(vk::Fence res)
	{
		Application::Get<VulkanSystem>()->GetDevice().destroyFence(res);
	}

	template <>
	vk::Semaphore VulkanObjectPool<vk::Semaphore>::Create()
	{
		auto device = Application::Get<VulkanSystem>()->GetDevice();

		vk::SemaphoreCreateInfo semaphoreInfo;
		return device.createSemaphore(semaphoreInfo);
	}

	template <>
	void VulkanObjectPool<vk::Semaphore>::Destroy(vk::Semaphore res)
	{
		Application::Get<VulkanSystem>()->GetDevice().destroySemaphore(res);
	}
}