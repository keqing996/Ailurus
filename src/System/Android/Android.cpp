
#include "Ailurus/Platform/Android/Android.h"

#if PLATFORM_ANDROID

#include <string>
#include <android/log.h>

namespace Ailurus
{
    Android::LogFunc Android::GetLogFunction()
    {
        return &AndroidLogCat;
    }

    int Android::GetAndroidLogLevel(Android::LogLevel level)
    {
        switch (level)
        {
            case Android::Default:
                return ANDROID_LOG_DEFAULT;
            case Android::Verbose:
                return ANDROID_LOG_VERBOSE;
            case Android::Debug:
                return ANDROID_LOG_DEBUG;
            case Android::Info:
                return ANDROID_LOG_INFO;
            case Android::Warn:
                return ANDROID_LOG_WARN;
            case Android::Error:
                return ANDROID_LOG_ERROR;
            case Android::Fatal:
                return ANDROID_LOG_FATAL;
            case Android::Silent:
                return ANDROID_LOG_SILENT;
        }

        return ANDROID_LOG_DEFAULT;
    }

    void Android::AndroidLogCat(Android::LogLevel level, const char* tag, const char* msg)
    {
        __android_log_print(GetAndroidLogLevel(level), tag, "%s", msg);
    }
}

#endif