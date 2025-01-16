#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include <format>
#include <thread>
#include <Ailurus/Container/LockFreeQueue.hpp>

using namespace Ailurus;

TEST_SUITE("LockFreeQueue")
{
    TEST_CASE("Basic")
    {
        LockFreeQueue<int, 5> lockFreeQueue;

        CHECK_EQ(lockFreeQueue.GetSize(), 8);

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
        LockFreeQueue<int, 100> lockFreeQueue;
        std::vector<int> result;
        result.reserve(1000);
        std::atomic<bool> finish { false };

        size_t enqueueFailCount = 0;
        std::thread producer([&]() -> void
        {
            for (auto i = 0; i < 1000; i++)
            {
                while (true)
                {
                    bool success = lockFreeQueue.TryEnqueue(i);
                    if (success)
                        break;

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
                    dequeueFailCount++;
            }
        });

        producer.join();
        consumer.join();

        CHECK_EQ(result.size(), 1000);
        for (int i = 0; i < result.size(); i++)
            CHECK_EQ(result[i], i);

        MESSAGE(std::format("enqueueFailCount = {}", enqueueFailCount));
        MESSAGE(std::format("dequeueFailCount = {}", dequeueFailCount));
    }

    TEST_CASE("SPSC - block enqueue & block dequeue")
    {
        LockFreeQueue<int, 100> lockFreeQueue;
        std::vector<int> result;
        result.reserve(1000);

        std::thread producer([&]() -> void
        {
            for (auto i = 0; i < 1000; i++)
                lockFreeQueue.Enqueue(i);
        });

        std::thread consumer([&]()->void
        {
            while (result.size() < 1000)
                result.push_back(lockFreeQueue.Dequeue());
        });

        producer.join();
        consumer.join();

        CHECK_EQ(result.size(), 1000);
        for (int i = 0; i < result.size(); i++)
            CHECK_EQ(result[i], i);
    }

    TEST_CASE("MPMC")
    {
        LockFreeQueue<int, 20> lockFreeQueue;

        std::vector<std::thread> producers;
        std::vector<std::thread> consumers;
        std::vector<std::vector<int>> consumerResults;
        std::atomic<bool> finish { false };

        producers.reserve(5);
        consumers.reserve(5);
        consumerResults.resize(5);
        for (auto& consumerResult: consumerResults)
            consumerResult.reserve(1000);

        for (auto i = 0; i < 5; i++)
        {
            producers.emplace_back([&, i]() -> void
            {
                for (auto num = 0 + i * 1000; num < 1000 + i * 1000; num++)
                    lockFreeQueue.Enqueue(num);
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

        std::vector<int> finalResult;
        for (auto& consumerResult: consumerResults)
            finalResult.insert(finalResult.end(), consumerResult.begin(), consumerResult.end());

        std::sort(finalResult.begin(), finalResult.end());

        CHECK_EQ(finalResult.size(), 5000);
        for (int i = 0; i < finalResult.size(); i++)
            CHECK_EQ(finalResult[i], i);
    }
}