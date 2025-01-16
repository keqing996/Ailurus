#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <Ailurus/Math/Vector.hpp>

using namespace Ailurus;

TEST_SUITE("Vector")
{
    TEST_CASE_TEMPLATE("Default constructor", T, int32_t, uint32_t, float, double)
    {
        {
            Vector<T, 3> vector;
            CHECK_EQ(vector.x(), 0);
            CHECK_EQ(vector.y(), 0);
            CHECK_EQ(vector.z(), 0);
        }

        {
            Vector<T, 5> vector;
            CHECK_EQ(vector[0], 0);
            CHECK_EQ(vector[1], 0);
            CHECK_EQ(vector[2], 0);
            CHECK_EQ(vector[3], 0);
            CHECK_EQ(vector[4], 0);
        }
    }

    TEST_CASE_TEMPLATE("Variant parameters constructor", T, int32_t, uint32_t, float, double)
    {
        {
            Vector<T, 3> vector(1, 2, 3);
            CHECK_EQ(vector.x(), 1);
            CHECK_EQ(vector.y(), 2);
            CHECK_EQ(vector.z(), 3);
        }

        {
            Vector<T, 5> vector(1, 2, 3, 4, 5);
            CHECK_EQ(vector[0], 1);
            CHECK_EQ(vector[1], 2);
            CHECK_EQ(vector[2], 3);
            CHECK_EQ(vector[3], 4);
            CHECK_EQ(vector[4], 5);
        }
    }

    TEST_CASE_TEMPLATE("Copy constructor", T, int32_t, uint32_t, float, double)
    {
        Vector<T, 3> src(1, 2, 3);
        Vector<T, 3> dst(src);
        CHECK_EQ(dst.x(), 1);
        CHECK_EQ(dst.y(), 2);
        CHECK_EQ(dst.z(), 3);
    }

    TEST_CASE_TEMPLATE("Move constructor", T, int32_t, uint32_t, float, double)
    {
        Vector<T, 3> src(1, 2, 3);
        Vector<T, 3> dst(std::move(src));
        CHECK_EQ(dst.x(), 1);
        CHECK_EQ(dst.y(), 2);
        CHECK_EQ(dst.z(), 3);
    }

    TEST_CASE_TEMPLATE("Copy assign operator", T, int32_t, uint32_t, float, double)
    {
        Vector<T, 3> src(1, 2, 3);
        Vector<T, 3> dst;
        dst = src;
        CHECK_EQ(dst.x(), 1);
        CHECK_EQ(dst.y(), 2);
        CHECK_EQ(dst.z(), 3);
    }

    TEST_CASE_TEMPLATE("Move assign operator", T, int32_t, uint32_t, float, double)
    {
        Vector<T, 3> src(1, 2, 3);
        Vector<T, 3> dst;
        dst = std::move(src);
        CHECK_EQ(dst.x(), 1);
        CHECK_EQ(dst.y(), 2);
        CHECK_EQ(dst.z(), 3);
    }

    TEST_CASE("Type cast operator")
    {
        Vector3f src(1.1f, 2.2f, 3.3f);
        Vector3i dst;
        dst = static_cast<Vector3i>(src);
        CHECK_EQ(dst.x(), 1);
        CHECK_EQ(dst.y(), 2);
        CHECK_EQ(dst.z(), 3);
    }

    TEST_CASE("Operator index & data pointer")
    {
        {
            Vector3i vector(1, 2 , 3);
            vector[1] = 10;
            vector[0] = vector[2];
            CHECK_EQ(vector.x(), 3);
            CHECK_EQ(vector.y(), 10);
            CHECK_EQ(vector.z(), 3);
        }
        {
            Vector3i vector(1, 2 , 3);
            vector.GetDataPtr()[1] = 10;
            vector.GetDataPtr()[0] = vector.GetDataPtr()[2];
            CHECK_EQ(vector.x(), 3);
            CHECK_EQ(vector.y(), 10);
            CHECK_EQ(vector.z(), 3);
        }
    }

    TEST_CASE("Magnitude")
    {
        Vector2f vector(3, 4);
        CHECK_EQ(vector.SquareMagnitude(), 25);
        CHECK_EQ(vector.Magnitude(), 5);
    }

    TEST_CASE("Normalize")
    {
        {
            Vector3f vector(3, 4, 5);
            vector.Normalize();

            auto length = static_cast<float>(std::sqrt(3 * 3 + 4 * 4 + 5 * 5));
            CHECK_EQ(vector.x(), 3.0f / length);
            CHECK_EQ(vector.y(), 4.0f / length);
            CHECK_EQ(vector.z(), 5.0f / length);
        }
        {
            Vector3f src(3, 4, 5);
            Vector3f vector = src.Normalized();

            auto length = static_cast<float>(std::sqrt(3 * 3 + 4 * 4 + 5 * 5));
            CHECK_EQ(vector.x(), 3.0f / length);
            CHECK_EQ(vector.y(), 4.0f / length);
            CHECK_EQ(vector.z(), 5.0f / length);
        }
    }

    TEST_CASE("Operator add")
    {
        Vector3f vector1(1, 2, 3);
        Vector3f vector2(4, 5, 6);

        Vector3f result = vector1 + vector2;
        CHECK_EQ(result.x(), 5);
        CHECK_EQ(result.y(), 7);
        CHECK_EQ(result.z(), 9);

        result += Vector3f::One;
        CHECK_EQ(result.x(), 6);
        CHECK_EQ(result.y(), 8);
        CHECK_EQ(result.z(), 10);
    }

    TEST_CASE("Operator sub")
    {
        Vector3f vector1(1, 2, 3);
        Vector3f vector2(4, 5, 6);

        Vector3f result = vector2 - vector1;
        CHECK_EQ(result.x(), 3);
        CHECK_EQ(result.y(), 3);
        CHECK_EQ(result.z(), 3);

        result -= Vector3f::One;
        CHECK_EQ(result.x(), 2);
        CHECK_EQ(result.y(), 2);
        CHECK_EQ(result.z(), 2);
    }

    TEST_CASE("Operator multiply")
    {
        Vector3f vector1(1, 2, 3);
        Vector3f vector2(4, 5, 6);

        {
            Vector3f result = vector1 * 2;
            CHECK_EQ(result.x(), 2);
            CHECK_EQ(result.y(), 4);
            CHECK_EQ(result.z(), 6);
        }
        {
            Vector3f result = 2 * vector1;
            CHECK_EQ(result.x(), 2);
            CHECK_EQ(result.y(), 4);
            CHECK_EQ(result.z(), 6);
        }
        {
            auto result = vector1 * vector2;
            CHECK_EQ(result, 32);
        }
        {
            vector1 *= 2;
            CHECK_EQ(vector1.x(), 2);
            CHECK_EQ(vector1.y(), 4);
            CHECK_EQ(vector1.z(), 6);
        }
    }

    TEST_CASE("Operator cross")
    {
        Vector3f vector1(1, 2, 3);
        Vector3f vector2(4, 5, 6);
        Vector3f result = vector1 ^ vector2;
        CHECK_EQ(result.x(), -3);
        CHECK_EQ(result.y(), 6);
        CHECK_EQ(result.z(), -3);
    }

    TEST_CASE("Operator equal")
    {
        const Vector3f vector1(1, 2, 3);
        const Vector3f vector2(4, 5, 6);
        const Vector3f vector3(4, 5, 6);

        CHECK_EQ(vector2 == vector3, true);
        CHECK_EQ(vector1 != vector2, true);
    }
}

