#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <Ailurus/Math/Math.hpp>

using namespace Ailurus;
using namespace Ailurus::Math;

TEST_SUITE("Matrix3x3")
{
	TEST_CASE("DegreeToRadian function")
	{
		// Test basic conversions
		CHECK(DegreeToRadian(0.0f) == doctest::Approx(0.0f));
		CHECK(DegreeToRadian(180.0f) == doctest::Approx(3.14159f).epsilon(0.0001f));
		CHECK(DegreeToRadian(360.0f) == doctest::Approx(6.28318f).epsilon(0.0001f));

		// Test negative values
		CHECK(DegreeToRadian(-90.0f) == doctest::Approx(-1.57079f).epsilon(0.0001f));

		// Test arbitrary value
		CHECK(DegreeToRadian(45.0f) == doctest::Approx(0.78539f).epsilon(0.0001f));
	}

	TEST_CASE("RadianToDegree function")
	{
		// Test basic conversions
		CHECK(RadianToDegree(0.0f) == doctest::Approx(0.0f));
		CHECK(RadianToDegree(3.14159f) == doctest::Approx(180.0f).epsilon(0.0001f));
		CHECK(RadianToDegree(6.28318f) == doctest::Approx(360.0f).epsilon(0.0001f));

		// Test negative values
		CHECK(RadianToDegree(-1.57079f) == doctest::Approx(-90.0f).epsilon(0.0001f));

		// Test arbitrary value
		CHECK(RadianToDegree(0.78539f) == doctest::Approx(45.0f).epsilon(0.0001f));
	}

	TEST_CASE("TranslateMatrix function")
	{
		// Test basic translation
		Vector3f translation(1.0f, 2.0f, 3.0f);
		Matrix4x4f matrix = TranslateMatrix(translation);

		// Check identity with translation column
		CHECK(matrix[0][0] == 1.0f);
		CHECK(matrix[1][1] == 1.0f);
		CHECK(matrix[2][2] == 1.0f);
		CHECK(matrix[3][3] == 1.0f);

		// Check translation components
		CHECK(matrix[0][3] == 1.0f);
		CHECK(matrix[1][3] == 2.0f);
		CHECK(matrix[2][3] == 3.0f);

		// Test with vector transformation
		Vector4f point(5.0f, 6.0f, 7.0f, 1.0f);
		Vector4f transformed = matrix * point;

		CHECK(transformed.x == doctest::Approx(6.0f));
		CHECK(transformed.y == doctest::Approx(8.0f));
		CHECK(transformed.z == doctest::Approx(10.0f));
		CHECK(transformed.w == doctest::Approx(1.0f));
	}

	TEST_CASE("ScaleMatrix function")
	{
		// Test basic scaling
		Vector3f scale(2.0f, 3.0f, 4.0f);
		Matrix4x4f matrix = ScaleMatrix(scale);

		// Check diagonal elements
		CHECK(matrix[0][0] == 2.0f);
		CHECK(matrix[1][1] == 3.0f);
		CHECK(matrix[2][2] == 4.0f);
		CHECK(matrix[3][3] == 1.0f);

		// Test with vector transformation
		Vector4f point(1.0f, 1.0f, 1.0f, 1.0f);
		Vector4f transformed = matrix * point;

		CHECK(transformed.x == doctest::Approx(2.0f));
		CHECK(transformed.y == doctest::Approx(3.0f));
		CHECK(transformed.z == doctest::Approx(4.0f));
		CHECK(transformed.w == doctest::Approx(1.0f));
	}

	TEST_CASE("Conversion between Euler angles and Quaternion")
	{
		// Test with some specific angles
		EulerAnglesf angles{ 0.5f, 0.3f, 0.1f };

		// Convert Euler to Quaternion
		Quaternionf quat = EulerAngleToQuaternion(angles);

		// Convert back to Euler
		EulerAnglesf resultAngles = QuaternionToEulerAngles(quat);

		// Check that the round-trip conversion is consistent
		CHECK(resultAngles.roll == doctest::Approx(angles.roll).epsilon(0.001f));
		CHECK(resultAngles.pitch == doctest::Approx(angles.pitch).epsilon(0.001f));
		CHECK(resultAngles.yaw == doctest::Approx(angles.yaw).epsilon(0.001f));
	}

	TEST_CASE("Conversion between Quaternion and Rotation Matrix")
	{
		// Create a quaternion representing a 90-degree rotation around Y axis
		Quaternionf quat = RotateAxis(Vector3f(0.0f, 1.0f, 0.0f), 3.14159f / 2);

		// Convert to rotation matrix
		Matrix4x4f rotMatrix = QuaternionToRotateMatrix(quat);

		// Test the rotation of a vector
		Vector3f vec(1.0f, 0.0f, 0.0f);
		Vector4f vec4(vec.x, vec.y, vec.z, 1.0f);

		Vector4f rotatedVec = rotMatrix * vec4;

		// A 90-degree rotation around Y should turn (1,0,0) to approximately (0,0,-1)
		CHECK(rotatedVec.x == doctest::Approx(0.0f).epsilon(0.001f));
		CHECK(rotatedVec.y == doctest::Approx(0.0f).epsilon(0.001f));
		CHECK(rotatedVec.z == doctest::Approx(-1.0f).epsilon(0.001f));

		// Convert back to quaternion
		Quaternionf resultQuat = RotateMatrixToQuaternion(rotMatrix);

		// Check that the quaternions are the same (considering that q and -q represent the same rotation)
		bool isSameRotation = quat.Dot(resultQuat) > 0.999f || quat.Dot(-1 * resultQuat) > 0.999f;
		CHECK(isSameRotation);
	}

	TEST_CASE("Euler angles to Rotation Matrix and back")
	{
		EulerAnglesf angles{ 0.1f, 0.2f, 0.3f };

		// Convert Euler to rotation matrix
		Matrix4x4f rotMatrix = EulerAngleToRotateMatrix(angles);

		// Convert back to Euler
		EulerAnglesf resultAngles = RotateMatrixToEulerAngle(rotMatrix);

		// Check the round-trip conversion
		CHECK(resultAngles.roll == doctest::Approx(angles.roll).epsilon(0.001f));
		CHECK(resultAngles.pitch == doctest::Approx(angles.pitch).epsilon(0.001f));
		CHECK(resultAngles.yaw == doctest::Approx(angles.yaw).epsilon(0.001f));
	}

	TEST_CASE("RotateAxis function")
	{
		// Test rotation around X axis
		Quaternionf quatX = RotateAxis(Vector3f(1.0f, 0.0f, 0.0f), 3.14159f / 2);
		Vector3f vecY(0.0f, 1.0f, 0.0f);
		Vector3f rotatedY = quatX * vecY;

		CHECK(rotatedY.x == doctest::Approx(0.0f).epsilon(0.001f));
		CHECK(rotatedY.y == doctest::Approx(0.0f).epsilon(0.001f));
		CHECK(rotatedY.z == doctest::Approx(1.0f).epsilon(0.001f));
	}

	TEST_CASE("MakeModelMatrix function")
	{
		Vector3f position(1.0f, 2.0f, 3.0f);
		Quaternionf rotation = RotateAxis(Vector3f(0.0f, 1.0f, 0.0f), 3.14159f / 2);
		Vector3f scale(2.0f, 2.0f, 2.0f);

		Matrix4x4f modelMatrix = MakeModelMatrix(position, rotation, scale);

		// Test the transformation of a point
		Vector4f point(1.0f, 0.0f, 0.0f, 1.0f);
		Vector4f transformed = modelMatrix * point;

		// Expected result: rotated, scaled, then translated
		CHECK(transformed.x == doctest::Approx(1.0f).epsilon(0.001f)); // 1 + 0*2
		CHECK(transformed.y == doctest::Approx(2.0f).epsilon(0.001f)); // 2 + 0*2
		CHECK(transformed.z == doctest::Approx(1.0f).epsilon(0.001f)); // 3 + (-1)*2
	}

	TEST_CASE("MakeViewMatrix function")
	{
		Vector3f cameraPos(-5.0f, 0.0f, 0.0f);
		Quaternionf cameraRot = Quaternionf::Identity;

		Matrix4x4f viewMatrix = MakeViewMatrix(cameraPos, cameraRot);

		// Test transforming a world point to view space
		Vector4f worldPoint(0.0f, 0.0f, 0.0f, 1.0f);
		Vector4f viewPoint = viewMatrix * worldPoint;

		// Camera at (0,0,5) looking down -Z, so the origin should be at (0,0,-5) in view space
		CHECK(viewPoint.x == doctest::Approx(5.0f).epsilon(0.001f));
		CHECK(viewPoint.y == doctest::Approx(0.0f).epsilon(0.001f));
		CHECK(viewPoint.z == doctest::Approx(0.0f).epsilon(0.001f));
	}

	TEST_CASE("Projection Matrices")
	{
		Vector3f cameraPos(-5.0f, 0.0f, 0.0f);
		Quaternionf cameraRot = Quaternionf::Identity;

		Matrix4x4f modelMatrix = Matrix4x4f::Identity;
		Matrix4x4f viewMatrix = MakeViewMatrix(cameraPos, cameraRot);
		Matrix4x4f orthoProjMatrix = MakeOrthoProjectionMatrix<float>(10.0f, 10.0f, 0.1f, 10.0f);
		Matrix4x4f perspProjMatrix = MakePerspectiveProjectionMatrix<float>(10.0f, 10.0f, 0.1f, 10.0f);

		Matrix4x4f orthoMVP = orthoProjMatrix * viewMatrix * modelMatrix;
		Matrix4x4f perspMVP = perspProjMatrix * viewMatrix * modelMatrix;

		Vector4f worldSpacePoint1 = Vector4f::Zero;
		Vector4f orthoClipSpacePoint1 = orthoMVP * worldSpacePoint1;
		Vector4f perspClipSpacePoint1 = perspMVP * worldSpacePoint1;


	}
}