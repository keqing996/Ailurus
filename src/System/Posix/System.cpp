#include "Ailurus/PlatformDefine.h"

#if AILURUS_PLATFORM_SUPPORT_POSIX

#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <cerrno>
#include <climits>

#include "Ailurus/System/System.h"

namespace Ailurus
{
    std::string System::GetMachineName()
    {
        char nameBuffer[256] {};

        if (::gethostname(nameBuffer, sizeof(nameBuffer)) == 0)
            return {};

        return nameBuffer;
    }

    std::string System::GetCurrentUserName()
    {
        return GetEnvironmentVariable("USER");
    }

    std::string System::GetEnvironmentVariable(const std::string& keyName)
    {
        const char* variable = ::getenv(keyName.c_str());
        return variable == nullptr ? std::string{} : std::string{ variable };
    }

    void System::SetEnvironmentVariable(const std::string& keyName, const std::string& value)
    {
        if (value.empty())
            ::unsetenv(keyName.c_str());
        else
            ::setenv(keyName.c_str(), value.c_str(), 1);
    }

    std::string System::GetHomeDirectory()
    {
        std::string homeDirectory = GetEnvironmentVariable("HOME");

        return homeDirectory.empty() ? "/" : homeDirectory;
    }

    std::string System::GetCurrentDirectory()
    {
        char buffer[PATH_MAX + 1] {};
        if (::getcwd(buffer, sizeof(buffer)) == nullptr)
            return {};

        return std::string{ buffer };
    }

    bool System::SetCurrentDirectory(const std::string& path)
    {
        return ::chdir(path.c_str()) != -1;
    }

    std::string System::GetExecutableDirectory()
    {
        char buffer[PATH_MAX + 1] {};
        if (::readlink("/proc/self/exe", buffer, sizeof(buffer)) == -1)
            return {};

        return std::string{ buffer };
    }

    std::string System::GetTempDirectory()
    {
        std::string result = GetEnvironmentVariable("TMPDIR");
        return result.empty() ? "/tmp" : result;
    }

    uint32_t System::GetLastError()
    {
        return errno;
    }
}

#endif