#pragma once

#include <cstddef>

namespace Ailurus
{
    class Memory
    {
    public:
        Memory() = delete;

    public:
        static void* VirtualReserve(void* wantedAddr, size_t size);
        static bool VirtualCommit(void* addr, size_t size);
        static void VirtualRelease(void* addr, size_t size);
        static size_t CurrentPageSize();
    };

}