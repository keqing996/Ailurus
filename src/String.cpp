#include "Ailurus/Utility/String.h"

namespace Ailurus
{
    namespace _internal
    {
        template<typename DelimType>
        static std::vector<std::string_view> SplitView(const std::string& inputStr, DelimType delimiter)
        {
            auto split = std::views::split(inputStr, delimiter);

            std::vector<std::string_view> result;
            for (const auto& element : split)
                result.emplace_back(element.begin(), element.end());

            return result;
        }

        template<typename DelimType>
        static std::vector<std::string> Split(const std::string& inputStr, DelimType delimiter)
        {
            auto split = std::views::split(inputStr, delimiter);

            std::vector<std::string> result;
            for (const auto& element : split)
                result.emplace_back(element.begin(), element.end());

            return result;
        }

        template<typename DelimType>
        static std::string Join(const std::vector<std::string>& strVec, DelimType delim)
        {
            std::ostringstream oss;

            for (auto itr = strVec.begin(); itr != strVec.end(); ++itr)
            {
                oss << *itr;
                if (itr != strVec.end() - 1)
                    oss << delim;
            }

            return oss.str();
        }
    }

    std::vector<std::string_view> String::SplitView(const std::string& inputStr, char delimiter)
    {
        return _internal::SplitView(inputStr, delimiter);
    }

    std::vector<std::string_view> String::SplitView(const std::string& inputStr, const std::string& delimiter)
    {
        return _internal::SplitView(inputStr, delimiter);
    }

    std::vector<std::string> String::Split(const std::string& inputStr, char delimiter)
    {
        return _internal::Split(inputStr, delimiter);
    }

    std::vector<std::string> String::Split(const std::string& inputStr, const std::string& delimiter)
    {
        return _internal::Split(inputStr, delimiter);
    }

    std::string String::Join(const std::vector<std::string>& strVec, char delim)
    {
        return _internal::Join(strVec, delim);
    }

    std::string String::Join(const std::vector<std::string>& strVec, const std::string& delim)
    {
        return _internal::Join(strVec, delim);
    }

    void String::Replace(std::string& inStr, const std::string& from, const std::string& to)
    {
        if (from.empty())
            return;

        size_t startPos = 0;
        while ((startPos = inStr.find(from, startPos)) != std::string::npos)
        {
            inStr.replace(startPos, from.length(), to);
            startPos += to.length();
        }
    }

    void String::TrimStart(std::string& str)
    {
        size_t start = 0;
        while (start < str.length() && std::isspace(str[start]))
            ++start;

        str = str.substr(start);
    }

    void String::TrimEnd(std::string& str)
    {
        if (str.empty()) return;
        size_t end = str.length() - 1;
        while (end != std::string::npos && std::isspace(str[end]))
            --end;

        str = str.substr(0, end + 1);
    }

    void String::Trim(std::string& str)
    {
        TrimStart(str);
        TrimEnd(str);
    }
}
