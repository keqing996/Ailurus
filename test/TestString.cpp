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

            CHECK_EQ(result.size(), 4);
            CHECK_EQ(result[0], "apple");
            CHECK_EQ(result[1], "banana");
            CHECK_EQ(result[2], "orange");
            CHECK_EQ(result[3], "");
        }
        {
            auto result = String::Split(s, ",");

            CHECK_EQ(result.size(), 4);
            CHECK_EQ(result[0], "apple");
            CHECK_EQ(result[1], "banana");
            CHECK_EQ(result[2], "orange");
            CHECK_EQ(result[3], "");
        }
    }

    TEST_CASE("Split edge cases")
    {
        SUBCASE("empty string")
        {
            auto r1 = String::Split("", ',');
            CHECK_EQ(r1.size(), 1);
            CHECK_EQ(r1[0], "");

            auto r2 = String::Split("", ",");
            CHECK_EQ(r2.size(), 1);
            CHECK_EQ(r2[0], "");
        }

        SUBCASE("delimiter only")
        {
            auto r1 = String::Split(",", ',');
            CHECK_EQ(r1.size(), 2);
            CHECK_EQ(r1[0], "");
            CHECK_EQ(r1[1], "");

            auto r2 = String::Split(",", ",");
            CHECK_EQ(r2.size(), 2);
            CHECK_EQ(r2[0], "");
            CHECK_EQ(r2[1], "");
        }

        SUBCASE("consecutive delimiters")
        {
            auto r1 = String::Split("a,,b", ',');
            CHECK_EQ(r1.size(), 3);
            CHECK_EQ(r1[0], "a");
            CHECK_EQ(r1[1], "");
            CHECK_EQ(r1[2], "b");
        }

        SUBCASE("leading delimiter")
        {
            auto r1 = String::Split(",a", ',');
            CHECK_EQ(r1.size(), 2);
            CHECK_EQ(r1[0], "");
            CHECK_EQ(r1[1], "a");
        }

        SUBCASE("no delimiter match")
        {
            auto r1 = String::Split("abc", ',');
            CHECK_EQ(r1.size(), 1);
            CHECK_EQ(r1[0], "abc");
        }

        SUBCASE("empty delimiter string returns original")
        {
            auto r1 = String::Split("abc", "");
            CHECK_EQ(r1.size(), 1);
            CHECK_EQ(r1[0], "abc");
        }

        SUBCASE("multi-char delimiter")
        {
            auto r1 = String::Split("a::b::c::", "::");
            CHECK_EQ(r1.size(), 4);
            CHECK_EQ(r1[0], "a");
            CHECK_EQ(r1[1], "b");
            CHECK_EQ(r1[2], "c");
            CHECK_EQ(r1[3], "");
        }
    }

    TEST_CASE("SplitView")
    {
        std::string s = "apple,banana,orange,";
        {
            auto result = String::SplitView(s, ',');

            CHECK_EQ(result.size(), 4);
            CHECK_EQ(result[0], "apple");
            CHECK_EQ(result[1], "banana");
            CHECK_EQ(result[2], "orange");
            CHECK_EQ(result[3], "");
        }
        {
            auto result = String::SplitView(s, ",");

            CHECK_EQ(result.size(), 4);
            CHECK_EQ(result[0], "apple");
            CHECK_EQ(result[1], "banana");
            CHECK_EQ(result[2], "orange");
            CHECK_EQ(result[3], "");
        }
    }

    TEST_CASE("SplitView edge cases")
    {
        SUBCASE("empty string")
        {
            std::string empty;
            auto r1 = String::SplitView(empty, ',');
            CHECK_EQ(r1.size(), 1);
            CHECK_EQ(r1[0], "");

            auto r2 = String::SplitView(empty, ",");
            CHECK_EQ(r2.size(), 1);
            CHECK_EQ(r2[0], "");
        }

        SUBCASE("delimiter only")
        {
            std::string s = ",";
            auto r1 = String::SplitView(s, ',');
            CHECK_EQ(r1.size(), 2);
            CHECK_EQ(r1[0], "");
            CHECK_EQ(r1[1], "");
        }

        SUBCASE("consecutive delimiters")
        {
            std::string s = "a,,b";
            auto r1 = String::SplitView(s, ',');
            CHECK_EQ(r1.size(), 3);
            CHECK_EQ(r1[0], "a");
            CHECK_EQ(r1[1], "");
            CHECK_EQ(r1[2], "b");
        }

        SUBCASE("leading delimiter")
        {
            std::string s = ",a";
            auto r1 = String::SplitView(s, ',');
            CHECK_EQ(r1.size(), 2);
            CHECK_EQ(r1[0], "");
            CHECK_EQ(r1[1], "a");
        }

        SUBCASE("no delimiter match")
        {
            std::string s = "abc";
            auto r1 = String::SplitView(s, ',');
            CHECK_EQ(r1.size(), 1);
            CHECK_EQ(r1[0], "abc");
        }

        SUBCASE("empty delimiter string returns original")
        {
            std::string s = "abc";
            auto r1 = String::SplitView(s, "");
            CHECK_EQ(r1.size(), 1);
            CHECK_EQ(r1[0], "abc");
        }

        SUBCASE("multi-char delimiter")
        {
            std::string s = "a::b::c::";
            auto r1 = String::SplitView(s, "::");
            CHECK_EQ(r1.size(), 4);
            CHECK_EQ(r1[0], "a");
            CHECK_EQ(r1[1], "b");
            CHECK_EQ(r1[2], "c");
            CHECK_EQ(r1[3], "");
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

    TEST_CASE("Join edge cases")
    {
        SUBCASE("empty vector")
        {
            std::vector<std::string> empty;
            CHECK_EQ(String::Join(empty, ","), "");
            CHECK_EQ(String::Join(empty, ','), "");
        }

        SUBCASE("single element")
        {
            std::vector<std::string> single = { "hello" };
            CHECK_EQ(String::Join(single, ","), "hello");
            CHECK_EQ(String::Join(single, ','), "hello");
        }

        SUBCASE("multi-char delimiter")
        {
            std::vector<std::string> strs = { "a", "b", "c" };
            CHECK_EQ(String::Join(strs, " :: "), "a :: b :: c");
        }
    }

    TEST_CASE("Replace")
    {
        std::string s = "apple,banana,orange,";
        String::Replace(s, ",", "-");
        CHECK_EQ(s, "apple-banana-orange-");
    }

    TEST_CASE("Replace edge cases")
    {
        SUBCASE("const overload returns new string")
        {
            const std::string original = "hello world";
            auto result = String::Replace(original, "world", "earth");
            CHECK_EQ(result, "hello earth");
            CHECK_EQ(original, "hello world");
        }

        SUBCASE("empty from does nothing")
        {
            std::string s = "hello";
            String::Replace(s, "", "x");
            CHECK_EQ(s, "hello");
        }

        SUBCASE("replace to empty string")
        {
            std::string s = "a-b-c";
            String::Replace(s, "-", "");
            CHECK_EQ(s, "abc");
        }

        SUBCASE("no match")
        {
            std::string s = "hello";
            String::Replace(s, "xyz", "abc");
            CHECK_EQ(s, "hello");
        }
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

    TEST_CASE("Trim edge cases")
    {
        SUBCASE("empty string")
        {
            std::string s;
            String::TrimStart(s);
            CHECK_EQ(s, "");

            String::TrimEnd(s);
            CHECK_EQ(s, "");

            String::Trim(s);
            CHECK_EQ(s, "");
        }

        SUBCASE("all whitespace")
        {
            std::string s1 = "   \t\n\r  ";
            String::Trim(s1);
            CHECK_EQ(s1, "");

            std::string s2 = "   \t\n\r  ";
            String::TrimStart(s2);
            CHECK_EQ(s2, "");

            std::string s3 = "   \t\n\r  ";
            String::TrimEnd(s3);
            CHECK_EQ(s3, "");
        }

        SUBCASE("no whitespace")
        {
            std::string s = "hello";
            String::Trim(s);
            CHECK_EQ(s, "hello");
        }

        SUBCASE("string with high bytes does not crash")
        {
            // Bytes >= 0x80 would cause UB with signed char in std::isspace
            std::string s = "  \xC0\xA0test\xF0  ";
            String::TrimStart(s);
            CHECK_EQ(s, "\xC0\xA0test\xF0  ");

            s = "  \xC0\xA0test\xF0  ";
            String::TrimEnd(s);
            CHECK_EQ(s, "  \xC0\xA0test\xF0");

            s = "  \xC0\xA0test\xF0  ";
            String::Trim(s);
            CHECK_EQ(s, "\xC0\xA0test\xF0");
        }
    }
}