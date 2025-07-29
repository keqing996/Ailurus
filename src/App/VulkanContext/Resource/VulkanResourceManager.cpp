#include "VulkanResourceManager.h"
#include "Ailurus/Utility/Logger.h"
#include "VulkanContext/Resource/VulkanBuffer.h"
#include "VulkanContext/Resource/VulkanResource.h"
#include "VulkanContext/VulkanContext.h"
#include "VulkanContext/Descriptor/VulkanDescriptorAllocator.h"

namespace Ailurus
{
	struct BufferMemoryRequirement
	{
		vk::MemoryRequirements requirements;
		uint32_t memTypeIndex;
	};

	struct CreatedBuffer
	{
		vk::DeviceSize realSize;
		vk::Buffer buffer;
		vk::DeviceMemory deviceMem;
	};

	static std::optional<BufferMemoryRequirement>
	GetBufferMemoryRequirement(vk::Buffer buffer, vk::MemoryPropertyFlags propertyFlag)
	{
		vk::MemoryRequirements memRequirements = VulkanContext::GetDevice().getBufferMemoryRequirements(buffer);

		// Find a memory type
		std::optional<uint32_t> memoryTypeIndex = std::nullopt;
		vk::PhysicalDeviceMemoryProperties memProperties = VulkanContext::GetPhysicalDevice().getMemoryProperties();
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((memRequirements.memoryTypeBits & (1 << i))
				&& (memProperties.memoryTypes[i].propertyFlags & propertyFlag) == propertyFlag)
			{
				memoryTypeIndex = i;
				break;
			}
		}

		if (memoryTypeIndex.has_value())
			return BufferMemoryRequirement{ memRequirements, *memoryTypeIndex };

		return std::nullopt;
	}

	static std::optional<CreatedBuffer>
	CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usageFlag, vk::MemoryPropertyFlags propertyFlag)
	{
		const auto device = VulkanContext::GetDevice();

		// Create buffer
		vk::BufferCreateInfo bufferInfo;
		bufferInfo.setSize(size)
			.setUsage(usageFlag)
			.setSharingMode(vk::SharingMode::eExclusive);

		const vk::Buffer buffer = device.createBuffer(bufferInfo);

		// Get gpu memory requirement
		const std::optional<BufferMemoryRequirement> memoryRequirement = GetBufferMemoryRequirement(buffer, propertyFlag);
		if (!memoryRequirement.has_value())
		{
			device.destroyBuffer(buffer);
			Logger::LogError("Failed to get buffer memory requirement when creating buffer.");
			return std::nullopt;
		}

		// Alloc memory
		vk::MemoryAllocateInfo allocInfo;
		allocInfo.setAllocationSize(memoryRequirement->requirements.size)
			.setMemoryTypeIndex(memoryRequirement->memTypeIndex);

		const vk::DeviceMemory deviceMem = device.allocateMemory(allocInfo);
		device.bindBufferMemory(buffer, deviceMem, 0);

		return CreatedBuffer{ memoryRequirement->requirements.size, buffer, deviceMem };
	}

	template <typename T>
	void DeleteVulkanResource(VulkanResource* pResource);

	template <>
	void DeleteVulkanResource<VulkanHostBuffer>(VulkanResource* pResource)
	{
		auto ptr = static_cast<VulkanHostBuffer*>(pResource);
		const auto device = VulkanContext::GetDevice();

		if (ptr->mappedAddr != nullptr)
			device.unmapMemory(ptr->deviceMemory);

		// Destroy buffer first, then device memory.
		if (ptr->buffer != nullptr)
			device.destroyBuffer(ptr->buffer);

		if (ptr->deviceMemory != nullptr)
			device.freeMemory(ptr->deviceMemory);

		delete ptr;
	}

	template <>
	void DeleteVulkanResource<VulkanDeviceBuffer>(VulkanResource* pResource)
	{
		auto ptr = static_cast<VulkanDeviceBuffer*>(pResource);
		const auto device = VulkanContext::GetDevice();

		// Destroy buffer first, then device memory.
		if (ptr->buffer != nullptr)
			device.destroyBuffer(ptr->buffer);

		if (ptr->deviceMemory != nullptr)
			device.freeMemory(ptr->deviceMemory);

		delete ptr;
	}

	VulkanResourceManager::~VulkanResourceManager()
	{
		// Clear pool objects
		while (_queuedCommandBuffers.size() > 0)
		{
			DestroyCommandBuffer(_queuedCommandBuffers.back());
			_queuedCommandBuffers.pop_back();
		}

		while (_queuedSemaphores.size() > 0)
		{
			DestroySemaphore(_queuedSemaphores.back());
			_queuedSemaphores.pop_back();
		}

		while (_queuedFences.size() > 0)
		{
			DestroyFence(_queuedFences.back());
			_queuedFences.pop_back();
		}

		_queuedDescriptorAllocators.clear();
	}

	VulkanDeviceBuffer* VulkanResourceManager::CreateDeviceBuffer(vk::DeviceSize size, DeviceBufferUsage usage)
	{
		vk::BufferUsageFlags usageFlag;
		usageFlag |= vk::BufferUsageFlagBits::eTransferDst;
		switch (usage)
		{
			case DeviceBufferUsage::Vertex:
				usageFlag |= vk::BufferUsageFlagBits::eVertexBuffer;
				break;
			case DeviceBufferUsage::Index:
				usageFlag |= vk::BufferUsageFlagBits::eIndexBuffer;
				break;
			case DeviceBufferUsage::Uniform:
				usageFlag |= vk::BufferUsageFlagBits::eUniformBuffer;
				break;
			default:
				Logger::LogError("Unknown gpu buffer usage type: {}", EnumReflection<DeviceBufferUsage>::ToString(usage));
				return nullptr;
		}

		constexpr vk::MemoryPropertyFlags propertyFlag = vk::MemoryPropertyFlagBits::eDeviceLocal;
		const std::optional<CreatedBuffer> bufferRet = CreateBuffer(size, usageFlag, propertyFlag);
		if (!bufferRet.has_value())
			return nullptr;

		VulkanDeviceBuffer* pBufferRaw = new VulkanDeviceBuffer(bufferRet->realSize, bufferRet->buffer, bufferRet->deviceMem);
		_resources.push_back(ResourcePtr(pBufferRaw, &DeleteVulkanResource<VulkanDeviceBuffer>));

		return pBufferRaw;
	}

	VulkanHostBuffer* VulkanResourceManager::CreateHostBuffer(vk::DeviceSize size, HostBufferUsage usage, bool coherentWithGpu)
	{
		vk::BufferUsageFlags usageFlag;
		switch (usage)
		{
			case HostBufferUsage::TransferSrc:
				usageFlag |= vk::BufferUsageFlagBits::eTransferSrc;
				break;
			default:
				Logger::LogError("Unknown cpu buffer usage type: {}", EnumReflection<HostBufferUsage>::ToString(usage));
				return nullptr;
		}

		vk::MemoryPropertyFlags propertyFlag = vk::MemoryPropertyFlagBits::eHostVisible;
		if (coherentWithGpu)
			propertyFlag |= vk::MemoryPropertyFlagBits::eHostCoherent;

		const std::optional<CreatedBuffer> bufferRet = CreateBuffer(size, usageFlag, propertyFlag);
		if (!bufferRet.has_value())
			return nullptr;

		// Map memory
		void* mappedAddr = VulkanContext::GetDevice().mapMemory(bufferRet->deviceMem, 0, size, {});

		VulkanHostBuffer* pBufferRaw = new VulkanHostBuffer(bufferRet->realSize, bufferRet->buffer, bufferRet->deviceMem, mappedAddr);
		_resources.push_back(ResourcePtr(pBufferRaw, &DeleteVulkanResource<VulkanHostBuffer>));

		return pBufferRaw;
	}

	void VulkanResourceManager::GarbageCollect()
	{
		// Mark
		static std::vector<uint64_t> needDeletedResourceIndex;
		needDeletedResourceIndex.clear();
		for (auto i = 0; i < _resources.size(); i++)
		{
			if (_resources[i]->IsMarkDeleted() && _resources[i]->GetRefCount() == 0)
				needDeletedResourceIndex.push_back(i);
		}

		// Release
		for (auto index : needDeletedResourceIndex)
			_resources[index] = nullptr;

		// Clean up
		static std::vector<ResourcePtr> resourcesBuffer;
		resourcesBuffer.clear();
		for (auto i = 0; i < _resources.size(); i++)
		{
			if (_resources[i] != nullptr)
				resourcesBuffer.push_back(std::move(_resources[i]));
		}

		std::swap(_resources, resourcesBuffer);
	}

	vk::CommandBuffer VulkanResourceManager::AllocateCommandBuffer()
	{
		if (_queuedCommandBuffers.empty())
		{
			return CreateCommandBuffer();
		}
		else
		{
			vk::CommandBuffer commandBuffer = _queuedCommandBuffers.back();
			_queuedCommandBuffers.pop_back();
			return commandBuffer;
		}
	}

	void VulkanResourceManager::FreeCommandBuffer(vk::CommandBuffer commandBuffer, bool destroyImmediately)
	{
		if (destroyImmediately)
		{
			DestroyCommandBuffer(commandBuffer);
		}
		else
		{
			_queuedCommandBuffers.push_back(commandBuffer);
		}
	}

	vk::Semaphore VulkanResourceManager::AllocateSemaphore()
	{
		if (_queuedSemaphores.empty())
		{
			return CreateSemaphore();
		}
		else
		{
			vk::Semaphore semaphore = _queuedSemaphores.back();
			_queuedSemaphores.pop_back();
			return semaphore;
		}
	}

	void VulkanResourceManager::FreeSemaphore(vk::Semaphore semaphore, bool destroyImmediately)
	{
		if (destroyImmediately)
		{
			DestroySemaphore(semaphore);
		}
		else
		{
			_queuedSemaphores.push_back(semaphore);
		}
	}

	vk::Fence VulkanResourceManager::AllocateFence()
	{
		if (_queuedFences.empty())
		{
			return CreateFence();
		}
		else
		{
			vk::Fence fence = _queuedFences.back();
			_queuedFences.pop_back();
			return fence;
		}
	}

	void VulkanResourceManager::FreeFence(vk::Fence fence, bool destroyImmediately)
	{
		if (destroyImmediately)
		{
			DestroyFence(fence);
		}
		else
		{
			_queuedFences.push_back(fence);
		}
	}

	std::unique_ptr<VulkanDescriptorAllocator> VulkanResourceManager::AllocateDescriptorAllocator()
	{
		if (_queuedDescriptorAllocators.empty())
		{
			return std::make_unique<VulkanDescriptorAllocator>();
		}
		else
		{
			auto pDescriptorPool = std::move(_queuedDescriptorAllocators.back());
			_queuedDescriptorAllocators.pop_back();
			return pDescriptorPool;
		}
	}

	void VulkanResourceManager::FreeDescriptorAllocator(std::unique_ptr<VulkanDescriptorAllocator>&& pDescriptorPool, bool destroyImmediately)
	{
		if (destroyImmediately)
		{
			pDescriptorPool.reset();
		}
		else
		{
			_queuedDescriptorAllocators.push_back(std::move(pDescriptorPool));
		}
	}

	vk::CommandBuffer VulkanResourceManager::CreateCommandBuffer()
	{
		vk::CommandBufferAllocateInfo allocInfo;
		allocInfo.setCommandPool(VulkanContext::GetCommandPool())
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandBufferCount(1);

		const std::vector<vk::CommandBuffer> tempBuffers = VulkanContext::GetDevice().allocateCommandBuffers(allocInfo);
		return tempBuffers[0];
	}

	vk::Fence VulkanResourceManager::CreateFence()
	{
		vk::FenceCreateInfo fenceInfo;
		return VulkanContext::GetDevice().createFence(fenceInfo);
	}

	vk::Semaphore VulkanResourceManager::CreateSemaphore()
	{
		vk::SemaphoreCreateInfo semaphoreInfo;
		return VulkanContext::GetDevice().createSemaphore(semaphoreInfo);
	}

	void VulkanResourceManager::DestroyCommandBuffer(vk::CommandBuffer buffer)
	{
		VulkanContext::GetDevice().freeCommandBuffers(VulkanContext::GetCommandPool(), buffer);
	}

	void VulkanResourceManager::DestroyFence(vk::Fence fence)
	{
		VulkanContext::GetDevice().destroyFence(fence);
	}

	void VulkanResourceManager::DestroySemaphore(vk::Semaphore semaphore)
	{
		VulkanContext::GetDevice().destroySemaphore(semaphore);
	}

} // namespace Ailurus