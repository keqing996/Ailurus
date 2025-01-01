#pragma once

#include <chrono>

namespace Ailurus
{
    enum class TimePrecision: int
    {
        Nanoseconds,
        Microseconds,
        Milliseconds,
        Seconds
    };

    template <TimePrecision T = TimePrecision::Milliseconds>
    class Timer
    {
    public:
        void SetNow()
        {
            _timePoint = std::chrono::steady_clock::now();
        }

        int64_t GetIntervalAndSetNow()
        {
            auto interval = GetInterval();
            SetNow();
            return interval;
        }

        int64_t GetInterval() const
        {
            int64_t value;
            if constexpr (T == TimePrecision::Nanoseconds)
                value = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - _timePoint).count();
            else if constexpr (T == TimePrecision::Microseconds)
                value = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - _timePoint).count();
            else if constexpr (T == TimePrecision::Seconds)
                value = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - _timePoint).count();
            else
                value = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - _timePoint).count();

            return value;
        }

    private:
        std::chrono::time_point<std::chrono::steady_clock> _timePoint;
    };
}