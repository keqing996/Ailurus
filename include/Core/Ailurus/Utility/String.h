#pragma once

#include <sstream>
#include <string>
#include <vector>
#include <ranges>
#include <algorithm>
#include <optional>

namespace Ailurus
{
    class String
    {
    public:
        String() = delete;

    public:
        /// \brief Convert wide string to utf8 char string. Should set local before calling this.
        static std::string
        WideStringToString(const std::wstring& wStr);

        /// \brief Convert utf8 char string to wide string. Should set local before calling this.
        static std::wstring
        StringToWideString(const std::string& str);

        static std::vector<std::string_view>
        SplitView(const std::string& inputStr, char delimiter);

        static std::vector<std::string_view>
        SplitView(const std::string& inputStr, const std::string& delimiter);

        static std::vector<std::string>
        Split(const std::string& inputStr, char delimiter);

        static std::vector<std::string>
        Split(const std::string& inputStr, const std::string& delimiter);

        static std::string
        Join(const std::vector<std::string>& strVec, char delim);

        static std::string
        Join(const std::vector<std::string>& strVec, const std::string& delim);

        static void
        Replace(std::string& inStr, const std::string& from, const std::string& to);

        static std::string
		Replace(const std::string& inStr, const std::string& from, const std::string& to);

        static void
        TrimStart(std::string& str);

        static void
        TrimEnd(std::string& str);

        static void
        Trim(std::string& str);
    };
}
