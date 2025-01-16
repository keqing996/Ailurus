#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "Ailurus/Utility/String.h"

using namespace Ailurus;

TEST_SUITE("String")
{
    TEST_CASE("Split")
    {
        std::string s = "apple,banana,orange,";
        {
            auto result = String::Split(s, ',');

            CHECK_EQ(result.size(), 3);
            CHECK_EQ(result[0], "apple");
            CHECK_EQ(result[1], "banana");
            CHECK_EQ(result[2], "orange");
        }
        {
            auto result = String::Split(s, ",");

            CHECK_EQ(result.size(), 3);
            CHECK_EQ(result[0], "apple");
            CHECK_EQ(result[1], "banana");
            CHECK_EQ(result[2], "orange");
        }
    }

    TEST_CASE("SplitView")
    {
        std::string s = "apple,banana,orange,";
        {
            auto result = String::SplitView(s, ',');

            CHECK_EQ(result.size(), 3);
            CHECK_EQ(result[0], "apple");
            CHECK_EQ(result[1], "banana");
            CHECK_EQ(result[2], "orange");
        }
        {
            auto result = String::SplitView(s, ",");

            CHECK_EQ(result.size(), 3);
            CHECK_EQ(result[0], "apple");
            CHECK_EQ(result[1], "banana");
            CHECK_EQ(result[2], "orange");
        }
    }

    TEST_CASE("Join")
    {
        std::vector<std::string> strs = { "apple", "banana", "orange" };
        {
            const auto result = String::Join(strs, ",");
            CHECK_EQ(result, "apple,banana,orange");
        }
        {
            const auto result = String::Join(strs, ',');
            CHECK_EQ(result, "apple,banana,orange");
        }
    }

    TEST_CASE("Replace")
    {
        std::string s = "apple,banana,orange,";
        String::Replace(s, ",", "-");
        CHECK_EQ(s, "apple-banana-orange-");
    }

    TEST_CASE("Trim")
    {
        std::string testWhiteSpace = "  \t \r \n \r\n  ";
        std::string content = "apple  banana orange";
        {
            std::string s = testWhiteSpace + content + testWhiteSpace;
            String::TrimStart(s);
            CHECK_EQ(s, content + testWhiteSpace);
        }
        {
            std::string s = testWhiteSpace + content + testWhiteSpace;
            String::TrimEnd(s);
            CHECK_EQ(s, testWhiteSpace + content);
        }
        {
            std::string s = testWhiteSpace + content + testWhiteSpace;
            String::Trim(s);
            CHECK_EQ(s, content);
        }
    }
}