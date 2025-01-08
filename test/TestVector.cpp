#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "doctest/doctest.h"

#include <Ailurus/Utility/Math/Vector.hpp>

using namespace Ailurus;

TEST_SUITE("Vector")
{
    TEST_CASE_TEMPLATE("Create no para", T, int32_t, uint32_t, float, double)
    {
        Vector<int, 3> vec(1, 2, 3);
        //auto a = vec.y()

        vec.x() = 1;
        vec.y() = 1;
        vec.z() = 1;

        Vector<int, 3> vec2(1, 2, 3);
        auto a = vec + vec2;

        CHECK_EQ(a.x(), 2);
        CHECK_EQ(a.y(), 3);
        CHECK_EQ(a.z(), 4);
    }
}

