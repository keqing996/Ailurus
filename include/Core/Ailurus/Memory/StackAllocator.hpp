#pragma once

#include <cstdint>
#include "Util/Util.hpp"
#include "Util/LinkNodeHeader.hpp"

namespace Ailurus
{
    template <size_t DefaultAlignment = 4>
    class StackAllocator
    {
    public:
        explicit StackAllocator(size_t size);
        ~StackAllocator();

        StackAllocator(const StackAllocator& rhs) = delete;
        StackAllocator(StackAllocator&& rhs) = delete;

    public:
        void* Allocate(size_t size);
        void* Allocate(size_t size, size_t alignment);
        void Deallocate(void* p);
        MemoryAllocatorLinkedNode* GetStackTop() const;

    private:
        bool NewFrame(size_t requiredSize);

    private:
        void* _pData;
        size_t _size;
        MemoryAllocatorLinkedNode* _pStackTop;
    };

    template<size_t DefaultAlignment>
    StackAllocator<DefaultAlignment>::StackAllocator(size_t size)
        : _pData(nullptr)
        , _size(size)
        , _pStackTop(nullptr)
    {
        if (_size < MemoryAllocatorLinkedNode::PaddedSize<DefaultAlignment>())
            _size = MemoryAllocatorLinkedNode::PaddedSize<DefaultAlignment>();

        _pData = static_cast<uint8_t*>(::malloc(_size));
        _pStackTop = nullptr;
    }

    template<size_t DefaultAlignment>
    StackAllocator<DefaultAlignment>::~StackAllocator()
    {
        ::free(_pData);
        _pData = nullptr;
    }

    template<size_t DefaultAlignment>
    void* StackAllocator<DefaultAlignment>::Allocate(size_t size)
    {
        return Allocate(size, DefaultAlignment);
    }

    template<size_t DefaultAlignment>
    void* StackAllocator<DefaultAlignment>::Allocate(size_t size, size_t alignment)
    {
        size_t headerSize = MemoryAllocatorLinkedNode::PaddedSize<DefaultAlignment>();
        size_t requiredSize = MemoryAllocatorUtil::UpAlignment(size, alignment);

        if (!NewFrame(requiredSize))
            return nullptr;

        _pStackTop->SetUsed(true);
        return MemoryAllocatorUtil::PtrOffsetBytes(_pStackTop, headerSize);
    }

    template<size_t DefaultAlignment>
    bool StackAllocator<DefaultAlignment>::NewFrame(size_t requiredSize)
    {
        size_t headerSize = MemoryAllocatorLinkedNode::PaddedSize<DefaultAlignment>();
        size_t totalOccupySize = headerSize + requiredSize;

        void* pNextLinkNode = _pStackTop == nullptr
            ? _pData
            : _pStackTop->MoveNext<DefaultAlignment>();

        size_t availableSize = MemoryAllocatorUtil::ToAddr(_pData) + _size - MemoryAllocatorUtil::ToAddr(pNextLinkNode);
        if (availableSize < totalOccupySize)
            return false;

        MemoryAllocatorLinkedNode* pResult = static_cast<MemoryAllocatorLinkedNode*>(pNextLinkNode);
        pResult->SetSize(requiredSize);
        pResult->SetPrevNode(_pStackTop);
        _pStackTop = pResult;

        return true;
    }

    template<size_t DefaultAlignment>
    void StackAllocator<DefaultAlignment>::Deallocate(void* p)
    {
        MemoryAllocatorLinkedNode* pHeader = MemoryAllocatorLinkedNode::BackStepToLinkNode<DefaultAlignment>(p);
        pHeader->SetUsed(false);

        if (_pStackTop == pHeader)
        {
            while (true)
            {
                if (_pStackTop == nullptr || _pStackTop->Used())
                    break;

                MemoryAllocatorLinkedNode* pPrevNode = _pStackTop->GetPrevNode();
                _pStackTop->ClearData();
                _pStackTop = pPrevNode;
            }
        }
    }

    template<size_t DefaultAlignment>
    MemoryAllocatorLinkedNode* StackAllocator<DefaultAlignment>::GetStackTop() const
    {
        return _pStackTop;
    }
}

