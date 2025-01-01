#pragma once

#include <vector>
#include <string>
#include <optional>

namespace Ailurus
{
    class File
    {
    public:
        File() = delete;

    public:
        static std::optional<std::vector<char>> LoadBinary(const std::string& filePath);

        static std::optional<std::string> LoadText(const std::string& filePath);

        static void EnsureDirectoryExist(const std::string& pathStr);

        static std::string GetFileName(const std::string& filePath);

        static std::string GetFileNameWithoutExtension(const std::string& filePath);

        static std::string GetFileExtension(const std::string& filePath);

    public:
        class CSV
        {
        public:
            CSV() = delete;

        public:
            static std::vector<std::string> SplitCsvLine(const std::string& sourceStr);
        };
    };

}