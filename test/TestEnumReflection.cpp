#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "Ailurus/Utility/EnumReflection.h"

namespace Ailurus
{
    REFLECTION_ENUM(Test, AAA, BBB, CCC);
}

using namespace Ailurus;

TEST_SUITE("Enum Reflection")
{
    TEST_CASE("Size & String")
    {
        auto size = EnumReflection<Test>::Size();
        CHECK_EQ(size, 3);

        auto str = EnumReflection<Test>::ToString(Test::BBB);
        CHECK_EQ(str, "BBB");

        auto value = EnumReflection<Test>::FromString("CCC");
        CHECK_EQ(value, Test::CCC);
    }

    TEST_CASE("Array")
    {
        std::array<std::string, 3> nameArray { "AAA", "BBB", "CCC" };
        auto reflectionNameArray = EnumReflection<Test>::GetNameArray();

        CHECK_EQ(reflectionNameArray.size(), nameArray.size());
        for (auto i = 0; i < nameArray.size(); i++)
            CHECK_EQ(nameArray[i], reflectionNameArray[i]);

        std::array<Test, 3> valueArray { Test::AAA, Test::BBB, Test::CCC };
        auto reflectionValueArray = EnumReflection<Test>::GetEnumArray();

        CHECK_EQ(reflectionValueArray.size(), valueArray.size());
        for (auto i = 0; i < valueArray.size(); i++)
            CHECK_EQ(valueArray[i], reflectionValueArray[i]);
    }

    TEST_CASE("Stream")
    {
        std::stringstream ss;
        ss << Test::BBB;
        CHECK_EQ(ss.str(), "BBB");

        std::string inputData = "CCC";
        std::istringstream inputStream(inputData);
        Test inputEnumValue;
        inputStream >> inputEnumValue;
        CHECK_EQ(inputEnumValue, Test::CCC);
    }
}