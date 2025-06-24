#include "Ailurus/Utility/String.h"

namespace Ailurus
{
    namespace _internal
    {
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
        std::vector<std::string_view> result;
        std::string_view strView(inputStr);
        size_t start = 0;
        size_t end = 0;

        while ((end = strView.find(delimiter, start)) != std::string_view::npos)
        {
            result.emplace_back(strView.substr(start, end - start));
            start = end + 1;
        }

        if (start < strView.size())
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

        if (start < strView.size())
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

        if (start < inputStr.size())
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

        if (start < inputStr.size())
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
