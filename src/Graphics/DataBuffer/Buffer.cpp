#include "Ailurus/Graphics/DataBuffer/Buffer.h"
#include "Ailurus/Graphics/Renderer.h"
#include "Ailurus/Utility/ScopeGuard.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
    Buffer::Buffer(const Renderer* pRenderer)
        : _pRenderer(pRenderer)
    {
    }

    std::optional<Buffer::BufferWithMem> Buffer::CreateBuffer(BufferType type, const char* bufferData, size_t bufferSize) const
    {
        auto stagingBufferRet = CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
        if (!stagingBufferRet.has_value())
        {
            Logger::LogError("Staging buffer create failed.");
            return std::nullopt;
        }

        auto [stagingBuffer, stagingBufferMemory] = stagingBufferRet.value();
        ScopeGuard stagingBufferReleaseGuard = [&]() -> void
        {
            const auto device = _pRenderer->GetLogicalDevice();
            device.destroyBuffer(stagingBuffer);
            device.freeMemory(stagingBufferMemory);
        };

        const auto device = _pRenderer->GetLogicalDevice();
        void* mappedAddr = device.mapMemory(stagingBufferMemory, 0, bufferSize, {});
        ::memcpy(mappedAddr, bufferData, bufferSize);
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

        auto targetBufferRet = CreateBuffer(bufferSize, targetBufferFlags,
            vk::MemoryPropertyFlagBits::eDeviceLocal);
        if (!targetBufferRet.has_value())
        {
            Logger::LogError("Vertex buffer create failed.");
            return std::nullopt;
        }

        CopyBuffer(stagingBuffer, targetBufferRet->buffer, bufferSize);

        return *targetBufferRet;
    }

    std::optional<Buffer::BufferWithMem> Buffer::CreateBuffer(vk::DeviceSize size,
                                                              vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) const
    {
        auto device = _pRenderer->GetLogicalDevice();

        vk::BufferCreateInfo bufferInfo;
        bufferInfo.setSize(size)
                .setUsage(usage)
                .setSharingMode(vk::SharingMode::eExclusive);

        vk::Buffer buffer = device.createBuffer(bufferInfo);

        vk::MemoryRequirements memRequirements = device.getBufferMemoryRequirements(buffer);

        // Find memory type
        std::optional<uint32_t> memoryTypeIndex = std::nullopt;
        vk::PhysicalDeviceMemoryProperties memProperties = _pRenderer->GetPhysicalDevice().getMemoryProperties();
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

    void Buffer::CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size) const
    {
        auto device = _pRenderer->GetLogicalDevice();

        vk::CommandBufferAllocateInfo allocInfo;
        allocInfo.setLevel(vk::CommandBufferLevel::ePrimary)
                .setCommandPool(_pRenderer->GetCommandPool())
                .setCommandBufferCount(1);

        std::vector<vk::CommandBuffer> tempCmdBuffer = device.allocateCommandBuffers(allocInfo);
        ScopeGuard bufferReleaseGuard = [&]() -> void
        {
            _pRenderer->GetLogicalDevice().freeCommandBuffers(
                _pRenderer->GetCommandPool(), tempCmdBuffer);
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

        auto renderQueue = _pRenderer->GetGraphicQueue();
        renderQueue.submit(submitInfo);
        renderQueue.waitIdle();
    }
}
