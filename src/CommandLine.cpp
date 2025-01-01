#include "Ailurus/Utility/CommandLine.h"

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

    CommandLine::~CommandLine()
    {
        for (auto& pOption: _allOptions)
            delete pOption;
    }

    CommandLine::Option::Option(const std::string& desc)
        : _desc(desc)
        , _settle(false)
    {
    }

    const std::string& CommandLine::Option::GetDesc()
    {
        return _desc;
    }

    bool CommandLine::Option::Settle() const
    {
        return _settle;
    }

    void CommandLine::Option::SetFullName(const std::string& fullName)
    {
        _fullName = fullName;
    }

    void CommandLine::Option::SetShortName(char shortName)
    {
        _shortName = shortName;
    }

    const std::optional<std::string>& CommandLine::Option::GetFullName() const
    {
        return _fullName;
    }

    const std::optional<char>& CommandLine::Option::GetShortName() const
    {
        return _shortName;
    }

    CommandLine::OptionNoValue::OptionNoValue(const std::string& desc): Option(desc)
    {
    }

    bool CommandLine::OptionNoValue::HasValue()
    {
        return false;
    }

    CmdOptionType CommandLine::OptionNoValue::Type()
    {
        return CmdOptionType::NoValue;
    }

    bool CommandLine::OptionNoValue::Set()
    {
        _settle = true;
        return true;
    }

    CommandLine::OptionSingleValue::OptionSingleValue(const std::string& desc): Option(desc)
    {
    }

    bool CommandLine::OptionSingleValue::HasValue()
    {
        return true;
    }

    CmdOptionType CommandLine::OptionSingleValue::Type()
    {
        return CmdOptionType::SingleValue;
    }

    void CommandLine::OptionSingleValue::SetValue(const std::string& str)
    {
        _value = str;
        _settle = true;
    }

    CommandLine::OptionMultiValue::OptionMultiValue(const std::string& desc): Option(desc)
    {
    }

    bool CommandLine::OptionMultiValue::HasValue()
    {
        return true;
    }

    CmdOptionType CommandLine::OptionMultiValue::Type()
    {
        return CmdOptionType::MultiValue;
    }

    void CommandLine::OptionMultiValue::AddValue(const std::string& str)
    {
        _values.push_back(str);
        _settle = true;
    }

    size_t CommandLine::OptionMultiValue::ValueCount() const
    {
        return _values.size();
    }

    const std::vector<std::string>& CommandLine::OptionMultiValue::GetValuesStringRaw() const
    {
        return _values;
    }

    void CommandLine::SetExitWhenErrorInput(bool doAbort)
    {
        _exitWhenErrorInput = doAbort;
    }

    void CommandLine::SetErrorInputExitCode(int code)
    {
        _errorInputExitCode = code;
    }

    void CommandLine::SetHelpPrintMessageFunc(const std::function<void()>& func)
    {
        _printHelpMessageFunc = func;
    }

    const std::vector<std::string>& CommandLine::GetInvalidInput() const
    {
        return _invalidInputRecord;
    }

    void CommandLine::Parse(int argc, char** argv)
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
            switch (pOption->Type())
            {
                case CmdOptionType::NoValue:
                {
                    auto* pNoValueOption = dynamic_cast<OptionNoValue*>(pOption);
                    pNoValueOption->Set();
                    break;
                }
                case CmdOptionType::SingleValue:
                {
                    auto* pSingleValueOption = dynamic_cast<OptionSingleValue*>(pOption);
                    if (index + 1 < argc)
                    {
                        pSingleValueOption->SetValue(argv[index + 1]);
                        index++;
                    }
                    break;
                }
                case CmdOptionType::MultiValue:
                {
                    auto* pMultiValueOption = dynamic_cast<OptionMultiValue*>(pOption);
                    while (index + 1 < argc)
                    {
                        std::string next(argv[index + 1]);
                        if (GetFullName(next) || GetShortName(next))
                            break;

                        pMultiValueOption->AddValue(next);
                        index++;
                    }
                    break;
                }
            }
        };

        auto TryProcessFullName = [&](const std::string& str) -> bool
        {
            const auto fullName = GetFullName(str);
            if (!fullName)
                return false;

            const auto& key = *fullName;
            if (const auto itr = _fullNameOptionMap.find(key); itr == _fullNameOptionMap.end())
            {
                if (_exitWhenErrorInput)
                {
                    std::cout << "Invalid option: " << key << std::endl;
                    std::exit(_errorInputExitCode);
                }

                _invalidInputRecord.push_back(str);
            }
            else
            {
                ProcessOption(itr->second);
            }

            return true;
        };

        auto TryProcessShortName = [&](const std::string& str) -> bool
        {
            const auto fullName = GetShortName(str);
            if (!fullName)
                return false;

            const auto key = *fullName;
            if (const auto itr = _shortNameOptionMap.find(key); itr == _shortNameOptionMap.end())
            {
                if (_exitWhenErrorInput)
                {
                    std::cout << "Invalid option: " << key << std::endl;
                    std::exit(_errorInputExitCode);
                }

                _invalidInputRecord.push_back(str);
            }
            else
            {
                ProcessOption(itr->second);
            }

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
        if (!_printHelpMessageFunc)
            _printHelpMessageFunc();
        else
        {
            std::stringstream outputStream;
            outputStream << "Options" << std::endl;
            for (const auto pOption: _allOptions)
            {
                auto fullName = pOption->GetFullName();
                auto shortName = pOption->GetShortName();

                outputStream << "\t";

                if (fullName && shortName)
                    outputStream << fullName.value() << ", " << shortName.value();
                else
                {
                    if (fullName)
                        outputStream << fullName.value();
                    if (shortName)
                        outputStream << shortName.value();
                }

                outputStream << "\t\t" << pOption->GetDesc() << std::endl;
            }

            std::cout << outputStream.str();
        }
    }
}
