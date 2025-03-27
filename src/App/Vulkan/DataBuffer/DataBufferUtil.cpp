#include "DataBufferUtil.h"
#include "Ailurus/Utility/ScopeGuard.h"
#include "Ailurus/Utility/Logger.h"
#include "Vulkan/Context/VulkanContext.h"

namespace Ailurus
{
    std::optional<BufferWithMem>
    DataBufferUtil::CreateBuffer(BufferType type, const void* bufferData, size_t bufferSizeInBytes)
    {
        auto stagingBufferRet = CreateBuffer(bufferSizeInBytes, vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
        if (!stagingBufferRet.has_value())
        {
            Logger::LogError("Staging buffer create failed.");
            return std::nullopt;
        }

        auto [stagingBuffer, stagingBufferMemory] = stagingBufferRet.value();
        ScopeGuard stagingBufferReleaseGuard = [&]() -> void
        {
            const auto device = VulkanContext::GetDevice();
            device.destroyBuffer(stagingBuffer);
            device.freeMemory(stagingBufferMemory);
        };

        const auto device = VulkanContext::GetDevice();
        void* mappedAddr = device.mapMemory(stagingBufferMemory, 0, bufferSizeInBytes, {});
        ::memcpy(mappedAddr, bufferData, bufferSizeInBytes);
        device.unmapMemory(stagingBufferMemory);

        vk::BufferUsageFlags targetBufferFlags = vk::BufferUsageFlagBits::eTransferDst;
        switch (type)
        {
            case BufferType::Vertex:
                targetBufferFlags |= vk::BufferUsageFlagBits::eVertexBuffer;
                break;
            case BufferType::Index:
                targetBufferFlags |= vk::BufferUsageFlagBits::eIndexBuffer;
                break;
            default:
                Logger::LogError("Unknown buffer type: {}", EnumReflection<BufferType>::ToString(type));
                return std::nullopt;
        }

        auto targetBufferRet = CreateBuffer(bufferSizeInBytes, targetBufferFlags,
            vk::MemoryPropertyFlagBits::eDeviceLocal);
        if (!targetBufferRet.has_value())
        {
            Logger::LogError("Vertex buffer create failed.");
            return std::nullopt;
        }

        CopyBuffer(stagingBuffer, targetBufferRet->buffer, bufferSizeInBytes);

        return *targetBufferRet;
    }

    std::optional<BufferWithMem>
    DataBufferUtil::CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
    {
        auto device = VulkanContext::GetDevice();

        vk::BufferCreateInfo bufferInfo;
        bufferInfo.setSize(size)
                .setUsage(usage)
                .setSharingMode(vk::SharingMode::eExclusive);

        vk::Buffer buffer = device.createBuffer(bufferInfo);

        vk::MemoryRequirements memRequirements = device.getBufferMemoryRequirements(buffer);

        // Find memory type
        std::optional<uint32_t> memoryTypeIndex = std::nullopt;
        vk::PhysicalDeviceMemoryProperties memProperties = VulkanContext::GetPhysicalDevice().getMemoryProperties();
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if ((memRequirements.memoryTypeBits & (1 << i))
                && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                memoryTypeIndex = i;
                break;
            }
        }

        if (!memoryTypeIndex.has_value())
        {
            device.destroyBuffer(buffer);
            return std::nullopt;
        }

        vk::MemoryAllocateInfo allocInfo;
        allocInfo.setAllocationSize(memRequirements.size)
                .setMemoryTypeIndex(*memoryTypeIndex);

        vk::DeviceMemory deviceMem = device.allocateMemory(allocInfo);
        device.bindBufferMemory(buffer,deviceMem, 0);

        return BufferWithMem{buffer, deviceMem};
    }

    void DataBufferUtil::CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
    {
        auto device = VulkanContext::GetDevice();

        vk::CommandBufferAllocateInfo allocInfo;
        allocInfo.setLevel(vk::CommandBufferLevel::ePrimary)
                .setCommandPool(VulkanContext::GetCommandPool())
                .setCommandBufferCount(1);

        std::vector<vk::CommandBuffer> tempCmdBuffer = device.allocateCommandBuffers(allocInfo);
        ScopeGuard bufferReleaseGuard = [&]() -> void
        {
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
}
