#include "VulkanDeviceBuffer.h"
#include "Ailurus/Utility/Logger.h"
#include "VulkanContext/VulkanContext.h"

namespace Ailurus
{
    static void DeviceBufferDeleter(VulkanResource* pResource)
	{
		auto ptr = static_cast<VulkanDeviceBuffer*>(pResource);
		const auto device = VulkanContext::GetDevice();

		try
		{
			// Destroy buffer first, then device memory.
			if (ptr->buffer != nullptr)
				device.destroyBuffer(ptr->buffer);

			if (ptr->deviceMemory != nullptr)
				device.freeMemory(ptr->deviceMemory);
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to delete device buffer resource: {}", e.what());
		}

		delete ptr;
	}

    VulkanResourcePtr VulkanDeviceBuffer::Create(vk::DeviceSize size, DeviceBufferUsage usage)
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
        return VulkanResourcePtr(pBufferRaw, &DeviceBufferDeleter);
    }
}