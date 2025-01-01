#include "Ailurus/PlatformDefine.h"

#if PLATFORM_WINDOWS

#include "Ailurus/System/System.h"
#include "Ailurus/Platform/Windows/WindowsDefine.h"
#include "Ailurus/Utility/String.h"
#include <Psapi.h>
#undef GetEnvironmentVariable
#undef SetEnvironmentVariable
#undef GetCurrentDirectory
#undef SetCurrentDirectory

namespace Ailurus
{
    std::string System::GetMachineName()
    {
        const DWORD bufferSize = MAX_COMPUTERNAME_LENGTH + 1;
        wchar_t nameBuffer[bufferSize];

        DWORD count = bufferSize;
        if (!::GetComputerNameW(nameBuffer, &count))
            return {};

        return String::WideStringToString(nameBuffer);
    }

    std::string System::GetCurrentUserName()
    {
        const DWORD bufferSize = 256 + 1;
        wchar_t nameBuffer[bufferSize];

        DWORD count = bufferSize;
        if (!::GetUserNameW(nameBuffer, &count))
            return {};

        return String::WideStringToString(nameBuffer);
    }

    std::string System::GetEnvironmentVariable(const std::string& keyName)
    {
        std::vector<wchar_t> buffer(1024, 0);
        const std::wstring keyNameW = String::StringToWideString(keyName);

        DWORD ret = ::GetEnvironmentVariableW(keyNameW.c_str(), buffer.data(), 1024 - 1);
        if (ret == 0)
            return {};

        if (ret < 1024 - 1)
            return String::WideStringToString(buffer.data());

        buffer.resize(ret + 1);
        ret = ::GetEnvironmentVariableW(keyNameW.c_str(), buffer.data(), ret);

        return String::WideStringToString(buffer.data());
    }

    void System::SetEnvironmentVariable(const std::string& keyName, const std::string& value)
    {
        const std::wstring keyNameW = String::StringToWideString(keyName);

        if (value.empty())
        {
            ::SetEnvironmentVariableW(keyNameW.c_str(), nullptr);
        }
        else
        {
            const std::wstring valueW = String::StringToWideString(value);
            ::SetEnvironmentVariableW(keyNameW.c_str(), valueW.c_str());
        }
    }

    std::string System::GetHomeDirectory()
    {
        std::string result = GetEnvironmentVariable("USERPROFILE");

        if (result.empty())
            result = GetEnvironmentVariable("HOMEDRIVE") + GetEnvironmentVariable("HOMEPATH");

        return result;
    }

    std::string System::GetCurrentDirectory()
    {
        std::wstring result;
        const auto size = ::GetCurrentDirectoryW(0, nullptr);
        result.resize(size);
        ::GetCurrentDirectoryW(size, result.data());
        return String::WideStringToString(result);
    }

    bool System::SetCurrentDirectory(const std::string& path)
    {
        std::wstring pathW = String::StringToWideString(path);
        return ::SetCurrentDirectoryW(pathW.c_str());
    }

    std::string System::GetExecutableDirectory()
    {
        wchar_t buffer[MAX_PATH + 1] = {};
        ::GetModuleFileNameW(nullptr, buffer, MAX_PATH);
        return String::WideStringToString(buffer);
    }

    std::string System::GetTempDirectory()
    {
        wchar_t buffer[MAX_PATH + 1] = {};
        ::GetTempPathW(MAX_PATH, buffer);
        return String::WideStringToString(buffer);
    }

    uint32_t System::GetLastError()
    {
        return ::GetLastError();
    }
}

#endif