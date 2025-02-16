#pragma once

#include <array>
#include <string>
#include <sstream>
#include <stdexcept>
#include "String.h"

namespace Ailurus
{
    namespace _internal
    {
        template<typename E> requires std::is_enum_v<E>
        constexpr int EnumSize() { return 0; }

        template<typename E> requires std::is_enum_v<E>
        std::string GetEnumStringValues() { return ""; }
    }

    template<typename EnumType> requires std::is_enum_v<EnumType>
    class EnumReflection
    {
        EnumReflection() = delete;

        template<std::size_t... Idx>
        static auto GenerateArray(std::index_sequence<Idx...>)
        {
            return std::array<EnumType, Size()>{{static_cast<EnumType>(Idx)...}};
        }

    public:
        constexpr static int Size()
        {
            return _internal::EnumSize<EnumType>();
        }

        static const std::array<std::string, Size()>& GetNameArray()
        {
            static std::array<std::string, Size()> nameArray;
            if (nameArray[0].empty())
            {
                std::string valuesStr = _internal::GetEnumStringValues<EnumType>();
                std::stringstream ss(valuesStr);
                for (auto& value: nameArray)
                {
                    std::getline(ss, value, ',');
                    String::Trim(value);
                }
            }

            return nameArray;
        };

        static const std::array<EnumType, Size()>& GetEnumArray()
        {
            static std::array<EnumType, Size()> values { GenerateArray(std::make_index_sequence<Size()>()) };
            return values;
        };

        constexpr static std::string const& ToString(EnumType arg)
        {
            return GetNameArray()[static_cast<unsigned>(arg)];
        }

        static EnumType FromString(const std::string& string)
        {
            auto const& nameArray = GetNameArray();

            for (unsigned int i = 0; i < nameArray.size(); i++)
            {
                if (string == nameArray[i])
                    return static_cast<EnumType>(i);
            }

            throw std::runtime_error("Error convert string to enum: " + string);
        }
    };

    template<typename EnumType> requires std::is_enum_v<EnumType>
    std::istream& operator>>(std::istream& input, EnumType& arg)
    {
        std::string val;
        input >> val;
        arg = EnumReflection<EnumType>::FromString(val);
        return input;
    }

    template<typename EnumType> requires std::is_enum_v<EnumType>
    std::ostream& operator<<(std::ostream& output, const EnumType& arg)
    {
        return output << EnumReflection<EnumType>::ToString(arg);
    }
}

#define REFLECTION_ENUM(EnumName,...)                           \
                                                                \
enum class EnumName : int                                       \
{                                                               \
    __VA_ARGS__                                                 \
};                                                              \
                                                                \
template<> inline                                               \
constexpr int Ailurus::_internal::EnumSize<EnumName>()          \
{                                                               \
    enum EnumName { __VA_ARGS__ };                              \
    EnumName enumArray[]{ __VA_ARGS__ };                        \
    return sizeof(enumArray) / sizeof(enumArray[0]);            \
}                                                               \
                                                                \
template<> inline                                               \
std::string Ailurus::_internal::GetEnumStringValues<EnumName>() \
{                                                               \
    return #__VA_ARGS__;                                        \
}



