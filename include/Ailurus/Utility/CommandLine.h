#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <functional>
#include <unordered_map>
#include <optional>
#include "../Assert.h"

namespace Ailurus
{
    enum class CmdOptionType
    {
        NoValue,
        SingleValue,
        MultiValue
    };

    class CommandLine
    {
    public:
        CommandLine() = default;
        ~CommandLine();

    public:
        class Option
        {
        public:
            explicit Option(const std::string& desc);
            virtual ~Option() = default;

            const std::string& GetDesc();
            bool Settle() const;
            virtual bool HasValue() = 0;
            virtual CmdOptionType Type() = 0;
            void SetFullName(const std::string& fullName);
            void SetShortName(char shortName);
            const std::optional<std::string>& GetFullName() const;
            const std::optional<char>& GetShortName() const;

        protected:
            std::string _desc;
            bool _settle;
            std::optional<std::string> _fullName;
            std::optional<char> _shortName;
        };

        class OptionNoValue: public Option
        {
        public:
            explicit OptionNoValue(const std::string& desc);

            bool HasValue() override;
            CmdOptionType Type() override;
            bool Set();
        };

        class OptionSingleValue : public Option
        {
        public:
            explicit OptionSingleValue(const std::string& desc);

        public:
            bool HasValue() override;
            CmdOptionType Type() override;
            void SetValue(const std::string& str);

            template<typename T>
            const T& GetValue(const std::function<T(const std::string&)>& converter)
            {
                return converter(_value);
            }

            template<typename T>
            const T& GetValue()
            {
                return Convert<T>(_value);
            }

        private:
            std::string _value;
        };

        class OptionMultiValue : public Option
        {
        public:
            explicit OptionMultiValue(const std::string& desc);

        public:
            bool HasValue() override;
            CmdOptionType Type() override;
            void AddValue(const std::string& str);
            size_t ValueCount() const;
            const std::vector<std::string>& GetValuesStringRaw() const;

            template<typename T>
            T GetValueAt(int index)
            {
                return Convert<T>(_values[index]);
            }

            template<typename T>
            T GetValueAt(int index, const std::function<T(const std::string&)>& converter)
            {
                return converter(_values[index]);
            }

        private:
            std::vector<std::string> _values;
        };

    public:
        void SetExitWhenErrorInput(bool doAbort);
        void SetErrorInputExitCode(int code);
        void SetHelpPrintMessageFunc(const std::function<void()>& func);
        const std::vector<std::string>& GetInvalidInput() const;

        template<CmdOptionType type>
        void AddOption(const std::string& fullName, char shortName, const std::string& desc);

        template<CmdOptionType type>
        void AddOption(const std::string& fullName, const std::string& desc);

        template<CmdOptionType type>
        void AddOption(char shortName, const std::string& desc);

        void Parse(int argc, char** argv);

    private:
        template<typename T>
        static T Convert(const std::string& str);

        template<CmdOptionType type>
        static Option* CreateOption(const std::string& desc);

        void PrintHelpMessage() const;

    private:
        // Error handle
        bool _exitWhenErrorInput = false;
        int _errorInputExitCode = 1;
        std::vector<std::string> _invalidInputRecord;

        // Options
        std::vector<Option*> _allOptions;
        std::unordered_map<std::string, Option*> _fullNameOptionMap;
        std::unordered_map<char, Option*> _shortNameOptionMap;

        // Help message
        std::function<void()> _printHelpMessageFunc;
    };

    template<typename T>
    T CommandLine::Convert(const std::string& str)
    {
        if constexpr (std::is_same_v<T, std::string>)           return str;
        if constexpr (std::is_same_v<T, int>)                   return std::stoi(str);
        if constexpr (std::is_same_v<T, float>)                 return std::stof(str);
        if constexpr (std::is_same_v<T, double>)                return std::stod(str);
        if constexpr (std::is_same_v<T, long>)                  return std::stol(str);
        if constexpr (std::is_same_v<T, unsigned long>)         return std::stoul(str);
        if constexpr (std::is_same_v<T, long long>)             return std::stoll(str);
        if constexpr (std::is_same_v<T, unsigned long long>)    return std::stoull(str);
        if constexpr (std::is_same_v<T, long double>)           return std::stold(str);

        T result;
        std::istringstream strStream(str);
        if (!(strStream >> result && strStream.eof()))
            throw std::bad_cast();

        return result;
    }

    template<CmdOptionType type>
    CommandLine::Option* CommandLine::CreateOption(const std::string& desc)
    {
        Option* pOption = nullptr;

        if constexpr (type == CmdOptionType::NoValue)
            pOption = new OptionNoValue(desc);
        else if constexpr (type == CmdOptionType::SingleValue)
            pOption = new OptionSingleValue(desc);
        else if constexpr (type == CmdOptionType::MultiValue)
            pOption = new OptionMultiValue(desc);

        return pOption;
    }

    template<CmdOptionType type>
    void CommandLine::AddOption(const std::string& fullName, char shortName, const std::string& desc)
    {
        Option* pOption = CreateOption<type>(desc);

        ASSERT_MSG(_fullNameOptionMap.find(fullName) != _fullNameOptionMap.end(), "Command line duplicate full name option");
        ASSERT_MSG(_shortNameOptionMap.find(shortName) != _shortNameOptionMap.end(), "Command line duplicate short name option");

        pOption->SetFullName(fullName);
        pOption->SetShortName(shortName);

        _allOptions.push_back(pOption);
        _fullNameOptionMap[fullName] = pOption;
        _shortNameOptionMap[shortName] = pOption;
    }

    template<CmdOptionType type>
    void CommandLine::AddOption(const std::string& fullName, const std::string& desc)
    {
        Option* pOption = CreateOption<type>(desc);

        ASSERT_MSG(_fullNameOptionMap.find(fullName) != _fullNameOptionMap.end(), "Command line duplicate full name option");

        pOption->SetFullName(fullName);

        _allOptions.push_back(pOption);
        _fullNameOptionMap[fullName] = pOption;
    }

    template<CmdOptionType type>
    void CommandLine::AddOption(char shortName, const std::string& desc)
    {
        Option* pOption = CreateOption<type>(desc);

        ASSERT_MSG(_shortNameOptionMap.find(shortName) != _shortNameOptionMap.end(), "Command line duplicate short name option");

        pOption->SetShortName(shortName);

        _allOptions.push_back(pOption);
        _shortNameOptionMap[shortName] = pOption;
    }
}
