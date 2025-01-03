#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <memory>

namespace Ailurus
{
    class CommandLine
    {
    public:
        struct Result
        {
            std::vector<std::string> values;
        };

    public:
        void SetUserDefinedHelpMessage(const std::string& message);

        void AddOption(const std::string& fullName, char shortName, const std::string& desc);
        void AddOption(const std::string& fullName, const std::string& desc);

        void Parse(int argc, const char** argv);
        const std::vector<std::string>& GetInvalidInput() const;

        const Result* operator[] (const std::string& fullname) const;

    private:
        struct Option
        {
            std::string fullname;
            std::string description;
            std::optional<char> shortName;
            bool hit = false;
            Result result;
        };

        void PrintHelpMessage() const;

    private:
        std::vector<std::string> _invalidInputRecord;

        // Options
        std::vector<std::unique_ptr<Option>> _allOptions;
        std::unordered_map<std::string, Option*> _fullNameOptionMap;
        std::unordered_map<char, Option*> _shortNameOptionMap;

        // Help message
        std::optional<std::string> _userDefinedHelpMessage = std::nullopt;
    };
}
