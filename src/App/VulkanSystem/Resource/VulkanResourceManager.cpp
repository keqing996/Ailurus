#include "VulkanResourceManager.h"
#include "Ailurus/Application/Application.h"
#include "Ailurus/Utility/ScopeGuard.h"
#include "Ailurus/Utility/Logger.h"
#include "VulkanSystem/Resource/VulkanBuffer.h"
#include "VulkanSystem/VulkanSystem.h"
#include <cstdint>
#include <memory>

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
		auto device = Application::Get<VulkanSystem>()->GetDevice();

		vk::MemoryRequirements memRequirements = device.getBufferMemoryRequirements(buffer);

		// Find memory type
		std::optional<uint32_t> memoryTypeIndex = std::nullopt;
		vk::PhysicalDeviceMemoryProperties memProperties = Application::Get<VulkanSystem>()->GetPhysicalDevice().getMemoryProperties();
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
		const auto device = Application::Get<VulkanSystem>()->GetDevice();

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
		const auto device = Application::Get<VulkanSystem>()->GetDevice();

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
		const auto device = Application::Get<VulkanSystem>()->GetDevice();

		// Destroy buffer first, then device memory.
		if (ptr->buffer != nullptr)
			device.destroyBuffer(ptr->buffer);

		if (ptr->deviceMemory != nullptr)
			device.freeMemory(ptr->deviceMemory);

		delete ptr;
	}

	VulkanDeviceBuffer* VulkanResourceManager::CreateDeviceBuffer(vk::DeviceSize size, GpuBufferUsage usage)
	{
		vk::BufferUsageFlags usageFlag;
		usageFlag |= vk::BufferUsageFlagBits::eTransferDst;
		switch (usage)
		{
			case GpuBufferUsage::Vertex:
				usageFlag |= vk::BufferUsageFlagBits::eVertexBuffer;
				break;
			case GpuBufferUsage::Index:
				usageFlag |= vk::BufferUsageFlagBits::eIndexBuffer;
				break;
			case GpuBufferUsage::Uniform:
				usageFlag |= vk::BufferUsageFlagBits::eUniformBuffer;
				break;
			default:
				Logger::LogError("Unknown gpu buffer usage type: {}", EnumReflection<GpuBufferUsage>::ToString(usage));
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

	VulkanHostBuffer* VulkanResourceManager::CreateCpuBuffer(vk::DeviceSize size, CpuBufferUsage usage, bool coherentWithGpu)
	{
		vk::BufferUsageFlags usageFlag;
		switch (usage)
		{
			case CpuBufferUsage::TransferSrc:
				usageFlag |= vk::BufferUsageFlagBits::eTransferSrc;
				break;
			default:
				Logger::LogError("Unknown cpu buffer usage type: {}", EnumReflection<CpuBufferUsage>::ToString(usage));
				return nullptr;
		}

		vk::MemoryPropertyFlags propertyFlag = vk::MemoryPropertyFlagBits::eHostVisible;
		if (coherentWithGpu)
			propertyFlag |= vk::MemoryPropertyFlagBits::eHostCoherent;

		const std::optional<CreatedBuffer> bufferRet = CreateBuffer(size, usageFlag, propertyFlag);
		if (!bufferRet.has_value())
			return nullptr;

		// Map memory
		void* mappedAddr = Application::Get<VulkanSystem>()->GetDevice().mapMemory(bufferRet->deviceMem, 0, size, {});

		VulkanHostBuffer* pBufferRaw = new VulkanHostBuffer(bufferRet->realSize, bufferRet->buffer, bufferRet->deviceMem, mappedAddr);
		_resources.push_back(ResourcePtr(pBufferRaw, &DeleteVulkanResource<VulkanHostBuffer>));

		return pBufferRaw;
	}

	void VulkanResourceManager::GarbageCollect()
	{
		static std::vector<uint64_t> needDeletedFrames;

		auto thisFrame = Application::Get<TimeSystem>()->FrameCount();
		for (auto i = 0; i < _resources.size(); i++)
		{
			auto& pResource = _resources[i];

			// Clear passed frames
			needDeletedFrames.clear();
			for (auto frame : pResource->_referencedFrames)
			{
				if (frame < thisFrame)
					needDeletedFrames.push_back(frame);
			}

			for (auto frame : needDeletedFrames)
				pResource->_referencedFrames.erase(frame);
		}
	}

} // namespace Ailurus