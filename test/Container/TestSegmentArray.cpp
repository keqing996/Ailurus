#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include <Ailurus/Container/SegmentArray.hpp>

using namespace Ailurus;

TEST_SUITE("SegmentArray")
{
    TEST_CASE("Basic")
    {
        {
            SegmentArray<int32_t, 10, 3> array;

            CHECK_EQ(array.GetInternalArraySize(), 12);
            CHECK_EQ(array.size(), 10);
        }
        {
            SegmentArray<int32_t, 10, 7> array;

            CHECK_EQ(array.GetInternalArraySize(), 14);
            CHECK_EQ(array.size(), 10);
        }
        {
            SegmentArray<int32_t, 12, 4> array;

            CHECK_EQ(array.GetInternalArraySize(), 12);
            CHECK_EQ(array.size(), 12);
        }
        {
            SegmentArray<int32_t, 14, 7> array;

            CHECK_EQ(array.GetInternalArraySize(), 14);
            CHECK_EQ(array.size(), 14);
        }
    }

    TEST_CASE("Construct - initialize list")
    {
        SegmentArray<int32_t, 5, 3> array {0, 1, 2, 3, 4};
        for (auto i = 0; i < array.size(); i++)
            CHECK_EQ(i, array[i]);
    }

    TEST_CASE("Copy constructor")
    {
        SegmentArray<int32_t, 5, 3> array {0, 1, 2, 3, 4};
        SegmentArray<int32_t, 5, 3> copy(array);
        for (auto i = 0; i < copy.size(); i++)
            CHECK_EQ(i, copy[i]);
    }

    TEST_CASE("Move constructor")
    {
        SegmentArray<int32_t, 5, 3> array {0, 1, 2, 3, 4};
        SegmentArray<int32_t, 5, 3> copy(std::move(array));
        for (auto i = 0; i < copy.size(); i++)
            CHECK_EQ(i, copy[i]);
    }

    TEST_CASE("Copy assign operator")
    {
        SegmentArray<int32_t, 5, 3> src {0, 1, 2, 3, 4};
        SegmentArray<int32_t, 5, 3> dst;
        dst = src;
        for (auto i = 0; i < dst.size(); i++)
            CHECK_EQ(i, dst[i]);
    }

    TEST_CASE("Move assign operator")
    {
        SegmentArray<int32_t, 5, 3> src {0, 1, 2, 3, 4};
        SegmentArray<int32_t, 5, 3> dst;
        dst = std::move(src);
        for (auto i = 0; i < dst.size(); i++)
            CHECK_EQ(i, dst[i]);
    }

    TEST_CASE("Index")
    {
        {
            SegmentArray<int32_t, 10, 3> array;
            std::fill(array.begin(), array.end(), 0);

            array[0] = 0;
            array[1] = 1;
            array[2] = 2;
            array[3] = 3;
            array[4] = 4;
            array[5] = 5;
            array[6] = 6;
            array[7] = 7;

            CHECK_EQ(array[0], 0);
            CHECK_EQ(array[1], 1);
            CHECK_EQ(array[2], 2);
            CHECK_EQ(array[3], 3);
            CHECK_EQ(array[4], 4);
            CHECK_EQ(array[5], 5);
            CHECK_EQ(array[6], 6);
            CHECK_EQ(array[7], 7);
            CHECK_EQ(array[8], 0);
            CHECK_EQ(array[9], 0);

            auto rawArray = array.GetRawArray();
            CHECK_EQ(rawArray[0],  0);
            CHECK_EQ(rawArray[1],  4);
            CHECK_EQ(rawArray[2],  0);
            CHECK_EQ(rawArray[3],  1);
            CHECK_EQ(rawArray[4],  5);
            CHECK_EQ(rawArray[5],  0);
            CHECK_EQ(rawArray[6],  2);
            CHECK_EQ(rawArray[7],  6);
            CHECK_EQ(rawArray[8],  0);
            CHECK_EQ(rawArray[9],  3);
            CHECK_EQ(rawArray[10], 7);
            CHECK_EQ(rawArray[11], 0);
        }
        {
            SegmentArray<int32_t, 10, 7> array;
            std::fill(array.begin(), array.end(), 0);

            array[0] = 0;
            array[1] = 1;
            array[2] = 2;
            array[3] = 3;
            array[4] = 4;
            array[5] = 5;
            array[6] = 6;
            array[7] = 7;

            CHECK_EQ(array[0], 0);
            CHECK_EQ(array[1], 1);
            CHECK_EQ(array[2], 2);
            CHECK_EQ(array[3], 3);
            CHECK_EQ(array[4], 4);
            CHECK_EQ(array[5], 5);
            CHECK_EQ(array[6], 6);
            CHECK_EQ(array[7], 7);
            CHECK_EQ(array[8], 0);
            CHECK_EQ(array[9], 0);

            auto rawArray = array.GetRawArray();
            CHECK_EQ(rawArray[0],  0);
            CHECK_EQ(rawArray[1],  2);
            CHECK_EQ(rawArray[2],  4);
            CHECK_EQ(rawArray[3],  6);
            CHECK_EQ(rawArray[4],  0);
            CHECK_EQ(rawArray[5],  0);
            CHECK_EQ(rawArray[6],  0);
            CHECK_EQ(rawArray[7],  1);
            CHECK_EQ(rawArray[8],  3);
            CHECK_EQ(rawArray[9],  5);
            CHECK_EQ(rawArray[10], 7);
            CHECK_EQ(rawArray[11], 0);
            CHECK_EQ(rawArray[12], 0);
            CHECK_EQ(rawArray[13], 0);
        }
    }

    TEST_CASE("Itr")
    {
        constexpr uint32_t size = 10;
        SegmentArray<int32_t, size, 9> array;

        for (int i = 0; i < size; i++)
            array[i] = i;

        // const itr
        {
            int index = 0;
            const auto& carray = array;
            for (auto number : carray)
            {
                CHECK_EQ(number, index);
                index++;
            }
        }

        // reverse const itr
        {
            int index = 9;
            const auto& carray = array;
            std::for_each(carray.rbegin(), carray.rend(), [&index](const auto& number)->void
            {
                CHECK_EQ(number, index);
                index--;
            });
        }

        // itr
        {
            for (auto& number : array)
                ++number;

            int index = 0;
            for (auto number : array)
            {
                CHECK_EQ(number, index + 1);
                index++;
            }
        }

        // reverse itr
        {
            std::for_each(array.rbegin(), array.rend(), [](auto& number)->void
            {
                --number;
            });

            int index = 0;
            for (auto number : array)
            {
                CHECK_EQ(number, index);
                index++;
            }
        }
    }
}