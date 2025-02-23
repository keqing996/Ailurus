#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <new>
#include "Ailurus/Memory/FreeListAllocator.hpp"
#include "Helper.h"

using namespace Ailurus;

template<typename T, size_t alignment, size_t blockSize>
void AllocateAndDelete()
{
    FreeListAllocator<alignment> allocator(blockSize);

    size_t allocationSize = MemoryAllocatorUtil::UpAlignment<sizeof(T), alignment>();
    size_t headerSize = MemoryAllocatorLinkedNode::PaddedSize<alignment>();
    size_t cellSize = allocationSize + headerSize;

    size_t numberToAllocate = blockSize / cellSize;

    // Allocate
    std::vector<T*> dataVec;
    MemoryAllocatorLinkedNode* pLastNode = nullptr;
    for (size_t i = 0; i < numberToAllocate; i++)
    {
        auto ptr = Alloc::New<T>(&allocator);
        CHECK(ptr != nullptr);

        MemoryAllocatorLinkedNode* pCurrentNode = MemoryAllocatorLinkedNode::BackStepToLinkNode<alignment>(ptr);

        std::cout << std::format("Allocate, addr = {:x}, node addr = {:x}, prev node = {:x}, node size = {}",
            ToAddr(ptr), ToAddr(pCurrentNode), ToAddr(pCurrentNode->GetPrevNode()), pCurrentNode->GetSize()) << std::endl;

        CHECK(pCurrentNode->GetPrevNode() == pLastNode);
        CHECK(pCurrentNode->Used() == true);
        if (i != numberToAllocate - 1)
            CHECK(pCurrentNode->GetSize() == allocationSize);

        dataVec.push_back(ptr);

        pLastNode = pCurrentNode;
    }

    // Can not allocate anymore
    T* pData = Alloc::New<T>(&allocator);
    CHECK(pData == nullptr);

    // Deallocate
    for (size_t i = 0; i < dataVec.size(); i++)
        Alloc::Delete(&allocator, dataVec[i]);

    // Check
    MemoryAllocatorLinkedNode* pFirstNode = allocator.GetFirstNode();
    CHECK(pFirstNode->Used() == false);
    CHECK(pFirstNode->GetPrevNode() == nullptr);
    CHECK(pFirstNode->GetSize() == blockSize - headerSize);
}

TEST_CASE("FreeListAllocator")
{
    AllocateAndDelete<uint32_t, 4, 128>();
    AllocateAndDelete<uint32_t, 4, 4096>();
    AllocateAndDelete<uint32_t, 8, 4096>();
    AllocateAndDelete<Data64B, 8, 4096>();
    AllocateAndDelete<Data128B, 8, 4096>();
}