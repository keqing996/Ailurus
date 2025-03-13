#pragma once

#include <optional>
#include <vulkan/vulkan.hpp>
#include "BufferType.h"

namespace Ailurus
{
    struct BufferWithMem
    {
        vk::Buffer buffer;
        vk::DeviceMemory deviceMemory;
    };

    class DataBufferUtil
    {
    public:
        DataBufferUtil() = delete;

    public:
        static std::optional<BufferWithMem>
        CreateBuffer(BufferType type, const char* bufferData, size_t bufferSizeInBytes);

        static std::optional<BufferWithMem>
        CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);

        static void
        CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);
    };
}