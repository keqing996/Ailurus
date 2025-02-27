#pragma once

#include <optional>
#include <vulkan/vulkan.hpp>
#include "BufferType.h"

namespace Ailurus
{
    class Renderer;

    class Buffer
    {
    protected:
        struct BufferWithMem
        {
            vk::Buffer buffer;
            vk::DeviceMemory deviceMemory;
        };

    public:
        Buffer(const Renderer* pRenderer);
        virtual ~Buffer() = default;

    protected:
        std::optional<BufferWithMem>
        CreateBuffer(BufferType type, const char* bufferData, size_t bufferSize) const;

        std::optional<BufferWithMem>
        CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) const;

        void
        CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size) const;

    protected:
        const Renderer* _pRenderer;
    };
}