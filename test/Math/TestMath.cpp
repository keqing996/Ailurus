#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <Ailurus/Math/Math.hpp>
#include <cmath>

using namespace Ailurus;
using namespace Ailurus::Math;

TEST_SUITE("Math Rotation")
{
	TEST_CASE_TEMPLATE("EulerAngleToQuaternion", T, float, double)
	{
		// Double conversion should return to original
		EulerAngles<T> angles(0.5, 0.3, 0.1);
		Quaternion<T> quat = EulerAngleToQuaternion(angles);
		EulerAngles<T> result = QuaternionToEulerAngles(quat);
		CHECK(std::abs(result.pitch - angles.pitch) < 1e-5);
		CHECK(std::abs(result.yaw - angles.yaw) < 1e-5);
		CHECK(std::abs(result.roll - angles.roll) < 1e-5);
	}

	TEST_CASE_TEMPLATE("QuaternionToEulerAngles", T, float, double)
	{
		// Identity quaternion should return zero angles
		Quaternion<T> quat = Quaternion<T>::Identity();
		EulerAngles<T> angles = QuaternionToEulerAngles(quat);

		CHECK(std::abs(angles.pitch) < 1e-5);
		CHECK(std::abs(angles.yaw) < 1e-5);
		CHECK(std::abs(angles.roll) < 1e-5);
	}

	TEST_CASE_TEMPLATE("QuaternionToMatrix", T, float, double)
	{
		// Identity quaternion should return identity matrix
		Quaternion<T> quat = Quaternion<T>::Identity();
		Matrix<T, 4, 4> mat = QuaternionToMatrix(quat);

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				if (i == j)
					CHECK(std::abs(mat[i][j] - 1) < 1e-5);
				else
					CHECK(std::abs(mat[i][j]) < 1e-5);
			}
		}
	}

	TEST_CASE_TEMPLATE("MatrixToQuaternion", T, float, double)
	{
		// Identity matrix should return identity quaternion
		Matrix<T, 4, 4> mat;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				mat[i][j] = (i == j) ? 1 : 0;
			}
		}

		Quaternion<T> quat = MatrixToQuaternion(mat);

		CHECK(std::abs(quat.x) < 1e-5);
		CHECK(std::abs(quat.y) < 1e-5);
		CHECK(std::abs(quat.z) < 1e-5);
		CHECK(std::abs(quat.w - 1) < 1e-5);
	}

	TEST_CASE_TEMPLATE("EulerAngleToMatrix", T, float, double)
	{
		// Zero euler angles should return identity matrix
		EulerAngles<T> angles(0, 0, 0);
		Matrix<T, 4, 4> mat = EulerAngleToMatrix(angles);

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				if (i == j)
				{
					CHECK(std::abs(mat[i][j] - 1) < 1e-5);
				}
				else
				{
					CHECK(std::abs(mat[i][j]) < 1e-5);
				}
			}
		}
	}

	TEST_CASE_TEMPLATE("MatrixToEulerAngle", T, float, double)
	{
		// Identity matrix should return zero angles
		Matrix<T, 4, 4> mat;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				mat[i][j] = (i == j) ? 1 : 0;
			}
		}

		EulerAngles<T> angles = MatrixToEulerAngle(mat);

		CHECK(std::abs(angles.pitch) < 1e-5);
		CHECK(std::abs(angles.yaw) < 1e-5);
		CHECK(std::abs(angles.roll) < 1e-5);
	}

	TEST_CASE_TEMPLATE("RotateAxis", T, float, double)
	{
		// Rotate 90 degrees around Y axis
		Vector<T, 3> yAxis(0, 1, 0);
		Quaternion<T> quat = RotateAxis(yAxis, std::numbers::pi_v<T> / 2);

		// Create a vector pointing along the X axis
		Vector<T, 3> vec(1, 0, 0);

		// // Apply the rotation, should point along the -Z axis
		Vector<T, 3> rotated = quat * vec;

		CHECK(std::abs(rotated.x()) < 1e-5);
		CHECK(std::abs(rotated.y()) < 1e-5);
		CHECK(std::abs(rotated.z() + 1) < 1e-5);
	}

	TEST_CASE_TEMPLATE("Full rotation conversion cycle", T, float, double)
	{
		// Create an any euler angle
		EulerAngles<T> original(0.1, 0.2, 0.3);

		// Convert to quaternion
		Quaternion<T> quat = EulerAngleToQuaternion(original);

		// Then convert to matrix
		Matrix<T, 4, 4> mat = QuaternionToMatrix(quat);

		// Then convert back to quaternion
		Quaternion<T> quat2 = MatrixToQuaternion(mat);

		// Then convert back to euler angles
		EulerAngles<T> result = QuaternionToEulerAngles(quat2);

		// Check if the result is close to the original
		CHECK(std::abs(result.pitch - original.pitch) < 1e-5);
		CHECK(std::abs(result.yaw - original.yaw) < 1e-5);
		CHECK(std::abs(result.roll - original.roll) < 1e-5);
	}
}