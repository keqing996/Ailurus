
#include <fstream>
#include <filesystem>
#include <sstream>
#include "Ailurus/Utility/ScopeGuard.h"
#include "Ailurus/Utility/String.h"
#include "Ailurus/Utility/File.h"

namespace Ailurus
{
    std::optional<std::vector<char>> File::LoadBinary(const std::string& filePath)
    {
        if (!std::filesystem::exists(filePath))
            return std::nullopt;

        std::ifstream fileStream(filePath, std::ios::binary);
        if (!fileStream.is_open())
            return std::nullopt;

        ScopeGuard fileStreamGuard = [&] { fileStream.close(); };
        {
            fileStream.seekg(0, std::ios::end);
            unsigned int size = fileStream.tellg();
            std::vector<char> content(size);

            fileStream.seekg(0, std::ios::beg);
            fileStream.read(content.data(), size);

            return content;
        }
    }

    std::optional<std::string> File::LoadText(const std::string& filePath)
    {
        if (!std::filesystem::exists(filePath))
            return std::nullopt;

        std::ifstream fileStream(filePath, std::ios::binary);
        if (!fileStream.is_open())
            return std::nullopt;

        ScopeGuard fileStreamGuard = [&] { fileStream.close(); };
        {
            std::ostringstream stringStream;
            stringStream << fileStream.rdbuf();

            return stringStream.str();
        }
    }

    void File::EnsureDirectoryExist(const std::string& pathStr)
    {
        std::filesystem::path path(pathStr);
        std::filesystem::path directory = std::filesystem::is_directory(path) ? path : path.parent_path();

        if (!std::filesystem::exists(directory))
            std::filesystem::create_directory(directory);
    }

    std::string File::GetFileName(const std::string& filePath)
    {
        return std::filesystem::path(filePath).filename().string();
    }

    std::string File::GetFileNameWithoutExtension(const std::string& filePath)
    {
        return std::filesystem::path(filePath).stem().string();
    }

    std::string File::GetFileExtension(const std::string& filePath)
    {
        return std::filesystem::path(filePath).extension().string();
    }

    std::vector<std::string> File::CSV::SplitCsvLine(const std::string& sourceStr)
    {
        auto subStringStripDoubleQuote = [&](size_t posStart, size_t posEnd) -> std::string
        {
            if (sourceStr[posStart] == '"' && sourceStr[posEnd - 1] == '"' && posEnd - posStart > 2)
            {
                posStart++;
                posEnd--;
            }

            return sourceStr.substr(posStart, posEnd - posStart);
        };

        std::vector<std::string> res;
        bool currentInDoubleQuote = false;
        size_t posStart = 0;
        size_t posEnd = posStart;

        while (true)
        {
            if (posEnd == sourceStr.size())
                break;

            if (sourceStr[posEnd] == '"')
                currentInDoubleQuote = !currentInDoubleQuote;

            if (!currentInDoubleQuote && sourceStr[posEnd] == ',')
            {
                res.push_back(subStringStripDoubleQuote(posStart, posEnd));
                posStart = posEnd + 1;
            }

            posEnd++;
        }

        res.push_back(subStringStripDoubleQuote(posStart, posEnd));

        return res;
    }
}
