#pragma once

#include <cstdint>
#include "Util/Util.hpp"
#include "Util/LinkNodeHeader.hpp"

namespace Ailurus
{
    template <size_t DefaultAlignment = 4>
    class FreeListAllocator
    {
    public:
        explicit FreeListAllocator(size_t size);
        ~FreeListAllocator();

        FreeListAllocator(const FreeListAllocator& rhs) = delete;
        FreeListAllocator(FreeListAllocator&& rhs) = delete;

    public:
        void* Allocate(size_t size);
        void* Allocate(size_t size, size_t alignment);
        void Deallocate(void* p);
        void* GetMemoryBlockPtr() const;
        MemoryAllocatorLinkedNode* GetFirstNode() const;

    private:
        bool IsValidHeader(const MemoryAllocatorLinkedNode* pHeader) const;

    private:
        void* _pData;
        size_t _size;
        MemoryAllocatorLinkedNode* _pFirstNode;
    };

    template<size_t DefaultAlignment>
    FreeListAllocator<DefaultAlignment>::FreeListAllocator(size_t size)
        : _pData(nullptr)
        , _size(size)
        , _pFirstNode(nullptr)
    {
        if (_size < MemoryAllocatorLinkedNode::PaddedSize<DefaultAlignment>())
            _size = MemoryAllocatorLinkedNode::PaddedSize<DefaultAlignment>();

        _pData = ::malloc(_size);

        _pFirstNode = static_cast<MemoryAllocatorLinkedNode*>(_pData);
        _pFirstNode->SetUsed(false);
        _pFirstNode->SetSize(_size - MemoryAllocatorLinkedNode::PaddedSize<DefaultAlignment>());
        _pFirstNode->SetPrevNode(nullptr);
    }

    template<size_t DefaultAlignment>
    FreeListAllocator<DefaultAlignment>::~FreeListAllocator()
    {
        ::free(_pData);
        _pData = nullptr;
    }

    template<size_t DefaultAlignment>
    void* FreeListAllocator<DefaultAlignment>::Allocate(size_t size)
    {
        return Allocate(size, DefaultAlignment);
    }

    template<size_t DefaultAlignment>
    void* FreeListAllocator<DefaultAlignment>::Allocate(size_t size, size_t alignment)
    {
        size_t headerSize = MemoryAllocatorLinkedNode::PaddedSize<DefaultAlignment>();
        size_t requiredSize = MemoryAllocatorUtil::UpAlignment(size, alignment);

        MemoryAllocatorLinkedNode* pCurrentNode = _pFirstNode;
        while (true)
        {
            if (pCurrentNode == nullptr)
                return nullptr;

            if (!pCurrentNode->Used() && pCurrentNode->GetSize() >= requiredSize)
            {
                pCurrentNode->SetUsed(true);

                void* pResult = MemoryAllocatorUtil::PtrOffsetBytes(pCurrentNode, headerSize);

                // Create a new node if left size is enough to place a new header.
                size_t leftSize = pCurrentNode->GetSize() - requiredSize;
                if (leftSize > headerSize)
                {
                    pCurrentNode->SetSize(requiredSize);

                    MemoryAllocatorLinkedNode* pNextNode = pCurrentNode->MoveNext<DefaultAlignment>();
                    pNextNode->SetPrevNode(pCurrentNode);
                    pNextNode->SetUsed(false);
                    pNextNode->SetSize(leftSize - headerSize);
                }

                return pResult;
            }

            // Move next
            pCurrentNode = pCurrentNode->MoveNext<DefaultAlignment>();
            if (!IsValidHeader(pCurrentNode))
                pCurrentNode = nullptr;
        }
    }

    template<size_t DefaultAlignment>
    void FreeListAllocator<DefaultAlignment>::Deallocate(void* p)
    {
        MemoryAllocatorLinkedNode* pCurrentNode = MemoryAllocatorLinkedNode::BackStepToLinkNode<DefaultAlignment>(p);
        pCurrentNode->SetUsed(false);

        // Merge forward
        while (true)
        {
            MemoryAllocatorLinkedNode* pNextNode = pCurrentNode->MoveNext<DefaultAlignment>();
            if (!IsValidHeader(pNextNode) || pNextNode->Used())
                break;

            size_t oldSize = pCurrentNode->GetSize();
            size_t newSize = oldSize + MemoryAllocatorLinkedNode::PaddedSize<DefaultAlignment>() + pNextNode->GetSize();
            pNextNode->ClearData();
            pCurrentNode->SetSize(newSize);
        }

        // Merge backward
        while (true)
        {
            MemoryAllocatorLinkedNode* pPrevNode = pCurrentNode->GetPrevNode();
            if (!IsValidHeader(pPrevNode) || pPrevNode->Used())
                break;

            // Adjust prev node's size
            size_t oldSize = pPrevNode->GetSize();
            size_t newSize = oldSize + MemoryAllocatorLinkedNode::PaddedSize<DefaultAlignment>() + pCurrentNode->GetSize();
            pPrevNode->SetSize(newSize);

            // Adjust next node's prev
            MemoryAllocatorLinkedNode* pNextNode = pCurrentNode->MoveNext<DefaultAlignment>();
            if (IsValidHeader(pNextNode))
                pNextNode->SetPrevNode(pPrevNode);

            // Clear this node
            pCurrentNode->ClearData();

            // Move backward
            pCurrentNode = pPrevNode;
        }
    }

    template<size_t DefaultAlignment>
    void* FreeListAllocator<DefaultAlignment>::GetMemoryBlockPtr() const
    {
        return _pData;
    }

    template<size_t DefaultAlignment>
    MemoryAllocatorLinkedNode* FreeListAllocator<DefaultAlignment>::GetFirstNode() const
    {
        return _pFirstNode;
    }

    template<size_t DefaultAlignment>
    bool FreeListAllocator<DefaultAlignment>::IsValidHeader(const MemoryAllocatorLinkedNode* pHeader) const
    {
        size_t dataBeginAddr = MemoryAllocatorUtil::ToAddr(_pData);
        size_t dataEndAddr = dataBeginAddr + _size;
        size_t headerStartAddr = MemoryAllocatorUtil::ToAddr(pHeader);
        size_t headerEndAddr = headerStartAddr + MemoryAllocatorLinkedNode::PaddedSize<DefaultAlignment>();
        return headerStartAddr >= dataBeginAddr && headerEndAddr < dataEndAddr;
    }
}
