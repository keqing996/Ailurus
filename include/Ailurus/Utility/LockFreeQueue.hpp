#pragma once

#include <atomic>
#include <memory>
#include <optional>
#include "SpinPause.h"

namespace Ailurus
{
    enum class LockFreeQueueStrategy
    {
        // SingleProducerSingleConsumer
        SPSC,
        // MultiProducerMultiConsumer
        MPMC
    };

    template<typename T, LockFreeQueueStrategy Strategy = LockFreeQueueStrategy::MPMC>
    class LockFreeQueue
    {
        enum class State: uint8_t
        {
            Empty,
            Loading,
            Loaded
        };

        static constexpr uint32_t CACHE_LINE = 64;
        static constexpr uint32_t ELEMENT_PRE_CACHELINE = CACHE_LINE / sizeof(State);

    public:
        explicit LockFreeQueue(uint32_t size)
            : _size(UpAlignmentPowerOfTwo(size))
        {
            _dataArray = static_cast<T*>(::malloc(_size * sizeof(T)));
            _states = static_cast<State*>(::malloc(_size * sizeof(State)));

            ::memset(_states, 0, _size);
        }

    public:
        uint32_t GetSize() const
        {
            return _size;
        }

        void Enqueue(T&& data)
        {

        }

        T Dequeue()
        {

        }

        bool TryEnqueue(T&& data)
        {

        }

        std::optional<T> TryDequeue()
        {

        }

    private:
        static constexpr
        uint32_t UpAlignmentPowerOfTwo(uint32_t value)
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

        static AILURUS_FORCE_INLINE
        uint32_t StateIndexToArrayIndex(uint32_t stateIndex)
        {
            if constexpr (IsPowerOfTwo(ELEMENT_PRE_CACHELINE))
                return stateIndex & (ELEMENT_PRE_CACHELINE - 1);
            else
                return stateIndex % ELEMENT_PRE_CACHELINE;
        }

        static constexpr
        bool IsPowerOfTwo(uint32_t n)
        {
            return n > 0 && (n & (n - 1)) == 0;
        }

    private:
        alignas(CACHE_LINE) uint32_t _size;
        alignas(CACHE_LINE) T* _dataArray = nullptr;
        alignas(CACHE_LINE) State* _states = nullptr;
        alignas(CACHE_LINE) std::atomic<uint32_t> _headIndex {};
        alignas(CACHE_LINE) std::atomic<uint32_t> _tailIndex {};
    };
}
