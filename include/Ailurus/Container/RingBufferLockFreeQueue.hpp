#pragma once

#include <atomic>
#include <memory>
#include <optional>
#include "../Utility/SpinPause.h"
#include "SegmentArray.hpp"

namespace Ailurus
{
    template<typename T, uint32_t WantedSize, bool KeepOrder = true>
    class RingBufferLockFreeQueue
    {
        enum class State: uint8_t
        {
            Unloaded,
            Loading,
            Loaded,
            Unloading
        };

        static constexpr uint32_t UpAlignmentPowerOfTwo(uint32_t value)
        {
            if (value <= 4)
                return 4;

            value--;
            value |= value >> 1;
            value |= value >> 2;
            value |= value >> 4;
            value |= value >> 8;
            value |= value >> 16;

            return value + 1;
        }

        static constexpr uint32_t CACHE_LINE = 64;
        static constexpr uint32_t ELEMENT_PRE_CACHELINE = CACHE_LINE / sizeof(T);
        static constexpr uint32_t Size = UpAlignmentPowerOfTwo(WantedSize);

    public:
        constexpr uint32_t GetSize() const
        {
            return Size;
        }

        bool Empty() const
        {
            uint32_t headIndex = _headIndex.load();
            uint32_t tailIndex = _tailIndex.load();
            return tailIndex == headIndex;
        }

        template<typename D = T>
        void Enqueue(D&& data)
        {
            // Safe get current head index
            uint32_t tailIndex = _tailIndex.fetch_add(1, KeepOrder ? std::memory_order_seq_cst : std::memory_order_relaxed);

            EnqueueImp(std::forward<D>(data), tailIndex);
        }

        T Dequeue()
        {
            // Safe get current head index
            uint32_t headIndex = _headIndex.fetch_add(1, KeepOrder ? std::memory_order_seq_cst : std::memory_order_relaxed);

            return DequeueImp(headIndex);
        }

        template<typename D = T>
        bool TryEnqueue(D&& data)
        {
            uint32_t tailIndex = _tailIndex.load();
            while (true)
            {
                uint32_t headIndex = _headIndex.load();

                if (tailIndex - headIndex >= Size)
                    return false;

                if (_tailIndex.compare_exchange_weak(tailIndex, tailIndex + 1, std::memory_order_acquire, std::memory_order_relaxed))
                    break;

                SpinPause();
            }

            EnqueueImp(std::forward<D>(data), tailIndex);

            return true;
        }

        std::optional<T> TryDequeue()
        {
            uint32_t headIndex = _headIndex.load();
            while (true)
            {
                uint32_t tailIndex = _tailIndex.load();

                if (tailIndex <= headIndex)
                    return std::nullopt;

                if (_headIndex.compare_exchange_weak(headIndex, headIndex + 1, std::memory_order_acquire, std::memory_order_relaxed))
                    break;

                SpinPause();
            }

            return DequeueImp(headIndex);
        }

    private:
        template<typename D = T>
        void EnqueueImp(D&& data, uint32_t tailIndex)
        {
            // Clamp to real index
            uint32_t realIndex = tailIndex & (Size - 1);

            // Check state, wait state to be empty.
            std::atomic<State>& state = _state[realIndex];
            while (true)
            {
                State expected = State::Unloaded;
                if (state.compare_exchange_weak(expected, State::Loading, std::memory_order_acquire, std::memory_order_relaxed))
                {
                    // Set data.
                    _data[realIndex] = std::forward<D>(data);

                    // Set state.
                    state.store(State::Loaded, std::memory_order_release);
                    return;
                }

                SpinPause();
            }
        }

        T DequeueImp(uint32_t headIndex)
        {
            // Clamp to real index
            uint32_t realIndex = headIndex & (Size - 1);

            // Check state, wait state to be loaded.
            std::atomic<State>& state = _state[realIndex];
            while (true)
            {
                State expected = State::Loaded;
                if (state.compare_exchange_weak(expected, State::Unloading, std::memory_order_acquire, std::memory_order_relaxed))
                {
                    // Cache element.
                    T element = std::move(_data[realIndex]);

                    // Set state.
                    state.store(State::Unloaded, std::memory_order_release);
                    return element;
                }

                SpinPause();
            }
        }

        static uint32_t RingBufferNextIndex(uint32_t current, uint32_t size)
        {
            if (current + 1 >= size)
                return 0;

            return current + 1;
        }

    private:
        // Padding index, avoid false sharing.
        alignas(CACHE_LINE) std::atomic<uint32_t> _headIndex {};
        alignas(CACHE_LINE) std::atomic<uint32_t> _tailIndex {};
        SegmentArray<T, Size, ELEMENT_PRE_CACHELINE> _data;
        SegmentArray<std::atomic<State>, Size, ELEMENT_PRE_CACHELINE> _state;
    };
}
