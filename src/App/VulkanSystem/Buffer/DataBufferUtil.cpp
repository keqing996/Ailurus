#include "DataBufferUtil.h"
#include "Ailurus/Application/Application.h"
#include "Ailurus/Utility/ScopeGuard.h"
#include "Ailurus/Utility/Logger.h"
#include "VulkanSystem/VulkanSystem.h"

namespace Ailurus
{
	

	void DataBufferUtil::DestroyBuffer(CpuBuffer& cpuBuffer)
	{
		const auto device = Application::Get<VulkanSystem>()->GetDevice();

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
		const auto device = Application::Get<VulkanSystem>()->GetDevice();

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
