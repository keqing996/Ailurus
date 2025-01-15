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
            CHECK_EQ(array.GetSize(), 10);
        }
        {
            SegmentArray<int32_t, 10, 7> array;

            CHECK_EQ(array.GetInternalArraySize(), 14);
            CHECK_EQ(array.GetSize(), 10);
        }
        {
            SegmentArray<int32_t, 12, 4> array;

            CHECK_EQ(array.GetInternalArraySize(), 12);
            CHECK_EQ(array.GetSize(), 12);
        }
        {
            SegmentArray<int32_t, 14, 7> array;

            CHECK_EQ(array.GetInternalArraySize(), 14);
            CHECK_EQ(array.GetSize(), 14);
        }
    }

    TEST_CASE("Index")
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
}