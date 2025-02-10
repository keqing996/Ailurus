
#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/async_logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/android_sink.h>
#include "Ailurus/Utility/Logger.h"
#include "Ailurus/Utility/ScopeGuard.h"
#include "Ailurus/Platform/Android/Android.h"
#include "Ailurus/Platform/Windows/Console.h"

namespace Ailurus
{
    static std::shared_ptr<spdlog::async_logger> _defaultLogger = nullptr;

    static spdlog::level::level_enum ToSpdLevel(Logger::Level level)
    {
        switch (level) {
            case Logger::Level::Info:
                return spdlog::level::level_enum::info;
            case Logger::Level::Warning:
                return spdlog::level::level_enum::warn;
            case Logger::Level::Error:
                return spdlog::level::level_enum::err;
        }
    }

    static void InitLogger()
    {
        static const char* DefaultLoggerName = "DefaultLogger";

#if AILURUS_PLATFORM_ANDROID
        _defaultLogger = std::make_shared<spdlog::async_logger>(
            DefaultLoggerName, std::make_shared<spdlog::sinks::android_sink_mt>(),
            spdlog::thread_pool(), spdlog::async_overflow_policy::block);
#else
        _defaultLogger = std::make_shared<spdlog::async_logger>(
            DefaultLoggerName, std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
            spdlog::thread_pool(), spdlog::async_overflow_policy::block);
#endif

        _defaultLogger->set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");
        _defaultLogger->flush_on(ToSpdLevel(Logger::Level::Error));
        _defaultLogger->set_level(ToSpdLevel(Logger::Level::Info));

        spdlog::set_default_logger(_defaultLogger);
    }

    static std::shared_ptr<spdlog::async_logger>& GetLogger()
    {
        if (_defaultLogger == nullptr)
            InitLogger();

        return _defaultLogger;
    }

    void Logger::SetFilterLevel(Level targetLevel)
    {
        GetLogger()->set_level(ToSpdLevel(targetLevel));
    }

    void Logger::LogInfo(const std::string_view& message)
    {
        Log(Level::Info, message);
    }

    void Logger::LogWarn(const std::string_view& message)
    {
        Log(Level::Warning, message);
    }

    void Logger::LogError(const std::string_view& message)
    {
        Log(Level::Error, message);
    }

    void Logger::Log(Level level, const std::string_view& message)
    {
        GetLogger()->log(ToSpdLevel(level), message);
    }
}
