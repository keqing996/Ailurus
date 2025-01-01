#pragma once

#include "../../PlatformDefine.h"

#if PLATFORM_ANDROID

namespace Ailurus
{
    class Android
    {
    public:
        enum LogLevel
        {
            Default,
            Verbose,
            Debug,
            Info,
            Warn,
            Error,
            Fatal,
            Silent
        };

        using LogFunc = void(*)(LogLevel, const char*, const char*);

    public:
        Android() = delete;

    public:
        static LogFunc GetLogFunction();

    private:
        static int GetAndroidLogLevel(LogLevel level);

        static void AndroidLogCat(LogLevel level, const char* tag, const char* msg);
    };
}

#endif