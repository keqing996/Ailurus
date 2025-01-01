#pragma once

#include <functional>
#include <string>
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

        using LogCallBack = std::function<void(Level, const char*, const char*)>;

    public:
        Logger() = delete;

    public:
        static void SetFilterLevel(Level targetLevel);
        static Level GetCurrentFilterLevel();
        static void SetTreadSafe(bool treadSafe);
        static bool GetTreadSafe();
        static void SetLogger(const LogCallBack& pFunc);

        static void LogInfo(const char* message);
        static void LogWarn(const char* message);
        static void LogError(const char* message);
        static void Log(Level level, const char* message);

        static void LogInfo(const char* tag, const char* message);
        static void LogWarn(const char* tag, const char* message);
        static void LogError(const char* tag, const char* message);
        static void Log(Level level, const char* tag, const char* message);

    private:
        static void LogStd(Level level, const char* tag, const char* message);

    private:
        static Level _filterLevel;
        static LogCallBack _logInfoCallVec;
        static bool _isThreadSafe;
        static std::mutex _mutex;
    };
}
