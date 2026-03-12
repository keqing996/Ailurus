#include "VulkanHostBuffer.h"
#include "VulkanContext/VulkanContext.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
    static void HostBufferDeleter(VulkanResource* pResource)
	{
		auto ptr = static_cast<VulkanHostBuffer*>(pResource);
		const auto device = VulkanContext::GetDevice();

		try
		{
			if (ptr->mappedAddr != nullptr)
				device.unmapMemory(ptr->deviceMemory);

			// Destroy buffer first, then device memory.
			if (ptr->buffer != nullptr)
				device.destroyBuffer(ptr->buffer);

			if (ptr->deviceMemory != nullptr)
				device.freeMemory(ptr->deviceMemory);
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to delete host buffer resource: {}", e.what());
		}

		delete ptr;
	}

    VulkanResourcePtr VulkanHostBuffer::Create(vk::DeviceSize size, HostBufferUsage usage, bool coherentWithGpu)
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
		try
		{
			void* mappedAddr = VulkanContext::GetDevice().mapMemory(bufferRet->deviceMem, 0, size, {});
			VulkanHostBuffer* pBufferRaw = new VulkanHostBuffer(bufferRet->realSize, bufferRet->buffer, bufferRet->deviceMem, mappedAddr);
            return VulkanResourcePtr(pBufferRaw, &HostBufferDeleter);
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to map memory: {}", e.what());
			return nullptr;
		}
    }
}