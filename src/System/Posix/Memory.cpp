#include "Ailurus/PlatformDefine.h"

#if PLATFORM_SUPPORT_POSIX

#include <sys/mman.h>
#include <unistd.h>
#include "Ailurus/System/Memory.h"

#if PLATFORM_IOS || PLATFORM_MAC
#include <mach/vm_page_size.h>
#endif

namespace Ailurus
{
    void* Memory::VirtualReserve(void* wantedAddr, size_t size)
    {
        void* result = ::mmap(wantedAddr, size, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
        return result == MAP_FAILED ? nullptr : result;
    }

    bool Memory::VirtualCommit(void* addr, size_t size)
    {
        return !::mprotect(addr, size, PROT_READ | PROT_WRITE);
    }

    void Memory::VirtualRelease(void* addr, size_t size)
    {
        ::munmap(addr, size);
    }

    size_t Memory::CurrentPageSize()
    {
#if PLATFORM_IOS || PLATFORM_MAC
        return vm_page_size;
#else
        return ::sysconf(_SC_PAGE_SIZE);
#endif
    }
}

#endif