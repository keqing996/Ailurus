#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <condition_variable>
#include <functional>

namespace Ailurus
{
    class ThreadPool
    {
    public:
        explicit ThreadPool(size_t threadNum)
        {
            for (size_t i = 0; i < threadNum; ++i)
            {
                _threads.emplace_back([this]()->void
                {
                    Worker();
                });
            }
        }

        ~ThreadPool()
        {
            ShutDown();
        }

        void ShutDown()
        {
            {
                std::unique_lock lock(_mutex);

                if (_shutdown)
                    return;

                _shutdown = true;
            }

            _conditionVar.notify_all();

            // Wait all tasks done
            for (std::thread &worker: _threads)
                worker.join();
        }

        bool Enqueue(const std::function<void()>& task)
        {
            {
                std::unique_lock lock(_mutex);

                if (_shutdown)
                    return false;

                _taskQueue.emplace(task);
            }

            _conditionVar.notify_one();
            return true;
        }

    private:
        void Worker()
        {
            while (true)
            {
                std::function<void()> task;

                {
                    std::unique_lock lock(_mutex);

                    if (_shutdown)
                    {
                        if (_taskQueue.empty())
                            return;

                        task = std::move(_taskQueue.front());
                        _taskQueue.pop();
                    }
                    else
                    {
                        if (!_taskQueue.empty())
                        {
                            task = std::move(_taskQueue.front());
                            _taskQueue.pop();
                        }
                        else
                        {
                            _conditionVar.wait(lock,[this]() -> bool
                            {
                                return _shutdown || !_taskQueue.empty();
                            });

                            if (_taskQueue.empty())
                                continue;

                            task = std::move(_taskQueue.front());
                            _taskQueue.pop();
                        }
                    }
                }

                task();
            }
        }

    private:
        std::vector<std::thread> _threads;
        std::queue<std::function<void()>> _taskQueue;

        std::mutex _mutex;
        std::condition_variable _conditionVar;
        bool _shutdown = false;
    };
}
