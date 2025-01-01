#include "Ailurus/PlatformDefine.h"

#if PLATFORM_SUPPORT_POSIX

#include "Ailurus/Utility/String.h"

namespace Ailurus
{
    std::string String::WideStringToString(const std::wstring& wStr)
    {
        const wchar_t* wideStr = wStr.c_str();
        if (wideStr == nullptr)
            return std::string{};

        int requiredSize = std::wcstombs(nullptr, wideStr, 0);

        std::vector<char> charBuffer(requiredSize + 1, 0);
        int size = std::wcstombs(charBuffer.data(), wideStr, requiredSize + 1);
        if (size < 0)
            return std::string{};

        return std::string(charBuffer.data());
    }

    std::wstring String::StringToWideString(const std::string& str)
    {
        const char* multiBytesStr = str.c_str();
        if (multiBytesStr == nullptr)
            return std::wstring{};

        int requiredSize = std::mbstowcs(nullptr, multiBytesStr, 0);

        std::vector<wchar_t> wideCharBuffer(requiredSize + 1, 0);
        int size = std::mbstowcs(wideCharBuffer.data(), multiBytesStr, requiredSize + 1);
        if (size < 0)
            return std::wstring{};

        return std::wstring(wideCharBuffer.data());
    }
}

#endif
