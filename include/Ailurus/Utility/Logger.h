#pragma once

#include <format>
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

    public:
        Logger() = delete;

    public:
        static void SetFilterLevel(Level targetLevel);

        static void LogInfo(const char* message);
        static void LogWarn(const char* message);
        static void LogError(const char* message);
        static void Log(Level level, const char* message);
        static void LogInfo(const std::string& message);
        static void LogWarn(const std::string& message);
        static void LogError(const std::string& message);
        static void Log(Level level, const std::string& message);

        template <typename... Args>
        static void LogInfo(const std::string& fmt, Args&&... args)
        {
            std::string formattedMessage = std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...));
            LogInfo(formattedMessage);
        }

        template <typename... Args>
        static void LogWarn(const std::string& fmt, Args&&... args)
        {
            std::string formattedMessage = std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...));
            LogWarn(formattedMessage);
        }

        template <typename... Args>
        static void LogError(const std::string& fmt, Args&&... args)
        {
            std::string formattedMessage = std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...));
            LogError(formattedMessage);
        }
    };
}
