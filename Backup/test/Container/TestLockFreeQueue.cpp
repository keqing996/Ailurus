#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include <format>
#include <numeric>
#include <thread>
#include <random>
#include <Ailurus/Container/LockFreeQueue.hpp>

using namespace Ailurus;

TEST_SUITE("LockFreeQueue")
{
    TEST_CASE("Up alignment")
    {
        {
            LockFreeQueue<int, 0> lockFreeQueue;
            CHECK_EQ(lockFreeQueue.GetSize(), 4);
        }
        {
            LockFreeQueue<int, 127> lockFreeQueue;
            CHECK_EQ(lockFreeQueue.GetSize(), 128);
        }
        {
            LockFreeQueue<int, 128> lockFreeQueue;
            CHECK_EQ(lockFreeQueue.GetSize(), 128);
        }
        {
            LockFreeQueue<int, 129> lockFreeQueue;
            CHECK_EQ(lockFreeQueue.GetSize(), 256);
        }
    }

    TEST_CASE("Basic")
    {
        LockFreeQueue<int, 5> lockFreeQueue;

        lockFreeQueue.Enqueue(1);
        lockFreeQueue.Enqueue(2);
        lockFreeQueue.Enqueue(3);
        lockFreeQueue.Enqueue(4);
        lockFreeQueue.Enqueue(5);
        lockFreeQueue.Enqueue(6);
        lockFreeQueue.Enqueue(7);
        lockFreeQueue.Enqueue(8);

        CHECK_EQ(false, lockFreeQueue.TryEnqueue(9));

        CHECK_EQ(lockFreeQueue.Dequeue(), 1);
        CHECK_EQ(lockFreeQueue.Dequeue(), 2);
        CHECK_EQ(lockFreeQueue.Dequeue(), 3);
        CHECK_EQ(lockFreeQueue.Dequeue(), 4);
        CHECK_EQ(lockFreeQueue.Dequeue(), 5);
        CHECK_EQ(lockFreeQueue.Dequeue(), 6);
        CHECK_EQ(lockFreeQueue.Dequeue(), 7);
        CHECK_EQ(lockFreeQueue.Dequeue(), 8);

        auto ret = lockFreeQueue.TryDequeue();
        CHECK_EQ(ret.has_value(), false);

        CHECK_EQ(lockFreeQueue.TryEnqueue(1), true);
        CHECK_EQ(lockFreeQueue.TryEnqueue(2), true);
        CHECK_EQ(lockFreeQueue.TryEnqueue(3), true);
        CHECK_EQ(lockFreeQueue.TryEnqueue(4), true);

        ret = lockFreeQueue.TryDequeue();
        CHECK_EQ(ret.has_value(), true);
        CHECK_EQ(*ret, 1);

        ret = lockFreeQueue.TryDequeue();
        CHECK_EQ(ret.has_value(), true);
        CHECK_EQ(*ret, 2);

        ret = lockFreeQueue.TryDequeue();
        CHECK_EQ(ret.has_value(), true);
        CHECK_EQ(*ret, 3);

        ret = lockFreeQueue.TryDequeue();
        CHECK_EQ(ret.has_value(), true);
        CHECK_EQ(*ret, 4);
    }

    TEST_CASE("SPSC - try enqueue & try dequeue")
    {
        constexpr uint32_t queueSize = 20;
        constexpr uint32_t dataSize = 500;

        LockFreeQueue<int, queueSize> lockFreeQueue;
        std::vector<int> result;
        result.reserve(dataSize);
        std::atomic<bool> finish { false };

        size_t enqueueFailCount = 0;
        std::thread producer([&]() -> void
        {
            for (auto i = 0; i < dataSize; i++)
            {
                while (true)
                {
                    bool success = lockFreeQueue.TryEnqueue(i);
                    if (success)
                        break;

                    SpinPause();

                    enqueueFailCount++;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(5));

            finish.store(true);
        });

        size_t dequeueFailCount = 0;
        std::thread consumer([&]()->void
        {
            while (true)
            {
                if (finish.load())
                    break;

                auto ret = lockFreeQueue.TryDequeue();
                if (ret.has_value())
                    result.push_back(*ret);
                else
                {
                    dequeueFailCount++;
                    SpinPause();
                }
            }
        });

        producer.join();
        consumer.join();

        CHECK_EQ(result.size(), dataSize);
        for (int i = 0; i < result.size(); i++)
            CHECK_EQ(result[i], i);

        MESSAGE(std::format("enqueueFailCount = {}", enqueueFailCount));
        MESSAGE(std::format("dequeueFailCount = {}", dequeueFailCount));
    }

    TEST_CASE("SPSC - block enqueue & block dequeue")
    {
        constexpr uint32_t queueSize = 20;
        constexpr uint32_t dataSize = 500;

        LockFreeQueue<int, queueSize> lockFreeQueue;
        std::vector<int> result;
        result.reserve(dataSize);

        std::thread producer([&]() -> void
        {
            for (auto i = 0; i < dataSize; i++)
                lockFreeQueue.Enqueue(i);
        });

        std::thread consumer([&]()->void
        {
            while (result.size() < dataSize)
                result.push_back(lockFreeQueue.Dequeue());
        });

        producer.join();
        consumer.join();

        CHECK_EQ(result.size(), dataSize);
        for (int i = 0; i < result.size(); i++)
            CHECK_EQ(result[i], i);
    }

    TEST_CASE("MPMC")
    {
        LockFreeQueue<int, 20> lockFreeQueue;

        std::vector<std::thread> producers;
        std::vector<std::thread> consumers;
        std::atomic<bool> finish { false };

        producers.reserve(5);
        consumers.reserve(5);

        std::vector<std::vector<int>> producerResults;
        producerResults.resize(5);
        for (auto& producerResult: producerResults)
            producerResult.reserve(1000);

        std::vector<std::vector<int>> consumerResults;
        consumerResults.resize(5);
        for (auto& consumerResult: consumerResults)
            consumerResult.reserve(1000);

        for (auto i = 0; i < 5; i++)
        {
            producers.emplace_back([&, i]() -> void
            {
                std::random_device rd;
                std::mt19937 mt(rd());
                std::uniform_int_distribution<int> dist(0, 100);

                for (auto num = 0 + i * 1000; num < 1000 + i * 1000; num++)
                {
                    int data = dist(mt);
                    producerResults[i].push_back(data);
                    lockFreeQueue.Enqueue(data);
                }
            });
        }

        for (auto i = 0; i < 5; i++)
        {
            consumers.emplace_back([&, i]() -> void
            {
                while (true)
                {
                    if (finish.load() && lockFreeQueue.Empty())
                        break;

                    auto ret = lockFreeQueue.TryDequeue();
                    if (ret.has_value())
                        consumerResults[i].push_back(*ret);
                }
            });
        }

        for (auto& producer: producers)
            producer.join();

        finish.store(true);

        for (auto& consumer: consumers)
            consumer.join();

        int64_t originalResult = 0;
        int64_t finalResult = 0;

        for (auto& producerResult: producerResults)
            originalResult += std::accumulate(producerResult.begin(), producerResult.end(), 0);

        for (auto& consumerResult: consumerResults)
            finalResult += std::accumulate(consumerResult.begin(), consumerResult.end(), 0);

        CHECK_EQ(originalResult, finalResult);
    }

    TEST_CASE("Race condition in TryEnqueue - queue full check")
    {
        // Test whether queue full check is accurate when multiple threads try to enqueue concurrently
        constexpr uint32_t queueSize = 8;
        constexpr uint32_t producerCount = 4;
        constexpr uint32_t itemsPerProducer = 100;

        LockFreeQueue<int, queueSize> lockFreeQueue;
        std::atomic<int> successCount { 0 };
        std::atomic<int> failCount { 0 };
        std::atomic<bool> start { false };

        std::vector<std::thread> producers;
        producers.reserve(producerCount);

        for (uint32_t i = 0; i < producerCount; i++)
        {
            producers.emplace_back([&, i]() -> void
            {
                // Wait for all threads to be ready
                while (!start.load()) {
                    std::this_thread::yield();
                }

                for (uint32_t j = 0; j < itemsPerProducer; j++)
                {
                    int value = i * 1000 + j;
                    bool success = lockFreeQueue.TryEnqueue(value);
                    if (success)
                        successCount.fetch_add(1);
                    else
                        failCount.fetch_add(1);
                }
            });
        }

        // Start a consumer thread to continuously consume items
        std::atomic<bool> stopConsumer { false };
        std::atomic<int> consumedCount { 0 };
        std::thread consumer([&]() -> void
        {
            while (!stopConsumer.load())
            {
                auto item = lockFreeQueue.TryDequeue();
                if (item.has_value())
                    consumedCount.fetch_add(1);
                else
                    std::this_thread::yield();
            }

            // Drain remaining elements
            while (true)
            {
                auto item = lockFreeQueue.TryDequeue();
                if (!item.has_value())
                    break;
                consumedCount.fetch_add(1);
            }
        });

        start.store(true);

        for (auto& producer : producers)
            producer.join();

        stopConsumer.store(true);
        consumer.join();

        MESSAGE(std::format("Success: {}, Failed: {}, Consumed: {}", 
                successCount.load(), failCount.load(), consumedCount.load()));
        
        // Number of successfully enqueued items should equal number of consumed items
        CHECK_EQ(successCount.load(), consumedCount.load());
    }

    TEST_CASE("Index wraparound test")
    {
        // Test correctness when indices wrap around
        constexpr uint32_t queueSize = 8;
        LockFreeQueue<int, queueSize> lockFreeQueue;

        // Simulate index wraparound through massive operations
        // Cannot directly manipulate _tailIndex as it's private
        // So we test through a large number of enqueue/dequeue operations
        constexpr uint32_t iterations = 10000;

        for (uint32_t i = 0; i < iterations; i++)
        {
            // Fill the queue
            for (uint32_t j = 0; j < queueSize; j++)
            {
                lockFreeQueue.Enqueue(static_cast<int>(i * queueSize + j));
            }

            // Drain the queue and verify
            for (uint32_t j = 0; j < queueSize; j++)
            {
                int value = lockFreeQueue.Dequeue();
                CHECK_EQ(value, static_cast<int>(i * queueSize + j));
            }
        }

        CHECK(lockFreeQueue.Empty());
    }

    TEST_CASE("ABA problem test")
    {
        // Test ABA problem: a thread reads an index value and gets suspended,
        // other threads complete a full cycle during this time, and the index returns to the same value
        constexpr uint32_t queueSize = 4;
        LockFreeQueue<int, queueSize> lockFreeQueue;
        
        std::atomic<bool> ready { false };
        std::atomic<int> stage { 0 };
        std::vector<int> dequeued;
        std::mutex dequeuedMutex;

        // Thread 1: slow consumer
        std::thread slowConsumer([&]() -> void
        {
            // Wait for queue to be filled
            while (stage.load() < 1) {
                std::this_thread::yield();
            }

            // Try to dequeue, but pause in the middle
            auto item = lockFreeQueue.TryDequeue();
            if (item.has_value())
            {
                std::lock_guard<std::mutex> lock(dequeuedMutex);
                dequeued.push_back(*item);
            }
        });

        // Thread 2: fast produce-consume loop
        std::thread fastLoop([&]() -> void
        {
            // Wait for initial filling
            while (stage.load() < 1) {
                std::this_thread::yield();
            }

            stage.store(2);

            // Execute multiple rounds of fast enqueue-dequeue to trigger ABA
            for (int round = 0; round < 100; round++)
            {
                // Fill the queue
                for (int i = 0; i < queueSize; i++)
                {
                    lockFreeQueue.TryEnqueue(1000 + round * queueSize + i);
                }

                // Drain the queue
                for (int i = 0; i < queueSize; i++)
                {
                    auto item = lockFreeQueue.TryDequeue();
                    if (item.has_value())
                    {
                        std::lock_guard<std::mutex> lock(dequeuedMutex);
                        dequeued.push_back(*item);
                    }
                }
            }
        });

        // Initial queue filling
        for (int i = 0; i < queueSize; i++)
        {
            lockFreeQueue.Enqueue(i);
        }

        stage.store(1);

        slowConsumer.join();
        fastLoop.join();

        MESSAGE(std::format("Total dequeued items: {}", dequeued.size()));
        
        // Verify no duplicate consumption or data loss
        // This test mainly checks if it crashes or produces abnormal behavior
        CHECK(dequeued.size() > 0);
    }

    TEST_CASE("Memory ordering test - visibility")
    {
        // Test if memory ordering correctly ensures data visibility
        constexpr uint32_t queueSize = 16;
        constexpr uint32_t iterations = 1000;
        
        for (uint32_t iter = 0; iter < iterations; iter++)
        {
            LockFreeQueue<std::pair<int, int>, queueSize> lockFreeQueue;
            std::atomic<bool> start { false };
            std::atomic<bool> producerDone { false };

            std::thread producer([&]() -> void
            {
                while (!start.load()) {
                    std::this_thread::yield();
                }

                for (int i = 0; i < 10; i++)
                {
                    lockFreeQueue.Enqueue(std::make_pair(i, i * 2));
                }
                
                producerDone.store(true, std::memory_order_release);
            });

            std::thread consumer([&]() -> void
            {
                while (!start.load()) {
                    std::this_thread::yield();
                }

                int consumed = 0;
                while (consumed < 10)
                {
                    auto item = lockFreeQueue.TryDequeue();
                    if (item.has_value())
                    {
                        auto [first, second] = *item;
                        // Verify data consistency
                        CHECK_EQ(second, first * 2);
                        consumed++;
                    }
                    else if (producerDone.load(std::memory_order_acquire))
                    {
                        // If producer is done, continue trying to consume remaining elements
                        continue;
                    }
                    else
                    {
                        std::this_thread::yield();
                    }
                }
            });

            start.store(true);
            producer.join();
            consumer.join();
        }
    }

    TEST_CASE("Concurrent size stress test")
    {
        // Stress test: massive concurrent operations to verify queue never exceeds capacity
        constexpr uint32_t queueSize = 32;
        constexpr uint32_t duration_ms = 500;
        
        LockFreeQueue<int, queueSize> lockFreeQueue;
        std::atomic<bool> stop { false };
        std::atomic<uint64_t> enqueueAttempts { 0 };
        std::atomic<uint64_t> enqueueSuccess { 0 };
        std::atomic<uint64_t> dequeueAttempts { 0 };
        std::atomic<uint64_t> dequeueSuccess { 0 };

        auto producerFunc = [&]() -> void
        {
            while (!stop.load())
            {
                enqueueAttempts.fetch_add(1);
                if (lockFreeQueue.TryEnqueue(42))
                    enqueueSuccess.fetch_add(1);
            }
        };

        auto consumerFunc = [&]() -> void
        {
            while (!stop.load())
            {
                dequeueAttempts.fetch_add(1);
                auto item = lockFreeQueue.TryDequeue();
                if (item.has_value())
                    dequeueSuccess.fetch_add(1);
            }
        };

        std::vector<std::thread> threads;
        threads.reserve(8);

        for (int i = 0; i < 4; i++)
            threads.emplace_back(producerFunc);
        
        for (int i = 0; i < 4; i++)
            threads.emplace_back(consumerFunc);

        std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
        stop.store(true);

        for (auto& thread : threads)
            thread.join();

        MESSAGE(std::format("Enqueue: {}/{}, Dequeue: {}/{}", 
                enqueueSuccess.load(), enqueueAttempts.load(),
                dequeueSuccess.load(), dequeueAttempts.load()));

        // Drain the queue and count remaining elements
        uint32_t remaining = 0;
        while (lockFreeQueue.TryDequeue().has_value())
            remaining++;

        MESSAGE(std::format("Remaining in queue: {}", remaining));
        
        // Number of successful enqueues = number of successful dequeues + remaining in queue
        CHECK_EQ(enqueueSuccess.load(), dequeueSuccess.load() + remaining);
        
        // Remaining elements should not exceed queue capacity
        CHECK(remaining <= queueSize);
    }

    TEST_CASE("TOCTOU issue in TryEnqueue - false rejection test")
    {
        // Test if TryEnqueue falsely rejects when queue has space due to race condition
        // between checking queue full and CAS operation
        constexpr uint32_t queueSize = 8;
        constexpr uint32_t iterations = 10000;
        
        std::atomic<uint64_t> falseRejections { 0 };
        std::atomic<uint64_t> totalAttempts { 0 };
        
        for (uint32_t iter = 0; iter < iterations; iter++)
        {
            LockFreeQueue<int, queueSize> lockFreeQueue;
            std::atomic<bool> start { false };
            std::atomic<bool> stopProducers { false };
            std::atomic<int> enqueueSuccess { 0 };
            std::atomic<int> enqueueFailed { 0 };

            // Multiple producers trying to fill the queue
            std::vector<std::thread> producers;
            producers.reserve(4);
            
            for (int i = 0; i < 4; i++)
            {
                producers.emplace_back([&, i]() -> void
                {
                    while (!start.load()) {
                        std::this_thread::yield();
                    }

                    for (int j = 0; j < 20; j++)
                    {
                        if (stopProducers.load())
                            break;
                            
                        totalAttempts.fetch_add(1);
                        if (lockFreeQueue.TryEnqueue(i * 100 + j))
                            enqueueSuccess.fetch_add(1);
                        else
                            enqueueFailed.fetch_add(1);
                    }
                });
            }

            // Aggressive consumer that creates space quickly
            std::atomic<bool> stopConsumer { false };
            std::atomic<int> dequeueCount { 0 };
            std::thread consumer([&]() -> void
            {
                while (!stopConsumer.load())
                {
                    auto item = lockFreeQueue.TryDequeue();
                    if (item.has_value())
                        dequeueCount.fetch_add(1);
                }
            });

            start.store(true);

            for (auto& producer : producers)
                producer.join();

            stopConsumer.store(true);
            consumer.join();

            // If queue had space but TryEnqueue returned false, it's a false rejection
            // We expect: enqueueSuccess + items_in_queue + reasonable_failures ≈ total_attempts
            int remaining = 0;
            while (lockFreeQueue.TryDequeue().has_value())
                remaining++;

            int accountedFor = enqueueSuccess.load();
            int failed = enqueueFailed.load();
            
            // If there were many failures but the queue has space, some might be false rejections
            if (failed > queueSize && remaining < queueSize / 2)
            {
                falseRejections.fetch_add(failed - queueSize);
            }
        }

        MESSAGE(std::format("Total attempts: {}, Estimated false rejections: {}", 
                totalAttempts.load(), falseRejections.load()));
        
        // This test is informational - false rejections are not necessarily a bug,
        // but high rates might indicate the TOCTOU issue
    }

    TEST_CASE("Extreme contention - queue boundary test")
    {
        // Test with extreme contention at queue boundaries
        // to see if we can trigger any capacity violations
        constexpr uint32_t queueSize = 4;  // Small queue for higher contention
        constexpr uint32_t threadCount = 8;
        constexpr uint32_t operationsPerThread = 1000;
        
        LockFreeQueue<int, queueSize> lockFreeQueue;
        std::atomic<bool> start { false };
        std::atomic<uint64_t> totalEnqueued { 0 };
        std::atomic<uint64_t> totalDequeued { 0 };
        std::atomic<uint64_t> enqueueRetries { 0 };
        std::atomic<uint64_t> dequeueRetries { 0 };
        
        std::vector<std::thread> threads;
        threads.reserve(threadCount);

        // Half producers, half consumers
        for (uint32_t i = 0; i < threadCount / 2; i++)
        {
            threads.emplace_back([&]() -> void
            {
                while (!start.load()) {
                    std::this_thread::yield();
                }

                for (uint32_t j = 0; j < operationsPerThread; j++)
                {
                    uint64_t retries = 0;
                    while (!lockFreeQueue.TryEnqueue(42))
                    {
                        retries++;
                        SpinPause();
                    }
                    totalEnqueued.fetch_add(1);
                    if (retries > 0)
                        enqueueRetries.fetch_add(retries);
                }
            });
        }

        for (uint32_t i = 0; i < threadCount / 2; i++)
        {
            threads.emplace_back([&]() -> void
            {
                while (!start.load()) {
                    std::this_thread::yield();
                }

                for (uint32_t j = 0; j < operationsPerThread; j++)
                {
                    uint64_t retries = 0;
                    while (!lockFreeQueue.TryDequeue().has_value())
                    {
                        retries++;
                        SpinPause();
                    }
                    totalDequeued.fetch_add(1);
                    if (retries > 0)
                        dequeueRetries.fetch_add(retries);
                }
            });
        }

        start.store(true);

        for (auto& thread : threads)
            thread.join();

        MESSAGE(std::format("Enqueued: {}, Dequeued: {}", 
                totalEnqueued.load(), totalDequeued.load()));
        MESSAGE(std::format("Enqueue retries: {}, Dequeue retries: {}", 
                enqueueRetries.load(), dequeueRetries.load()));

        // Verify all operations completed successfully
        CHECK_EQ(totalEnqueued.load(), threadCount / 2 * operationsPerThread);
        CHECK_EQ(totalDequeued.load(), threadCount / 2 * operationsPerThread);

        // Queue should be empty
        CHECK(lockFreeQueue.Empty());
    }

    TEST_CASE("TryEnqueue accuracy analysis")
    {
        // Analyze the accuracy of queue full detection in TryEnqueue
        // to see if there are false rejections
        constexpr uint32_t queueSize = 16;
        constexpr uint32_t testIterations = 1000;
        
        uint64_t totalAttempts = 0;
        uint64_t totalSuccesses = 0;
        uint64_t totalFailures = 0;
        uint64_t suspiciousRejections = 0;

        for (uint32_t iter = 0; iter < testIterations; iter++)
        {
            LockFreeQueue<int, queueSize> lockFreeQueue;
            std::atomic<bool> start { false };
            std::atomic<int> successCount { 0 };
            std::atomic<int> failCount { 0 };
            std::atomic<int> queueSizeAtFailure { 0 };

            // Producer thread
            std::thread producer([&]() -> void
            {
                while (!start.load()) {
                    std::this_thread::yield();
                }

                for (int i = 0; i < 50; i++)
                {
                    totalAttempts++;
                    if (lockFreeQueue.TryEnqueue(i))
                    {
                        successCount.fetch_add(1);
                        totalSuccesses++;
                    }
                    else
                    {
                        failCount.fetch_add(1);
                        totalFailures++;
                    }
                }
            });

            // Slow consumer to allow queue to fill
            std::thread consumer([&]() -> void
            {
                while (!start.load()) {
                    std::this_thread::yield();
                }

                std::this_thread::sleep_for(std::chrono::microseconds(100));
                
                for (int i = 0; i < 25; i++)
                {
                    lockFreeQueue.TryDequeue();
                    std::this_thread::sleep_for(std::chrono::microseconds(10));
                }
            });

            start.store(true);
            producer.join();
            consumer.join();

            // Count remaining items
            int remaining = 0;
            while (lockFreeQueue.TryDequeue().has_value())
                remaining++;

            // If we have many failures but queue isn't full, that's suspicious
            int totalInQueue = successCount.load();
            if (failCount.load() > 5 && remaining < queueSize - 2)
            {
                suspiciousRejections++;
            }
        }

        MESSAGE(std::format("Total attempts: {}, Successes: {}, Failures: {}", 
                totalAttempts, totalSuccesses, totalFailures));
        MESSAGE(std::format("Suspicious rejection rate: {}/{} ({:.2f}%)", 
                suspiciousRejections, testIterations, 
                (double)suspiciousRejections / testIterations * 100.0));
        
        double failureRate = (double)totalFailures / totalAttempts * 100.0;
        MESSAGE(std::format("Overall failure rate: {:.2f}%", failureRate));
    }

    TEST_CASE("Queue full detection timing analysis")
    {
        // Test if the queue full check has timing issues
        constexpr uint32_t queueSize = 8;
        LockFreeQueue<int, queueSize> lockFreeQueue;

        // Fill queue completely
        for (uint32_t i = 0; i < queueSize; i++)
        {
            CHECK(lockFreeQueue.TryEnqueue(i));
        }

        // Queue should be full now
        CHECK_FALSE(lockFreeQueue.TryEnqueue(999));

        // Remove one item
        auto item = lockFreeQueue.TryDequeue();
        CHECK(item.has_value());

        // Now there should be space
        CHECK(lockFreeQueue.TryEnqueue(888));

        // Verify queue works correctly
        std::vector<int> values;
        while (auto val = lockFreeQueue.TryDequeue())
        {
            values.push_back(*val);
        }

        CHECK_EQ(values.size(), queueSize);
    }

    TEST_CASE("Concurrent full/empty transitions")
    {
        // Test behavior when queue rapidly transitions between full and empty
        constexpr uint32_t queueSize = 4;
        constexpr uint32_t iterations = 1000;
        
        uint64_t unexpectedFailures = 0;
        
        for (uint32_t iter = 0; iter < iterations; iter++)
        {
            LockFreeQueue<int, queueSize> lockFreeQueue;
            std::atomic<bool> start { false };
            std::atomic<int> producerAttempts { 0 };
            std::atomic<int> producerSuccesses { 0 };

            // Producer: tries to add exactly queueSize items
            std::thread producer([&]() -> void
            {
                while (!start.load()) {
                    std::this_thread::yield();
                }

                int inserted = 0;
                while (inserted < queueSize)
                {
                    producerAttempts.fetch_add(1);
                    if (lockFreeQueue.TryEnqueue(inserted))
                    {
                        producerSuccesses.fetch_add(1);
                        inserted++;
                    }
                }
            });

            // Consumer: immediately starts removing items
            std::thread consumer([&]() -> void
            {
                while (!start.load()) {
                    std::this_thread::yield();
                }

                for (int i = 0; i < queueSize / 2; i++)
                {
                    while (!lockFreeQueue.TryDequeue().has_value())
                    {
                        std::this_thread::yield();
                    }
                }
            });

            start.store(true);
            producer.join();
            consumer.join();

            // Producer should always succeed in inserting queueSize items
            // (even if some are consumed, it should be able to refill)
            int attempts = producerAttempts.load();
            int successes = producerSuccesses.load();
            
            // If attempts are way more than successes, there might be false rejections
            if (attempts > successes * 3)
            {
                unexpectedFailures++;
            }
        }

        MESSAGE(std::format("Iterations with excessive retries: {}/{} ({:.2f}%)", 
                unexpectedFailures, iterations,
                (double)unexpectedFailures / iterations * 100.0));
        
        // Some retries are expected, but excessive retries might indicate issues
        CHECK(unexpectedFailures < iterations / 10);  // Less than 10% should have issues
    }
}