#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include <new>
#include "Ailurus/Memory/StackAllocator.hpp"
#include "Helper.h"

using namespace Ailurus;

template<typename T, size_t alignment, size_t blockSize, bool deleteReverse>
void AllocateAndDelete()
{
    StackAllocator<alignment> allocator(blockSize);

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
    if (deleteReverse)
    {
        for (int i = dataVec.size() - 1; i >= 0; i--)
        {
            MemoryAllocatorLinkedNode* pCurrentStackTop = allocator.GetStackTop();
            MemoryAllocatorLinkedNode* pPrevFrame = pCurrentStackTop->GetPrevNode();

            Alloc::Delete<T>(&allocator, dataVec[i]);

            CHECK(allocator.GetStackTop() == pPrevFrame);
        }
    }
    else
    {
        for (size_t i = 0; i < dataVec.size(); i++)
        {
            MemoryAllocatorLinkedNode* pCurrentStackTop = allocator.GetStackTop();
            bool isStackTop = MemoryAllocatorLinkedNode::BackStepToLinkNode<alignment>(dataVec[i]) == pCurrentStackTop;

            Alloc::Delete<T>(&allocator, dataVec[i]);

            if (!isStackTop)
                CHECK(pCurrentStackTop == allocator.GetStackTop());
            else
                CHECK(allocator.GetStackTop() == nullptr);
        }
    }


    // Check
    MemoryAllocatorLinkedNode* pFirstNode = allocator.GetStackTop();
    CHECK(pFirstNode == nullptr);
}

TEST_CASE("StackAllocator")
{
    AllocateAndDelete<uint32_t, 4, 128, true>();
    AllocateAndDelete<uint32_t, 4, 4096, true>();
    AllocateAndDelete<uint32_t, 8, 4096, true>();
    AllocateAndDelete<Data64B, 8, 4096, true>();
    AllocateAndDelete<Data128B, 8, 4096, true>();

    AllocateAndDelete<uint32_t, 4, 128, false>();
    AllocateAndDelete<uint32_t, 4, 4096, false>();
    AllocateAndDelete<uint32_t, 8, 4096, false>();
    AllocateAndDelete<Data64B, 8, 4096, false>();
    AllocateAndDelete<Data128B, 8, 4096, false>();
}