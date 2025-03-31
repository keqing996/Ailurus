#pragma once

#include "Util.hpp"

namespace Ailurus
{
    class MemoryAllocatorLinkedNode
    {
    public:
        size_t GetSize() const
        {
            return _usedAndSize & ~MemoryAllocatorUtil::HIGHEST_BIT_MASK;
        }

        void SetSize(size_t size)
        {
            _usedAndSize = (_usedAndSize & MemoryAllocatorUtil::HIGHEST_BIT_MASK) | (size & ~MemoryAllocatorUtil::HIGHEST_BIT_MASK);
        }

        bool Used() const
        {
            return (_usedAndSize & MemoryAllocatorUtil::HIGHEST_BIT_MASK) != 0;
        }

        void SetUsed(bool used)
        {
            if (used)
                _usedAndSize |= MemoryAllocatorUtil::HIGHEST_BIT_MASK;
            else
                _usedAndSize &= ~MemoryAllocatorUtil::HIGHEST_BIT_MASK;
        }

        MemoryAllocatorLinkedNode* GetPrevNode() const
        {
            return _pPrev;
        }

        void SetPrevNode(MemoryAllocatorLinkedNode* prev)
        {
            _pPrev = prev;
        }

        void ClearData()
        {
            _pPrev = nullptr;
            _usedAndSize = 0;
        }

        template <size_t DefaultAlignment>
        MemoryAllocatorLinkedNode* MoveNext()
        {
            return MemoryAllocatorUtil::PtrOffsetBytes(this, GetSize() + PaddedSize<DefaultAlignment>());
        }

    public:
        template <size_t DefaultAlignment>
        static constexpr size_t PaddedSize()
        {
            return MemoryAllocatorUtil::GetPaddedSize<MemoryAllocatorLinkedNode, DefaultAlignment>();
        }

        template <size_t DefaultAlignment>
        static MemoryAllocatorLinkedNode* BackStepToLinkNode(void* ptr)
        {
            return static_cast<MemoryAllocatorLinkedNode*>(MemoryAllocatorUtil::PtrOffsetBytes(ptr, -PaddedSize<DefaultAlignment>()));
        }

    private:
        MemoryAllocatorLinkedNode* _pPrev;
        size_t _usedAndSize;
    };
}
