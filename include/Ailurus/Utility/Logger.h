#pragma once

#include <functional>
#include <string_view>
#include <mutex>

namespace Ailurus
{
    class Logger
    {
    public:
        enum class Level: int
        {
            Info = 0,
            Warning = 1,
            Error = 2
        };

        using LogCallBack = std::function<void(Level, const std::string_view&, const std::string_view&)>;

    public:
        Logger() = delete;

    public:
        static void SetFilterLevel(Level targetLevel);
        static Level GetCurrentFilterLevel();
        static void SetTreadSafe(bool treadSafe);
        static bool GetTreadSafe();
        static void SetLogger(const LogCallBack& pFunc);

        static void LogInfo(const std::string_view& message);
        static void LogWarn(const std::string_view& message);
        static void LogError(const std::string_view& message);
        static void Log(Level level, const std::string_view& message);

        static void LogInfo(const std::string_view& tag, const std::string_view& message);
        static void LogWarn(const std::string_view& tag, const std::string_view& message);
        static void LogError(const std::string_view& tag, const std::string_view& message);
        static void Log(Level level, const std::string_view& tag, const std::string_view& message);

    private:
        static void LogStd(Level level, const std::string_view& tag, const std::string_view& message);

    private:
        static Level _filterLevel;
        static LogCallBack _logInfoCallVec;
        static bool _isThreadSafe;
        static std::mutex _mutex;
    };
}
