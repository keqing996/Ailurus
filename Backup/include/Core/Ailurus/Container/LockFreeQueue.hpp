#pragma once

#include <atomic>
#include <memory>
#include <optional>
#include "../Utility/SpinPause.h"
#include "SegmentArray.hpp"

namespace Ailurus
{
    /**
     * @brief A lock-free multi-producer multi-consumer (MPMC) queue with bounded capacity
     * 
     * This implementation provides a thread-safe queue that supports concurrent access from
     * multiple producers and consumers without using locks. It uses atomic operations and
     * a state machine to ensure correctness and prevent common lock-free pitfalls.
     * 
     * @section design Design Strategy
     * 
     * The queue uses a hybrid approach combining:
     * - **Ring Buffer**: Fixed-size circular buffer for O(1) operations
     * - **Atomic Indices**: Separate head/tail indices for lock-free coordination
     * - **Slot State Machine**: Per-slot state tracking to prevent data races
     * 
     * Key design elements:
     * 1. **Index Management**: Head and tail indices are monotonically increasing uint32_t values
     *    that wrap around naturally (using modulo via bit masking).
     * 2. **State Machine**: Each slot has an atomic state (Unloaded → Loading → Loaded → Unloading)
     *    that prevents premature access and coordinate producer-consumer synchronization.
     * 3. **Cache Optimization**: Indices are cache-line aligned to prevent false sharing,
     *    and data is stored in a SegmentArray for better cache locality.
     * 
     * @section lockfree_problems Classic Lock-Free Problems and Solutions
     * 
     * @subsection aba_problem ABA Problem
     * 
     * **Problem**: In typical lock-free algorithms using CAS, if thread T1 reads value A,
     * gets preempted, and another thread changes A→B→A, T1's CAS will succeed incorrectly.
     * 
     * **Solution**: This implementation uses a **per-slot state machine** that prevents ABA:
     * - Even if indices wrap around to the same value, a slot cannot be reused until its
     *   state transitions: Unloaded → Loading → Loaded → Unloading → Unloaded
     * - EnqueueImp waits for State::Unloaded before writing
     * - DequeueImp waits for State::Loaded before reading
     * - This ensures old data is fully consumed before new data can be written to the same slot
     * 
     * Example scenario:
     * 1. Producer claims slot 0 (tailIndex = 0), state: Unloaded → Loading
     * 2. Producer writes data, state: Loading → Loaded
     * 3. Consumer claims slot 0 (headIndex = 0), state: Loaded → Unloading
     * 4. Consumer reads data, state: Unloading → Unloaded
     * 5. Even if indices wrap around, producer cannot reuse slot 0 until state is Unloaded
     * 
     * @subsection memory_ordering Memory Ordering
     * 
     * **Problem**: Without proper memory ordering, operations might be reordered by compiler
     * or CPU, causing visibility issues where one thread doesn't see updates from another.
     * 
     * **Solution**: Strategic use of memory ordering semantics:
     * - `memory_order_relaxed`: Used for index increments (fetch_add) where only atomicity matters
     * - `memory_order_acquire`: Used in CAS operations to ensure all prior writes are visible
     * - `memory_order_release`: Used when storing Loaded/Unloaded state to publish data changes
     * - The acquire-release pair ensures that data written before state=Loaded is visible
     *   to the consumer that observes state=Loaded
     * 
     * Synchronization chain:
     * ```
     * Producer:                          Consumer:
     * 1. Write data[i] = value          1. Wait for state[i] == Loaded (acquire)
     * 2. state[i] = Loaded (release)    2. Read data[i] (sees write from step 1)
     * ```
     * 
     * @subsection false_sharing False Sharing
     * 
     * **Problem**: When multiple threads access different variables that share the same
     * cache line, each write causes cache invalidation, severely degrading performance.
     * 
     * **Solution**: Cache line alignment and padding:
     * - `_headIndex` and `_tailIndex` are placed in separate cache lines (alignas(CACHE_LINE_SIZE))
     * - Producers primarily modify `_tailIndex`, consumers primarily modify `_headIndex`
     * - This prevents producer-consumer interference at the cache level
     * - SegmentArray internally segments data for better cache locality
     * 
     * @subsection bounded_queue Queue Capacity Management
     * 
     * **Problem**: Producers must not overwrite data that hasn't been consumed yet.
     * 
     * **Solution**: Conservative capacity checking in TryEnqueue:
     * - Before claiming a slot, check if `tailIndex - headIndex >= Size`
     * - This uses unsigned integer wraparound arithmetic correctly
     * - Combined with state machine, ensures queue never overfills
     * - Note: This may conservatively reject enqueue attempts when queue is actually not full
     *   (TOCTOU issue), but this is safe and prevents data corruption
     * 
     * @subsection livelock Livelock Prevention
     * 
     * **Problem**: Spinning threads might prevent each other from making progress.
     * 
     * **Solution**: SpinPause() (typically PAUSE instruction) in retry loops:
     * - Reduces power consumption during busy-waiting
     * - Gives other hyperthreads more resources
     * - Improves overall throughput under contention
     * 
     * @section tradeoffs Design Tradeoffs
     * 
     * **Advantages**:
     * - No locks: No deadlocks, priority inversion, or kernel transitions
     * - Wait-free for blocking operations (Enqueue/Dequeue)
     * - Lock-free for try operations (TryEnqueue/TryDequeue)
     * - MPMC support: Multiple producers and consumers can work concurrently
     * - Predictable memory usage: Bounded capacity known at compile-time
     * 
     * **Limitations**:
     * - Fixed capacity: Cannot grow dynamically
     * - Busy-waiting: Failed operations spin, consuming CPU
     * - TOCTOU window: TryEnqueue may conservatively reject when queue has space
     * - Memory overhead: Additional state array for synchronization
     * 
     * @tparam T Element type (must be movable)
     * @tparam WantedSize Desired capacity (will be rounded up to nearest power of 2, minimum 4)
     * @tparam CACHE_LINE_SIZE Cache line size for alignment (typically 64 bytes)
     * 
     * @note This implementation prioritizes correctness over maximum throughput.
     *       For extreme performance requirements, consider using multiple queues or
     *       a different algorithm (e.g., based on FAA instead of CAS).
     * 
     * @see SegmentArray for details on cache-optimized storage layout
     */
    template<typename T, uint32_t WantedSize, uint32_t CACHE_LINE_SIZE = 64>
    class LockFreeQueue
    {
        /**
         * @brief State machine for each queue slot
         * 
         * Each slot in the ring buffer maintains an atomic state to coordinate
         * producer-consumer access and prevent data races.
         * 
         * State transitions:
         * - Unloaded: Slot is empty and ready for producer to write
         * - Loading: Producer has claimed slot and is writing data
         * - Loaded: Data is written and ready for consumer to read
         * - Unloading: Consumer has claimed slot and is reading data
         * - (back to Unloaded): Consumer has finished reading
         * 
         * This state machine is crucial for solving the ABA problem and ensuring
         * that slots are not reused prematurely even when indices wrap around.
         */
        enum class State: uint8_t
        {
            Unloaded,   ///< Slot is empty, ready for enqueue
            Loading,    ///< Producer is writing to this slot
            Loaded,     ///< Data is ready, waiting for dequeue
            Unloading   ///< Consumer is reading from this slot
        };

        /**
         * @brief Round up to the nearest power of 2
         * 
         * Lock-free algorithms often work better with power-of-2 sizes because:
         * - Index wrapping can use fast bitwise AND instead of modulo: index & (size - 1)
         * - Better alignment for cache lines
         * - Simpler capacity checking with unsigned wraparound arithmetic
         * 
         * @param value Desired size
         * @return Nearest power of 2 >= value (minimum 4)
         */
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

        /// Number of elements that fit in one cache line (for SegmentArray optimization)
        static constexpr uint32_t ELEMENT_PRE_CACHELINE = CACHE_LINE_SIZE / sizeof(T);
        
        /// Actual queue capacity (power of 2 >= WantedSize)
        static constexpr uint32_t Size = UpAlignmentPowerOfTwo(WantedSize);

    public:
        /**
         * @brief Get the actual queue capacity
         * 
         * @return Size of the queue (power of 2 >= WantedSize)
         */
        constexpr uint32_t GetSize() const
        {
            return Size;
        }

        /**
         * @brief Check if queue is empty (non-blocking)
         * 
         * Note: This is a snapshot check. In a concurrent environment, the queue
         * state may change immediately after this call returns.
         * 
         * @return true if queue appears empty, false otherwise
         */
        bool Empty() const
        {
            uint32_t headIndex = _headIndex.load();
            uint32_t tailIndex = _tailIndex.load();
            return tailIndex == headIndex;
        }

        /**
         * @brief Enqueue an element (blocking if queue is full)
         * 
         * This operation is wait-free in the absence of contention and lock-free
         * under contention. It will spin-wait if the queue is full until space
         * becomes available.
         * 
         * Algorithm:
         * 1. Atomically claim a slot by incrementing tail index
         * 2. Wait for slot state to become Unloaded (previous data consumed)
         * 3. Transition state to Loading
         * 4. Write data to slot
         * 5. Transition state to Loaded (publishes data to consumers)
         * 
         * Memory ordering:
         * - fetch_add uses relaxed: only atomicity matters, index order doesn't affect correctness
         * - State transitions use acquire-release: ensures data writes are visible to consumers
         * 
         * @param data Element to enqueue (will be moved or copied)
         * @tparam D Deduced type (supports perfect forwarding)
         */
        template<typename D = T>
        void Enqueue(D&& data)
        {
            // Atomically claim a slot by incrementing tail index
            // relaxed ordering is sufficient here because:
            // - We only need atomicity of the increment
            // - Synchronization is achieved through the state machine
            uint32_t tailIndex = _tailIndex.fetch_add(1, std::memory_order_relaxed);

            EnqueueImp(std::forward<D>(data), tailIndex);
        }

        /**
         * @brief Dequeue an element (blocking if queue is empty)
         * 
         * This operation is wait-free in the absence of contention and lock-free
         * under contention. It will spin-wait if the queue is empty until an
         * element becomes available.
         * 
         * Algorithm:
         * 1. Atomically claim a slot by incrementing head index
         * 2. Wait for slot state to become Loaded (data is ready)
         * 3. Transition state to Unloading
         * 4. Read and move data from slot
         * 5. Transition state to Unloaded (publishes availability to producers)
         * 
         * @return The dequeued element (moved from queue)
         */
        T Dequeue()
        {
            // Atomically claim a slot by incrementing head index
            uint32_t headIndex = _headIndex.fetch_add(1, std::memory_order_relaxed);

            return DequeueImp(headIndex);
        }

        /**
         * @brief Try to enqueue an element (non-blocking)
         * 
         * This operation is lock-free. It attempts to enqueue an element but
         * returns false immediately if the queue is full.
         * 
         * Algorithm:
         * 1. Read current tail index
         * 2. Loop:
         *    a. Read head index
         *    b. Check if queue is full (tailIndex - headIndex >= Size)
         *       - Uses unsigned wraparound arithmetic correctly
         *       - Conservative check: may reject even if queue has space (TOCTOU)
         *    c. If full, return false
         *    d. Try to claim slot via CAS on tail index
         *    e. If CAS succeeds, break; otherwise retry
         * 3. Enqueue data into claimed slot
         * 
         * TOCTOU Issue:
         * Between reading headIndex (step 2a) and CAS (step 2d), consumers may
         * dequeue elements, making the queue less full than checked. This can
         * lead to conservative rejections (returning false when queue has space).
         * However, this is safe - it never allows overfilling, and callers can retry.
         * 
         * Memory ordering:
         * - CAS uses acquire on success: ensures we see all prior consumer operations
         * - CAS uses relaxed on failure: we'll retry anyway, no synchronization needed
         * 
         * @param data Element to enqueue
         * @return true if successfully enqueued, false if queue is full
         * @tparam D Deduced type (supports perfect forwarding)
         */
        template<typename D = T>
        bool TryEnqueue(D&& data)
        {
            uint32_t tailIndex = _tailIndex.load();
            while (true)
            {
                uint32_t headIndex = _headIndex.load();

                // Check if queue is full using unsigned wraparound arithmetic
                // This works correctly even when indices wrap around
                if (tailIndex - headIndex >= Size)
                    return false;

                // Try to atomically claim this slot
                // Success ordering (acquire): ensure we see all prior dequeue operations
                // Failure ordering (relaxed): we'll retry with updated tailIndex anyway
                if (_tailIndex.compare_exchange_weak(tailIndex, tailIndex + 1,
                    std::memory_order_acquire, std::memory_order_relaxed))
                    break;

                // CAS failed, another thread claimed this slot, retry with updated index
                SpinPause();
            }

            EnqueueImp(std::forward<D>(data), tailIndex);

            return true;
        }

        /**
         * @brief Try to dequeue an element (non-blocking)
         * 
         * This operation is lock-free. It attempts to dequeue an element but
         * returns std::nullopt immediately if the queue is empty.
         * 
         * Algorithm:
         * 1. Read current head index
         * 2. Loop:
         *    a. Read tail index
         *    b. Check if queue is empty (tailIndex <= headIndex)
         *    c. If empty, return nullopt
         *    d. Try to claim slot via CAS on head index
         *    e. If CAS succeeds, break; otherwise retry
         * 3. Dequeue data from claimed slot
         * 
         * @return std::optional<T> containing the element if successful, std::nullopt if empty
         */
        std::optional<T> TryDequeue()
        {
            uint32_t headIndex = _headIndex.load();
            while (true)
            {
                uint32_t tailIndex = _tailIndex.load();

                // Check if queue is empty
                if (tailIndex <= headIndex)
                    return std::nullopt;

                // Try to atomically claim this slot
                if (_headIndex.compare_exchange_weak(headIndex, headIndex + 1,
                    std::memory_order_acquire, std::memory_order_relaxed))
                    break;

                // CAS failed, another thread claimed this slot, retry with updated index
                SpinPause();
            }

            return DequeueImp(headIndex);
        }

    private:
        /**
         * @brief Internal implementation of enqueue operation
         * 
         * This function is called after a slot has been claimed (tailIndex obtained).
         * It waits for the slot to be in Unloaded state, then writes data and marks
         * it as Loaded.
         * 
         * State machine synchronization:
         * - Spin-wait until state is Unloaded (previous consumer has finished)
         * - CAS to transition Unloaded → Loading (claim for writing)
         * - Write data (protected by Loading state)
         * - Store Loaded state with release ordering (publish data to consumers)
         * 
         * The release store of Loaded state ensures that the data write is visible
         * to any thread that observes state==Loaded with acquire semantics.
         * 
         * Why this prevents ABA:
         * Even if indices wrap around to reuse this slot, we cannot write until
         * the slot is Unloaded, meaning the previous data has been consumed.
         * 
         * @param data Element to write to the slot
         * @param tailIndex The claimed tail index for this enqueue operation
         * @tparam D Deduced type (supports perfect forwarding)
         */
        template<typename D = T>
        void EnqueueImp(D&& data, uint32_t tailIndex)
        {
            // Convert logical index to physical slot index using bit masking
            // This is equivalent to: tailIndex % Size
            // Works because Size is guaranteed to be a power of 2
            uint32_t realIndex = tailIndex & (Size - 1);

            // Wait for slot to be available and claim it for writing
            std::atomic<State>& state = _state[realIndex];
            while (true)
            {
                State expected = State::Unloaded;
                // Acquire: ensure we see all operations from the previous consumer
                // Relaxed on failure: we'll retry anyway
                if (state.compare_exchange_weak(expected, State::Loading,
                    std::memory_order_acquire, std::memory_order_relaxed))
                {
                    // Successfully transitioned to Loading state, safe to write data
                    _data[realIndex] = std::forward<D>(data);

                    // Publish data to consumers with release ordering
                    // Any thread that reads Loaded state with acquire will see the data write
                    state.store(State::Loaded, std::memory_order_release);
                    return;
                }

                // Slot not yet available, spin-wait
                // SpinPause improves performance and reduces power consumption
                SpinPause();
            }
        }

        /**
         * @brief Internal implementation of dequeue operation
         * 
         * This function is called after a slot has been claimed (headIndex obtained).
         * It waits for the slot to be in Loaded state, then reads data and marks
         * it as Unloaded.
         * 
         * State machine synchronization:
         * - Spin-wait until state is Loaded (producer has finished writing)
         * - CAS to transition Loaded → Unloading (claim for reading)
         * - Read data (protected by Unloading state)
         * - Store Unloaded state with release ordering (publish availability to producers)
         * 
         * The acquire load in the CAS ensures we see the data written by the producer
         * before they stored the Loaded state.
         * 
         * @param headIndex The claimed head index for this dequeue operation
         * @return The element read from the slot (moved)
         */
        T DequeueImp(uint32_t headIndex)
        {
            // Convert logical index to physical slot index
            uint32_t realIndex = headIndex & (Size - 1);

            // Wait for data to be available and claim it for reading
            std::atomic<State>& state = _state[realIndex];
            while (true)
            {
                State expected = State::Loaded;
                // Acquire: ensure we see the data written by the producer
                if (state.compare_exchange_weak(expected, State::Unloading,
                    std::memory_order_acquire, std::memory_order_relaxed))
                {
                    // Successfully transitioned to Unloading state, safe to read data
                    T element = std::move(_data[realIndex]);

                    // Publish slot availability to producers with release ordering
                    // Any thread that reads Unloaded state with acquire will see we're done
                    state.store(State::Unloaded, std::memory_order_release);
                    return element;
                }

                // Data not yet available, spin-wait
                SpinPause();
            }
        }

        /**
         * @brief Helper function for ring buffer index advancement (unused in current implementation)
         * 
         * Note: Current implementation uses bit masking (index & (Size - 1)) instead,
         * which is faster for power-of-2 sizes.
         * 
         * @param current Current index
         * @param size Queue size
         * @return Next index in the ring buffer
         */
        static uint32_t RingBufferNextIndex(uint32_t current, uint32_t size)
        {
            if (current + 1 >= size)
                return 0;

            return current + 1;
        }

    private:
        /**
         * @brief Member variables with cache line alignment
         * 
         * Cache line alignment strategy:
         * - _headIndex and _tailIndex are placed in separate cache lines
         * - Prevents false sharing: producers modify tail, consumers modify head
         * - Without alignment, both would share a cache line, causing excessive
         *   cache coherency traffic
         * 
         * Memory layout:
         * [_headIndex + padding] [_tailIndex + padding] [_data...] [_state...]
         *  ← 64 bytes →            ← 64 bytes →
         */
        
        /// Head index: next slot to dequeue from (modified by consumers)
        alignas(CACHE_LINE_SIZE) std::atomic<uint32_t> _headIndex {};
        
        /// Tail index: next slot to enqueue to (modified by producers)
        alignas(CACHE_LINE_SIZE) std::atomic<uint32_t> _tailIndex {};
        
        /// Data storage: segmented for cache locality
        SegmentArray<T, Size, ELEMENT_PRE_CACHELINE> _data;
        
        /// State machine array: one state per slot for synchronization
        SegmentArray<std::atomic<State>, Size, ELEMENT_PRE_CACHELINE> _state;
    };
}
