#pragma once

#include <atomic>
#include <memory>
#include <optional>
#include "SpinPause.h"

namespace Ailurus
{
    namespace _internal::LockFreeQueue
    {
        static constexpr uint32_t CACHE_LINE = 64;

        enum class State: uint8_t
        {
            Unloaded,
            Loading,
            Loaded,
            Unloading
        };

        template<typename Element>
        class BucketArray
        {
            static constexpr uint32_t ELEMENT_PRE_CACHELINE = CACHE_LINE / sizeof(Element);

        public:
            explicit BucketArray(uint32_t size)
                : _rawDataArray(static_cast<Element*>(::malloc(size * sizeof(Element))))
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

        template<typename T>
        class Storge
        {
        public:
            explicit Storge(uint32_t wantedSize)
                : size(UpAlignmentPowerOfTwo(wantedSize))
                , data(size)
                , state(size)
            {
            }

        public:
            // Padding index, avoid false sharing.
            alignas(CACHE_LINE) std::atomic<uint32_t> headIndex {};
            alignas(CACHE_LINE) std::atomic<uint32_t> tailIndex {};

            uint32_t size;
            BucketArray<T> data;
            BucketArray<std::atomic<State>> state;

        private:
            static uint32_t UpAlignmentPowerOfTwo(uint32_t value)
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
        };

        inline uint32_t RingBufferNextIndex(uint32_t current, uint32_t size)
        {
            if (current + 1 >= size)
                return 0;

            return current + 1;
        }

    }

    enum class LockFreeQueueStrategy
    {
        // SingleProducerSingleConsumer
        SPSC,
        // MultiProducerMultiConsumer
        MPMC
    };

    template<typename T, LockFreeQueueStrategy Strategy = LockFreeQueueStrategy::MPMC>
    class LockFreeQueue;

    template<typename T>
    class LockFreeQueue<T, LockFreeQueueStrategy::SPSC>
    {
        using State = _internal::LockFreeQueue::State;
        using Storge = _internal::LockFreeQueue::Storge<T>;
    public:
        explicit LockFreeQueue(uint32_t size)
            : _storge(size)
        {
        }

    public:
        uint32_t GetSize()
        {
            return _storge.size;
        }

        void Enqueue(T&& data)
        {
            // Safe get current head index
            uint32_t headIndex = _storge.headIndex.load();
            _storge.headIndex.store(headIndex + 1);

            // Clamp to real index
            uint32_t realIndex = headIndex & _storge.size - 1;

            // Check state, wait state to be empty.
            std::atomic<State>& state = _storge.state[realIndex];
            while (state.load(std::memory_order_acquire) != State::Unloaded)
                SpinPause();

            // Set data.
            _storge.data[realIndex] = std::forward<T>(data);

            // Set state.
            state.store(State::Loaded, std::memory_order_relaxed);
        }

        T Dequeue()
        {
            // Safe get current head index
            uint32_t tailIndex = _storge.tailIndex.load();
            _storge.tailIndex.store(tailIndex + 1);

            // Clamp to real index
            uint32_t realIndex = tailIndex & _storge.size - 1;

            // Check state, wait state to be loaded.
            std::atomic<State>& state = _storge.state[realIndex];
            while (state.load(std::memory_order_acquire) != State::Loaded)
                SpinPause();

            // Cache element.
            T element = std::move(_storge.data[realIndex]);

            // Set state.
            state.store(State::Unloaded, std::memory_order_relaxed);
            return element;
        }

        bool TryEnqueue(T&& data)
        {
        }

        std::optional<T> TryDequeue()
        {
        }

    private:
        Storge _storge;
    };


    template<typename T>
    class LockFreeQueue<T, LockFreeQueueStrategy::MPMC> : _internal::LockFreeQueue::Storge<T>
    {
        using State = _internal::LockFreeQueue::State;
        using Storge = _internal::LockFreeQueue::Storge<T>;
    public:
        explicit LockFreeQueue(uint32_t size, bool keepOrder)
            : _storge(size)
            , _keepOrder(keepOrder)
        {
        }

    public:
        uint32_t GetSize()
        {
            return _storge.size;
        }

        void Enqueue(T&& data)
        {
            // Safe get current head index
            uint32_t headIndex = _storge.headIndex.fetch_add(1, _keepOrder ? std::memory_order_seq_cst : std::memory_order_relaxed);

            // Clamp to real index
            uint32_t realIndex = headIndex & _storge.size - 1;

            // Check state, wait state to be empty.
            std::atomic<State>& state = _storge.state[realIndex];
            while (true)
            {
                State expected = State::Unloaded;
                if (state.compare_exchange_weak(expected, State::Loading, std::memory_order_acquire, std::memory_order_relaxed))
                {
                    // Set data.
                    _storge.data[realIndex] = std::forward<T>(data);

                    // Set state.
                    state.store(State::Loaded, std::memory_order_relaxed);
                    return;
                }

                SpinPause();
            }
        }

        T Dequeue()
        {
            // Safe get current head index
            uint32_t tailIndex = _storge.tailIndex.fetch_add(1, _keepOrder ? std::memory_order_seq_cst : std::memory_order_relaxed);

            // Clamp to real index
            uint32_t realIndex = tailIndex & _storge.size - 1;

            // Check state, wait state to be loaded.
            std::atomic<State>& state = _storge.state[realIndex];
            while (true)
            {
                State expected = State::Loaded;
                if (state.compare_exchange_weak(expected, State::Unloading, std::memory_order_acquire, std::memory_order_relaxed))
                {
                    // Cache element.
                    T element = std::move(_storge.data[realIndex]);

                    // Set state.
                    state.store(State::Unloaded, std::memory_order_relaxed);
                    return element;
                }

                SpinPause();
            }
        }

        bool TryEnqueue(T&& data)
        {
        }

        std::optional<T> TryDequeue()
        {
        }

    private:
        Storge _storge;
        bool _keepOrder;
    };
}
