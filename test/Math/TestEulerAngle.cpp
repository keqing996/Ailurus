#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <Ailurus/Math/EulerAngle.hpp>

using namespace Ailurus;

TEST_SUITE("EulerAngle")
{
	TEST_CASE_TEMPLATE("Default constructor", T, float, double)
	{
		EulerAngles<T> angles;
		CHECK_EQ(angles.pitch, 0);
		CHECK_EQ(angles.yaw, 0);
		CHECK_EQ(angles.roll, 0);
	}

	TEST_CASE_TEMPLATE("Parameterized constructor", T, float, double)
	{
		EulerAngles<T> angles(1, 2, 3);
		CHECK_EQ(angles.pitch, 1);
		CHECK_EQ(angles.yaw, 2);
		CHECK_EQ(angles.roll, 3);
	}

	TEST_CASE_TEMPLATE("Copy constructor", T, float, double)
	{
		EulerAngles<T> angles1(1, 2, 3);
		EulerAngles<T> angles2(angles1);
		CHECK_EQ(angles2.pitch, 1);
		CHECK_EQ(angles2.yaw, 2);
		CHECK_EQ(angles2.roll, 3);
	}

	TEST_CASE_TEMPLATE("Move constructor", T, float, double)
	{
		EulerAngles<T> angles1(1, 2, 3);
		EulerAngles<T> angles2(std::move(angles1));
		CHECK_EQ(angles2.pitch, 1);
		CHECK_EQ(angles2.yaw, 2);
		CHECK_EQ(angles2.roll, 3);
	}

	TEST_CASE_TEMPLATE("Copy assignment operator", T, float, double)
	{
		EulerAngles<T> angles1(1, 2, 3);
		EulerAngles<T> angles2;
		angles2 = angles1;
		CHECK_EQ(angles2.pitch, 1);
		CHECK_EQ(angles2.yaw, 2);
		CHECK_EQ(angles2.roll, 3);
	}

	TEST_CASE_TEMPLATE("Move assignment operator", T, float, double)
	{
		EulerAngles<T> angles1(1, 2, 3);
		EulerAngles<T> angles2;
		angles2 = std::move(angles1);
		CHECK_EQ(angles2.pitch, 1);
		CHECK_EQ(angles2.yaw, 2);
		CHECK_EQ(angles2.roll, 3);
	}

	TEST_CASE_TEMPLATE("Equality operator", T, float, double)
	{
		EulerAngles<T> angles1(1, 2, 3);
		EulerAngles<T> angles2(1, 2, 3);
		CHECK(angles1 == angles2);
	}

	TEST_CASE_TEMPLATE("Inequality operator", T, float, double)
	{
		EulerAngles<T> angles1(1, 2, 3);
		EulerAngles<T> angles2(3, 2, 1);
		CHECK(angles1 != angles2);
	}

	TEST_CASE_TEMPLATE("ToQuaternion", T, float, double)
	{
		EulerAngles<T> angles(1, 2, 3);
		Quaternion<T> quat = angles.ToQuaternion();
		// Add appropriate checks for quaternion values
	}

	TEST_CASE_TEMPLATE("FromQuaternion", T, float, double)
	{
		Quaternion<T> quat(1, 2, 3, 4);
		EulerAngles<T> angles = EulerAngles<T>::FromQuaternion(quat);
		// Add appropriate checks for Euler angles values
	}
}