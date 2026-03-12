#include "Ailurus/Utility/String.h"

#include <algorithm>
#include <cctype>

namespace Ailurus
{
    namespace _internal
    {
        template<typename DelimType>
        static std::string Join(const std::vector<std::string>& strVec, DelimType delim)
        {
            std::string result;
            for (size_t i = 0; i < strVec.size(); ++i)
            {
                if (i > 0)
                    result += delim;
                result += strVec[i];
            }
            return result;
        }
    }

    std::vector<std::string_view> String::SplitView(const std::string& inputStr, char delimiter)
    {
        std::vector<std::string_view> result;
        std::string_view strView(inputStr);
        size_t start = 0;
        size_t end = 0;

        while ((end = strView.find(delimiter, start)) != std::string_view::npos)
        {
            result.emplace_back(strView.substr(start, end - start));
            start = end + 1;
        }

        if (start <= strView.size())
            result.emplace_back(strView.substr(start));

        return result;
    }

    std::vector<std::string_view> String::SplitView(const std::string& inputStr, const std::string& delimiter)
    {
        if (delimiter.empty())
            return { inputStr };

        std::vector<std::string_view> result;
        std::string_view strView(inputStr);
        size_t start = 0;
        size_t end = 0;
        const size_t delimLen = delimiter.size();

        while ((end = strView.find(delimiter, start)) != std::string_view::npos)
        {
            result.emplace_back(strView.substr(start, end - start));
            start = end + delimLen;
        }

        if (start <= strView.size())
            result.emplace_back(strView.substr(start));

        return result;
    }

    std::vector<std::string> String::Split(const std::string& inputStr, char delimiter)
    {
        std::vector<std::string> result;
        size_t start = 0;
        size_t end = 0;

        while ((end = inputStr.find(delimiter, start)) != std::string::npos)
        {
            result.emplace_back(inputStr.substr(start, end - start));
            start = end + 1;
        }

        if (start <= inputStr.size())
            result.emplace_back(inputStr.substr(start));

        return result;
    }

    std::vector<std::string> String::Split(const std::string& inputStr, const std::string& delimiter)
    {
        if (delimiter.empty())
            return { inputStr };

        std::vector<std::string> result;
        size_t start = 0;
        size_t end = 0;
        const size_t delimLen = delimiter.size();

        while ((end = inputStr.find(delimiter, start)) != std::string::npos)
        {
            result.emplace_back(inputStr.substr(start, end - start));
            start = end + delimLen;
        }

        if (start <= inputStr.size())
            result.emplace_back(inputStr.substr(start));

        return result;
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

    std::string String::Replace(const std::string& inStr, const std::string& from, const std::string& to)
    {
    	std::string result = inStr;
    	Replace(result, from, to);
    	return result;
    }

    void String::TrimStart(std::string& str)
    {
        auto it = std::find_if_not(str.begin(), str.end(),
            [](unsigned char c) { return std::isspace(c); });
        str.erase(str.begin(), it);
    }

    void String::TrimEnd(std::string& str)
    {
        auto it = std::find_if_not(str.rbegin(), str.rend(),
            [](unsigned char c) { return std::isspace(c); });
        str.erase(it.base(), str.end());
    }

    void String::Trim(std::string& str)
    {
        auto isSpace = [](unsigned char c) { return std::isspace(c); };
        auto start = std::find_if_not(str.begin(), str.end(), isSpace);
        auto end = std::find_if_not(str.rbegin(), str.rend(), isSpace).base();
        str = (start < end) ? std::string(start, end) : std::string();
    }
}
