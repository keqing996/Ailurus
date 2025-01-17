#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include <format>
#include <numeric>
#include <thread>
#include <random>
#include <Ailurus/Container/RingBufferLockFreeQueue.hpp>
#include <Ailurus/Container/LinkedListLockFreeQueue.hpp>

using namespace Ailurus;

TEST_SUITE("LockFreeQueue")
{
    TEST_CASE("Up alignment")
    {
        {
            RingBufferLockFreeQueue<int, 0> lockFreeQueue;
            CHECK_EQ(lockFreeQueue.GetSize(), 4);
        }
        {
            RingBufferLockFreeQueue<int, 127> lockFreeQueue;
            CHECK_EQ(lockFreeQueue.GetSize(), 128);
        }
        {
            RingBufferLockFreeQueue<int, 128> lockFreeQueue;
            CHECK_EQ(lockFreeQueue.GetSize(), 128);
        }
        {
            RingBufferLockFreeQueue<int, 129> lockFreeQueue;
            CHECK_EQ(lockFreeQueue.GetSize(), 256);
        }
    }

    TEST_CASE("Basic")
    {
        RingBufferLockFreeQueue<int, 5> lockFreeQueue;

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

        RingBufferLockFreeQueue<int, queueSize> lockFreeQueue;
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

        RingBufferLockFreeQueue<int, queueSize> lockFreeQueue;
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
        RingBufferLockFreeQueue<int, 20> lockFreeQueue;

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
}