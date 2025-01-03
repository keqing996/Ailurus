#include <sstream>
#include <iostream>
#include "Ailurus/Utility/CommandLine.h"
#include "Ailurus/Assert.h"

namespace Ailurus
{
    static std::optional<std::string> GetFullName(const std::string& input)
    {
        if (input.size() > 2 && input[0] == '-' && input[1] == '-')
            return input.substr(2);

        return std::nullopt;
    }

    static std::optional<char> GetShortName(const std::string& input)
    {
        if (input.size() == 2 && input[0] == '-' && std::isalpha(input[1]))
            return input[1];

        return std::nullopt;
    }

    void CommandLine::SetUserDefinedHelpMessage(const std::string& message)
    {
        _userDefinedHelpMessage = message;
    }

    void CommandLine::AddOption(const std::string& fullName, char shortName, const std::string& desc)
    {
        ASSERT_MSG(_fullNameOptionMap.find(fullName) == _fullNameOptionMap.end(), "Command line duplicate full name option");
        ASSERT_MSG(_shortNameOptionMap.find(shortName) == _shortNameOptionMap.end(), "Command line duplicate short name option");

        auto pOption = std::make_unique<Option>();
        pOption->fullname = fullName;
        pOption->shortName = shortName;
        pOption->description = desc;

        _fullNameOptionMap[fullName] = pOption.get();
        _shortNameOptionMap[shortName] = pOption.get();
        _allOptions.push_back(std::move(pOption));
    }

    void CommandLine::AddOption(const std::string& fullName, const std::string& desc)
    {
        ASSERT_MSG(_fullNameOptionMap.find(fullName) == _fullNameOptionMap.end(), "Command line duplicate full name option");

        auto pOption = std::make_unique<Option>();
        pOption->fullname = fullName;
        pOption->shortName = std::nullopt;
        pOption->description = desc;

        _fullNameOptionMap[fullName] = pOption.get();
        _allOptions.push_back(std::move(pOption));
    }

    const std::vector<std::string>& CommandLine::GetInvalidInput() const
    {
        return _invalidInputRecord;
    }

    const CommandLine::Result* CommandLine::operator[](const std::string& fullname) const
    {
        auto itr = _fullNameOptionMap.find(fullname);
        if (itr == _fullNameOptionMap.end())
            return nullptr;

        const Option& option = *itr->second;
        if (!option.hit)
            return nullptr;

        return &option.result;
    }

    void CommandLine::Parse(int argc, const char** argv)
    {
        if (argc == 1)
        {
            PrintHelpMessage();
            std::exit(0);
        }

        if (argc == 2 && (argv[1] == std::string{ "-h" } || argv[1] == std::string{ "-help" } || argv[1] == std::string{ "-?" }))
        {
            PrintHelpMessage();
            std::exit(0);
        }

        _invalidInputRecord.clear();
        int index = 1;

        auto ProcessOption = [&](Option* pOption) -> void
        {
            pOption->hit = true;
            while (index + 1 < argc)
            {
                std::string next(argv[index + 1]);
                if (GetFullName(next) || GetShortName(next))
                    break;

                pOption->result.values.push_back(next);
                index++;
            }
        };

        auto TryProcessFullName = [&](const std::string& str) -> bool
        {
            const auto fullName = GetFullName(str);
            if (!fullName)
                return false;

            const auto& key = *fullName;
            if (const auto itr = _fullNameOptionMap.find(key); itr == _fullNameOptionMap.end())
                return false;
            else
                ProcessOption(itr->second);

            return true;
        };

        auto TryProcessShortName = [&](const std::string& str) -> bool
        {
            const auto fullName = GetShortName(str);
            if (!fullName)
                return false;

            const auto key = *fullName;
            if (const auto itr = _shortNameOptionMap.find(key); itr == _shortNameOptionMap.end())
                return false;
            else
                ProcessOption(itr->second);

            return true;
        };

        for (; index < argc; index++)
        {
            std::string str(argv[index]);

            if (TryProcessFullName(str))
                continue;

            if (TryProcessShortName(str))
                continue;

            _invalidInputRecord.push_back(str);
        }
    }

    void CommandLine::PrintHelpMessage() const
    {
        if (!_userDefinedHelpMessage)
            std::cout << *_userDefinedHelpMessage << std::endl;
        else
        {
            std::stringstream outputStream;
            outputStream << "Options" << std::endl;
            for (const auto& pOption: _allOptions)
            {
                outputStream << "\t" << pOption->fullname;

                if (pOption->shortName.has_value())
                    outputStream << ", " << pOption->shortName.value() << '\n';

                outputStream << "\t\t" << pOption->description << std::endl;
            }

            std::cout << outputStream.str();
        }
    }
}
