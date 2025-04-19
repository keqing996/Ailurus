#include "DataBufferUtil.h"
#include "Ailurus/Utility/ScopeGuard.h"
#include "Ailurus/Utility/Logger.h"
#include "Vulkan/Context/VulkanContext.h"

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
		auto device = VulkanContext::GetDevice();

		vk::MemoryRequirements memRequirements = device.getBufferMemoryRequirements(buffer);

		// Find memory type
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

	std::optional<CpuBuffer>
	DataBufferUtil::CreateCpuBuffer(vk::DeviceSize size, CpuBufferUsage usage, bool coherentWithGpu)
	{
		vk::BufferUsageFlags usageFlag;
		switch (usage)
		{
			case CpuBufferUsage::TransferSrc:
				usageFlag |= vk::BufferUsageFlagBits::eTransferSrc;
				break;
			default:
				Logger::LogError("Unknown cpu buffer usage type: {}", EnumReflection<CpuBufferUsage>::ToString(usage));
				return std::nullopt;
		}

		vk::MemoryPropertyFlags propertyFlag = vk::MemoryPropertyFlagBits::eHostVisible;
		if (coherentWithGpu)
			propertyFlag |= vk::MemoryPropertyFlagBits::eHostCoherent;

		const std::optional<CreatedBuffer> bufferRet = CreateBuffer(size, usageFlag, propertyFlag);
		if (!bufferRet.has_value())
			return std::nullopt;

		// Map memory
		void* mappedAddr = VulkanContext::GetDevice().mapMemory(bufferRet->deviceMem, 0, size, {});

		return CpuBuffer{ bufferRet->realSize, bufferRet->buffer,
			bufferRet->deviceMem, mappedAddr };
	}

	std::optional<GpuBuffer>
	DataBufferUtil::CreateGpuBuffer(vk::DeviceSize size, GpuBufferUsage usage)
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
				return std::nullopt;
		}

		constexpr vk::MemoryPropertyFlags propertyFlag = vk::MemoryPropertyFlagBits::eDeviceLocal;
		const std::optional<CreatedBuffer> bufferRet = CreateBuffer(size, usageFlag, propertyFlag);
		if (!bufferRet.has_value())
			return std::nullopt;

		return GpuBuffer{ bufferRet->realSize, bufferRet->buffer, bufferRet->deviceMem };
	}

	void DataBufferUtil::DestroyBuffer(CpuBuffer& cpuBuffer)
	{
		const auto device = VulkanContext::GetDevice();

		if (cpuBuffer.mappedAddr != nullptr)
		{
			device.unmapMemory(cpuBuffer.deviceMemory);
			cpuBuffer.mappedAddr = nullptr;
		}

		// Destroy buffer first, then device memory.
		if (cpuBuffer.buffer != nullptr)
		{
			device.destroyBuffer(cpuBuffer.buffer);
			cpuBuffer.buffer = nullptr;
		}

		if (cpuBuffer.deviceMemory != nullptr)
		{
			device.freeMemory(cpuBuffer.deviceMemory);
			cpuBuffer.deviceMemory = nullptr;
		}
	}

	void DataBufferUtil::DestroyBuffer(GpuBuffer& gpuBuffer)
	{
		const auto device = VulkanContext::GetDevice();

		// Destroy buffer first, then device memory.
		if (gpuBuffer.buffer != nullptr)
		{
			device.destroyBuffer(gpuBuffer.buffer);
			gpuBuffer.buffer = nullptr;
		}

		if (gpuBuffer.deviceMemory != nullptr)
		{
			device.freeMemory(gpuBuffer.deviceMemory);
			gpuBuffer.deviceMemory = nullptr;
		}
	}

	void DataBufferUtil::CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
	{
		auto device = VulkanContext::GetDevice();

		vk::CommandBufferAllocateInfo allocInfo;
		allocInfo.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandPool(VulkanContext::GetCommandPool())
			.setCommandBufferCount(1);

		std::vector<vk::CommandBuffer> tempCmdBuffer = device.allocateCommandBuffers(allocInfo);
		ScopeGuard bufferReleaseGuard = [&]() -> void {
			VulkanContext::GetDevice().freeCommandBuffers(VulkanContext::GetCommandPool(), tempCmdBuffer);
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

		auto renderQueue = VulkanContext::GetGraphicQueue();
		renderQueue.submit(submitInfo);
		renderQueue.waitIdle();
	}
} // namespace Ailurus
