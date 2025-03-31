#include "Ailurus/PlatformDefine.h"

#if AILURUS_PLATFORM_WINDOWS

#include "Ailurus/Platform/Windows/WindowsDefine.h"
#include "Ailurus/System/Memory.h"

namespace Ailurus
{
    void* Memory::VirtualReserve(void* wantedAddr, size_t size)
    {
        void* result = ::VirtualAlloc(wantedAddr, size, MEM_RESERVE, PAGE_READWRITE);
        if (result != nullptr)
            return result;

        return ::VirtualAlloc(nullptr, size, MEM_RESERVE, PAGE_READWRITE);
    }

    bool Memory::VirtualCommit(void* addr, size_t size)
    {
        return addr == ::VirtualAlloc(addr, size, MEM_COMMIT, PAGE_READWRITE);
    }

    void Memory::VirtualRelease(void* addr, size_t size)
    {
        (void)size; // size used in POSIX
        ::VirtualFree(addr, 0, MEM_RELEASE);
    }

    size_t Memory::CurrentPageSize()
    {
        ::SYSTEM_INFO sysInfo;
        ::GetSystemInfo(&sysInfo);
        return sysInfo.dwPageSize;
    }
}

#endif