#pragma once

#include <doctest/doctest.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <Ailurus/Math/Math.hpp>

using namespace Ailurus;
using namespace Ailurus::Math;

namespace MathTestHelper
{
	template <typename T>
	void CheckQuaternionEqual(const Quaternion<T>& quat1, const Quaternion<T>& quat2)
	{
		CHECK(quat1.w == doctest::Approx(quat2.w));
		CHECK(quat1.x == doctest::Approx(quat2.x));
		CHECK(quat1.y == doctest::Approx(quat2.y));
		CHECK(quat1.z == doctest::Approx(quat2.z));
	}

	template <typename T>
	void CheckQuaternionEqual(const Quaternion<T>& quat1, const glm::quat& quat2)
	{
		CHECK(quat1.w == doctest::Approx(quat2.w));
		CHECK(quat1.x == doctest::Approx(quat2.x));
		CHECK(quat1.y == doctest::Approx(quat2.y));
		CHECK(quat1.z == doctest::Approx(quat2.z));
	}

	template <typename T>
	void CheckVectorEqual(const Vector2<T>& vec1, const Vector2<T>& vec2)
	{
		CHECK(vec1.x == doctest::Approx(vec2.x));
		CHECK(vec1.y == doctest::Approx(vec2.y));
	}

	template <typename T>
	void CheckVectorEqual(const Vector2<T>& vec1, const glm::vec<2, T>& vec2)
	{
		CHECK(vec1.x == doctest::Approx(vec2.x));
		CHECK(vec1.y == doctest::Approx(vec2.y));
	}

	template <typename T>
	void CheckVectorEqual(const Vector3<T>& vec1, const Vector3<T>& vec2)
	{
		CHECK(vec1.x == doctest::Approx(vec2.x));
		CHECK(vec1.y == doctest::Approx(vec2.y));
		CHECK(vec1.z == doctest::Approx(vec2.z));
	}

	template <typename T>
	void CheckVectorEqual(const Vector3<T>& vec1, const glm::vec<3, T>& vec2)
	{
		CHECK(vec1.x == doctest::Approx(vec2.x));
		CHECK(vec1.y == doctest::Approx(vec2.y));
		CHECK(vec1.z == doctest::Approx(vec2.z));
	}

	template <typename T>
	void CheckVectorEqual(const Vector4<T>& vec1, const Vector4<T>& vec2)
	{
		CHECK(vec1.x == doctest::Approx(vec2.x));
		CHECK(vec1.y == doctest::Approx(vec2.y));
		CHECK(vec1.z == doctest::Approx(vec2.z));
		CHECK(vec1.w == doctest::Approx(vec2.w));
	}

	template <typename T>
	void CheckVectorEqual(const Vector4<T>& vec1, const glm::vec<4, T>& vec2)
	{
		CHECK(vec1.x == doctest::Approx(vec2.x));
		CHECK(vec1.y == doctest::Approx(vec2.y));
		CHECK(vec1.z == doctest::Approx(vec2.z));
		CHECK(vec1.w == doctest::Approx(vec2.w));
	}

	template <typename T>
	void CheckMatrixEqual(const Matrix2x2<T>& mat1, const Matrix2x2<T>& mat2)
	{
		for (int i = 0; i < 2; ++i)
		{
			for (int j = 0; j < 2; ++j)
			{
				CHECK(mat1(i, j) == doctest::Approx(mat2(i, j)));
			}
		}
	}

	template <typename T>
	void CheckMatrixEqual(const Matrix2x2<T>& mat1, const glm::mat<2, 2, T>& mat2)
	{
		for (int i = 0; i < 2; ++i)
		{
			for (int j = 0; j < 2; ++j)
			{
				CHECK(mat1(i, j) == doctest::Approx(mat2[j][i]));
			}
		}
	}

	template <typename T>
	void CheckMatrixEqual(const Matrix3x3<T>& mat1, const Matrix3x3<T>& mat2)
	{
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				CHECK(mat1(i, j) == doctest::Approx(mat2(i, j)));
			}
		}
	}

	template <typename T>
	void CheckMatrixEqual(const Matrix3x3<T>& mat1, const glm::mat<3, 3, T>& mat2)
	{
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				CHECK(mat1(i, j) == doctest::Approx(mat2[j][i]));
			}
		}
	}

	template <typename T>
	void CheckMatrixEqual(const Matrix4x4<T>& mat1, const Matrix4x4<T>& mat2)
	{
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				CHECK(mat1(i, j) == doctest::Approx(mat2(i, j)));
			}
		}
	}

	template <typename T>
	void CheckMatrixEqual(const Matrix4x4<T>& mat1, const glm::mat<4, 4, T>& mat2)
	{
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				CHECK(mat1(i, j) == doctest::Approx(mat2[j][i]));
			}
		}
	}
} // namespace MathTestHelper
