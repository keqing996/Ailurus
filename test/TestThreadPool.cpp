#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "Ailurus/Utility/ThreadPool.hpp"
#include <atomic>
#include <future>
#include <unordered_set>

using namespace Ailurus;

TEST_SUITE("ThreadPool")
{
    TEST_CASE("Single task is executed")
    {
        ThreadPool pool(1);
        std::atomic<bool> executed = false;
        std::promise<void> done;
        auto future = done.get_future();

        bool enqueued = pool.Enqueue([&]() -> void
        {
            executed = true;
            done.set_value();
        });

        CHECK(enqueued);
        future.get();
        CHECK(executed);
    }

    TEST_CASE("Multiple tasks are executed")
    {
        constexpr int taskNum = 100;
        ThreadPool pool(4);
        std::atomic<int> counter = 0;

        for (int i = 0; i < taskNum; ++i)
            pool.Enqueue([&counter]() { ++counter; });

        pool.ShutDown();

        CHECK(counter == taskNum);
    }

    TEST_CASE("Enqueue returns false after shutdown")
    {
        ThreadPool pool(1);
        pool.ShutDown();
        bool result = pool.Enqueue([]() ->void { });
        CHECK_FALSE(result);
    }

    TEST_CASE("Tasks in queue are processed after shutdown")
    {
        constexpr int taskNum = 10;
        std::atomic<int> counter = 0;

        ThreadPool pool(1);

        for (auto i = 0; i < taskNum; i++)
        {
            pool.Enqueue([&counter]()->void
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                ++counter;
            });
        }

        CHECK_EQ(counter, 0);
        pool.ShutDown();
        CHECK_EQ(counter, taskNum);
    }

    TEST_CASE("Destructor processes all tasks")
    {
        std::atomic<int> counter = 0;
        constexpr int taskNum = 100;
        {
            ThreadPool pool(4);
            for (int i = 0; i < taskNum; ++i)
                pool.Enqueue([&counter]() -> void { ++counter; });
        }

        CHECK_EQ(counter, taskNum);
    }

    TEST_CASE("Tasks are executed by multiple threads")
    {
        constexpr size_t threadNum = 4;
        ThreadPool pool(threadNum);

        std::mutex mtx;
        std::unordered_set<std::thread::id> threadIds;

        constexpr int taskNums = 100;
        for (int i = 0; i < taskNums; ++i)
        {
            pool.Enqueue([&]() -> void
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                std::lock_guard<std::mutex> lock(mtx);
                threadIds.insert(std::this_thread::get_id());
            });
        }

        pool.ShutDown();

        CHECK_EQ(threadIds.size(), threadNum);
    }
}
