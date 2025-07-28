#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>
#include "VulkanBuffer.h"

namespace Ailurus
{
	class VulkanDescriptorAllocator;

	class VulkanResourceManager : public NonCopyable, public NonMovable
	{
		using ResourcePtr = std::unique_ptr<VulkanResource, std::function<void(VulkanResource*)>>;
	public:
		~VulkanResourceManager();

	public:
		VulkanDeviceBuffer* CreateDeviceBuffer(vk::DeviceSize size, DeviceBufferUsage usage);
		VulkanHostBuffer* CreateHostBuffer(vk::DeviceSize size, HostBufferUsage usage, bool coherentWithGpu = true);

		void GarbageCollect();

		// Object resources
		auto AllocateCommandBuffer() -> vk::CommandBuffer;
		auto AllocateSemaphore() -> vk::Semaphore;
		auto AllocateFence() -> vk::Fence;
		auto AllocateDescriptorAllocator() -> std::unique_ptr<VulkanDescriptorAllocator>;
		void FreeCommandBuffer(vk::CommandBuffer commandBuffer, bool destroyImmediately = false);
		void FreeSemaphore(vk::Semaphore semaphore, bool destroyImmediately = false);
		void FreeFence(vk::Fence fence, bool destroyImmediately = false);
		void FreeDescriptorAllocator(std::unique_ptr<VulkanDescriptorAllocator>&& pDescriptorPool, bool destroyImmediately = false);

	private:
		static auto CreateCommandBuffer() -> vk::CommandBuffer;
		static auto CreateFence() -> vk::Fence;
		static auto CreateSemaphore() -> vk::Semaphore;
		static void DestroyCommandBuffer(vk::CommandBuffer buffer);
		static void DestroyFence(vk::Fence fence);
		static void DestroySemaphore(vk::Semaphore semaphore);

	private:
		// Command buffer resources
		std::vector<ResourcePtr> _resources;

		// Object resources
		std::vector<vk::CommandBuffer> _queuedCommandBuffers{};
		std::vector<vk::Fence> _queuedFences{};
		std::vector<vk::Semaphore> _queuedSemaphores{};
		std::vector<std::unique_ptr<VulkanDescriptorAllocator>> _queuedDescriptorAllocators{};
	};
} // namespace Ailurus