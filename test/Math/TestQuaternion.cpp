#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include <Ailurus/Math/Quaternion.hpp>
#include "MathHelper.hpp"

using namespace Ailurus;

TEST_SUITE("Quaternion")
{
    TEST_CASE_TEMPLATE("Default constructor", T, float, double)
    {
        Quaternion<T> quat;
        CHECK_EQ(quat.x, 0);
        CHECK_EQ(quat.y, 0);
        CHECK_EQ(quat.z, 0);
        CHECK_EQ(quat.w, 1);
    }

    TEST_CASE_TEMPLATE("Parameterized constructor", T, float, double)
    {
        Quaternion<T> quat(1, 2, 3, 4);
        CHECK_EQ(quat.x, 1);
        CHECK_EQ(quat.y, 2);
        CHECK_EQ(quat.z, 3);
        CHECK_EQ(quat.w, 4);
    }

    TEST_CASE_TEMPLATE("Copy constructor", T, float, double)
    {
        Quaternion<T> quat1(1, 2, 3, 4);
        Quaternion<T> quat2(quat1);
        CHECK_EQ(quat2.x, 1);
        CHECK_EQ(quat2.y, 2);
        CHECK_EQ(quat2.z, 3);
        CHECK_EQ(quat2.w, 4);
    }

    TEST_CASE_TEMPLATE("Move constructor", T, float, double)
    {
        Quaternion<T> quat1(1, 2, 3, 4);
        Quaternion<T> quat2(std::move(quat1));
        CHECK_EQ(quat2.x, 1);
        CHECK_EQ(quat2.y, 2);
        CHECK_EQ(quat2.z, 3);
        CHECK_EQ(quat2.w, 4);
    }

    TEST_CASE_TEMPLATE("Copy assignment operator", T, float, double)
    {
        Quaternion<T> quat1(1, 2, 3, 4);
        Quaternion<T> quat2;
        quat2 = quat1;
        CHECK_EQ(quat2.x, 1);
        CHECK_EQ(quat2.y, 2);
        CHECK_EQ(quat2.z, 3);
        CHECK_EQ(quat2.w, 4);
    }

    TEST_CASE_TEMPLATE("Move assignment operator", T, float, double)
    {
        Quaternion<T> quat1(1, 2, 3, 4);
        Quaternion<T> quat2;
        quat2 = std::move(quat1);
        CHECK_EQ(quat2.x, 1);
        CHECK_EQ(quat2.y, 2);
        CHECK_EQ(quat2.z, 3);
        CHECK_EQ(quat2.w, 4);
    }

    TEST_CASE_TEMPLATE("Equality operators", T, float, double)
    {
        Quaternion<T> quat1(1, 2, 3, 4);
        Quaternion<T> quat2(1, 2, 3, 4);
        Quaternion<T> quat3(4, 3, 2, 1);
        CHECK(quat1 == quat2);
        CHECK(quat1 != quat3);
    }

    TEST_CASE_TEMPLATE("Addition operator", T, float, double)
    {
        {
        	Quaternion<T> quat1(1, 2, 3, 4);
        	Quaternion<T> quat2(4, 3, 2, 1);
        	Quaternion<T> result = quat1 + quat2;
        	CHECK_EQ(result.x, 5);
        	CHECK_EQ(result.y, 5);
        	CHECK_EQ(result.z, 5);
        	CHECK_EQ(result.w, 5);
        }
        {
        	Quaternion<T> quat1(1, 2, 3, 4);
        	Quaternion<T> quat2(4, 3, 2, 1);
        	quat1 += quat2;
        	CHECK_EQ(quat1.x, 5);
        	CHECK_EQ(quat1.y, 5);
        	CHECK_EQ(quat1.z, 5);
        	CHECK_EQ(quat1.w, 5);
        }
    }

    TEST_CASE_TEMPLATE("Subtraction operator", T, float, double)
    {
        {
        	Quaternion<T> quat1(1, 2, 3, 4);
        	Quaternion<T> quat2(4, 3, 2, 1);
        	Quaternion<T> result = quat1 - quat2;
        	CHECK_EQ(result.x, -3);
        	CHECK_EQ(result.y, -1);
        	CHECK_EQ(result.z, 1);
        	CHECK_EQ(result.w, 3);
        }
        {
        	Quaternion<T> quat1(1, 2, 3, 4);
        	Quaternion<T> quat2(4, 3, 2, 1);
        	quat1 -= quat2;
        	CHECK_EQ(quat1.x, -3);
        	CHECK_EQ(quat1.y, -1);
        	CHECK_EQ(quat1.z, 1);
        	CHECK_EQ(quat1.w, 3);
        }
    }

    TEST_CASE_TEMPLATE("Multiplication operator", T, float, double)
    {
        Quaternion<T> quat1(1, 2, 3, 4);
        Quaternion<T> quat2(4, 3, 2, 1);
        Quaternion<T> result = quat1 * quat2;
        CHECK_EQ(result.x, 12);
        CHECK_EQ(result.y, 24);
        CHECK_EQ(result.z, 6);
        CHECK_EQ(result.w, -12);
    }

    TEST_CASE_TEMPLATE("Scalar multiplication operator", T, float, double)
    {
        {
        	Quaternion<T> quat(1, 2, 3, 4);
        	T scalar = 2;
        	Quaternion<T> result = quat * scalar;
        	CHECK_EQ(result.x, 2);
        	CHECK_EQ(result.y, 4);
        	CHECK_EQ(result.z, 6);
        	CHECK_EQ(result.w, 8);
        }
        {
        	Quaternion<T> quat(1, 2, 3, 4);
        	T scalar = 2;
        	Quaternion<T> result = scalar * quat;
        	CHECK_EQ(result.x, 2);
        	CHECK_EQ(result.y, 4);
        	CHECK_EQ(result.z, 6);
        	CHECK_EQ(result.w, 8);
        }
	    {
        	Quaternion<T> quat(1, 2, 3, 4);
        	quat *= 2;
        	CHECK_EQ(quat.x, 2);
        	CHECK_EQ(quat.y, 4);
        	CHECK_EQ(quat.z, 6);
        	CHECK_EQ(quat.w, 8);
	    }
    }

    TEST_CASE_TEMPLATE("Dot product", T, float, double)
    {
        Quaternion<T> quat1(1, 2, 3, 4);
        Quaternion<T> quat2(4, 3, 2, 1);
        T result = quat1.Dot(quat2);
        CHECK_EQ(result, 20);
    }

    TEST_CASE_TEMPLATE("Magnitude", T, float, double)
    {
        Quaternion<T> quat(1, 2, 3, 4);
        T result = quat.Magnitude();
        CHECK_EQ(result, doctest::Approx(std::sqrt(30)));
    }

    TEST_CASE_TEMPLATE("Normalization", T, float, double)
    {
        Quaternion<T> quat(1, 2, 3, 4);
        quat.Normalize();
        T mag = std::sqrt(30);
        CHECK_EQ(quat.x, 1 / mag);
        CHECK_EQ(quat.y, 2 / mag);
        CHECK_EQ(quat.z, 3 / mag);
        CHECK_EQ(quat.w, 4 / mag);
    }

    TEST_CASE_TEMPLATE("Conjugate", T, float, double)
    {
        Quaternion<T> quat(1, 2, 3, 4);
        Quaternion<T> result = quat.Conjugate();
        CHECK_EQ(result.x, -1);
        CHECK_EQ(result.y, -2);
        CHECK_EQ(result.z, -3);
        CHECK_EQ(result.w, 4);
    }

    TEST_CASE_TEMPLATE("Inverse", T, float, double)
    {
        Quaternion<T> quat(1, 2, 3, 4);
        Quaternion<T> result = quat.Inverse();
        T norm = 1 / 30.0;
        CHECK_EQ(result.x, doctest::Approx(-1 * norm));
        CHECK_EQ(result.y, doctest::Approx(-2 * norm));
        CHECK_EQ(result.z, doctest::Approx(-3 * norm));
        CHECK_EQ(result.w, doctest::Approx(4 * norm));
    }
}