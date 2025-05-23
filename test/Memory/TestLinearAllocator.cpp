#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include <new>
#include "Ailurus/Memory/LinearAllocator.hpp"
#include "Helper.h"

using namespace Ailurus;

template<typename T, size_t alignment, size_t blockSize>
void AllocateAndDelete(size_t* alreadyAllocateSize, LinearAllocator<alignment>* pAllocator)
{
    size_t availableSize = pAllocator->GetAvailableSpaceSize();

    T* ptr = Alloc::New<T>(pAllocator);

    size_t leftAvailableSize = pAllocator->GetAvailableSpaceSize();

    size_t allocationSize = MemoryAllocatorUtil::UpAlignment<sizeof(T), alignment>();
    void* pMemBlock = pAllocator->GetMemoryBlockPtr();
    void* pCurrent = pAllocator->GetCurrentPtr();

    if (*alreadyAllocateSize + allocationSize > blockSize)
    {
        std::cout << std::format("Allocation failed, pMem = 0x{:x}, pCur = 0x{:x}, need = {}, available = {}, left = {}"
            , ToAddr(pMemBlock), ToAddr(pCurrent), allocationSize, availableSize, leftAvailableSize) << std::endl;
        CHECK(ptr == nullptr);
    }
    else
    {
        *alreadyAllocateSize += allocationSize;

        std::cout << std::format("Allocation success, pMem = 0x{:x}, pCur = 0x{:x}, need = {}, available = {}, left = {}"
            , ToAddr(pMemBlock), ToAddr(pCurrent), allocationSize, availableSize, leftAvailableSize) << std::endl;

        CHECK(ToAddr(pCurrent) == ToAddr(pMemBlock) + *alreadyAllocateSize);

        Alloc::Delete(pAllocator, ptr);

        CHECK(ToAddr(pCurrent) == ToAddr(pMemBlock) + *alreadyAllocateSize);
    }
}

template <size_t alignment, size_t blockSize>
void TestAllocation()
{
    std::cout << "======== Test Allocation ========" << std::endl;
    std::cout << std::format("Alignment = {}, Block Size = {}", alignment, blockSize) << std::endl;

    LinearAllocator<alignment> allocator(blockSize);

    void* pMemBlock = allocator.GetMemoryBlockPtr();
    void* pCurrent = allocator.GetCurrentPtr();
    size_t alreadyAllocateSize = 0;

    std::cout << std::format("Allocator block start addr: 0x{:x}", ToAddr(pMemBlock)) << std::endl;
    CHECK(pMemBlock != nullptr);
    CHECK(pMemBlock == pCurrent);

    AllocateAndDelete<uint32_t, alignment, blockSize>(&alreadyAllocateSize, &allocator);
    AllocateAndDelete<uint32_t, alignment, blockSize>(&alreadyAllocateSize, &allocator);
    AllocateAndDelete<uint64_t, alignment, blockSize>(&alreadyAllocateSize, &allocator);
    AllocateAndDelete<Data64B, alignment, blockSize>(&alreadyAllocateSize, &allocator);
    AllocateAndDelete<Data64B, alignment, blockSize>(&alreadyAllocateSize, &allocator);
    AllocateAndDelete<Data128B, alignment, blockSize>(&alreadyAllocateSize, &allocator);
    AllocateAndDelete<uint32_t, alignment, blockSize>(&alreadyAllocateSize, &allocator);
    AllocateAndDelete<uint32_t, alignment, blockSize>(&alreadyAllocateSize, &allocator);
    AllocateAndDelete<uint32_t, alignment, blockSize>(&alreadyAllocateSize, &allocator);
    AllocateAndDelete<uint32_t, alignment, blockSize>(&alreadyAllocateSize, &allocator);
    AllocateAndDelete<Data32B, alignment, blockSize>(&alreadyAllocateSize, &allocator);
}

TEST_CASE("LinearAllocator")
{
    TestAllocation<4, 128>();
    TestAllocation<4, 256>();
    TestAllocation<4, 512>();
    TestAllocation<8, 128>();
    TestAllocation<8, 256>();
    TestAllocation<8, 512>();
}