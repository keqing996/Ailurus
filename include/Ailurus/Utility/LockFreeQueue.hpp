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

    template<typename T, LockFreeQueueStrategy Strategy = LockFreeQueueStrategy::MPMC, bool KeepOrder = false>
    class LockFreeQueue
    {
        static constexpr uint32_t CACHE_LINE = 64;

        enum class State: uint8_t
        {
            Empty,
            Enqueuing,
            Loaded,
            Dequeuing
        };

        template<typename Element>
        class BucketArray
        {
            static constexpr uint32_t ELEMENT_PRE_CACHELINE = CACHE_LINE / sizeof(Element);

        public:
            explicit BucketArray(uint32_t size)
                : _rawDataArray(::malloc(size * sizeof(Element)))
            {
                ::memset(_rawDataArray, 0, size * sizeof(Element));
            }

        public:
            Element& operator[](uint32_t index)
            {
                uint32_t bucketIndex = index / ELEMENT_PRE_CACHELINE;

                uint32_t inBucketIndex;
                if constexpr (IsPowerOfTwo(ELEMENT_PRE_CACHELINE))
                    inBucketIndex = index & (ELEMENT_PRE_CACHELINE - 1);
                else
                    inBucketIndex = index % ELEMENT_PRE_CACHELINE;

                uint32_t finalIndex = bucketIndex * ELEMENT_PRE_CACHELINE + inBucketIndex;
                return _rawDataArray[finalIndex];
            }

        private:
            static constexpr
            bool IsPowerOfTwo(uint32_t n)
            {
                return n > 0 && (n & (n - 1)) == 0;
            }

        private:
            Element* _rawDataArray;
        };

    public:
        explicit LockFreeQueue(uint32_t size)
            : _size(UpAlignmentPowerOfTwo(size))
            , _data(_size)
            , _state(_size)
        {
        }

    public:
        uint32_t GetSize() const
        {
            return _size;
        }

        void Enqueue(T&& data)
        {
            if constexpr (Strategy == LockFreeQueueStrategy::SPSC)
            {
                // Safe get current head index
                uint32_t headIndex = _headIndex.load();
                _headIndex.store(headIndex + 1);

                // Check state, wait state to be empty.
                std::atomic<State>& state = _state[headIndex];
                while (state.load(std::memory_order_acquire) != State::Empty)
                    SpinPause();

                // Set data.
                _data[headIndex] = std::forward<T>(data);

                // Set state.
                state.store(State::Loaded, std::memory_order_relaxed);
            }
            else
            {
                // Safe get current head index
                constexpr auto order = KeepOrder ? std::memory_order_seq_cst : std::memory_order_relaxed;
                uint32_t headIndex = _headIndex.fetch_add(1, order);

                // Check state, wait state to be empty.
                std::atomic<State>& state = _state[headIndex];
                while (true)
                {
                    State expected = State::Empty;
                    if (state.compare_exchange_weak(expected, State::Enqueuing, std::memory_order_acquire, std::memory_order_relaxed))
                    {
                        // Set data.
                        _data[headIndex] = std::forward<T>(data);

                        // Set state.
                        state.store(State::Loaded, std::memory_order_relaxed);
                        return;
                    }

                    SpinPause();
                }
            }
        }

        T Dequeue()
        {
            if constexpr (Strategy == LockFreeQueueStrategy::SPSC)
            {

            }
            else
            {

            }
        }

        bool TryEnqueue(T&& data)
        {
            if constexpr (Strategy == LockFreeQueueStrategy::SPSC)
            {

            }
            else
            {

            }
        }

        std::optional<T> TryDequeue()
        {
            if constexpr (Strategy == LockFreeQueueStrategy::SPSC)
            {

            }
            else
            {

            }
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



    private:
        // Padding index, avoid false sharing.
        alignas(CACHE_LINE) std::atomic<uint32_t> _headIndex {};
        alignas(CACHE_LINE) std::atomic<uint32_t> _tailIndex {};

        uint32_t _size;
        BucketArray<T> _data;
        BucketArray<std::atomic<State>> _state;
    };
}
