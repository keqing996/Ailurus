#pragma once

#include <format>
#include <string_view>
#include <string>

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

        static void LogInfo(const std::string_view& message);
        static void LogWarn(const std::string_view& message);
        static void LogError(const std::string_view& message);
        static void Log(Level level, const std::string_view& message);

        template <typename... Args>
        static void LogInfo(const std::string& fmt, Args&&... args)
        {
            LogInfo(std::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        static void LogWarn(const std::string& fmt, Args&&... args)
        {
            LogInfo(std::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        static void LogError(const std::string& fmt, Args&&... args)
        {
            LogInfo(std::format(fmt, std::forward<Args>(args)...));
        }
    };
}
