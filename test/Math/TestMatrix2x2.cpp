#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <Ailurus/Math/Matrix2x2.hpp>
#include <Ailurus/Math/Vector2.hpp>

using namespace Ailurus;

TEST_SUITE("Matrix2x2")
{
	TEST_CASE_TEMPLATE("Matrix2x2 constructors", T, int32_t, uint32_t, float, double)
	{
		// Default constructor
		Matrix2x2<T> m1;
		CHECK(m1(0, 0) == T(0));
		CHECK(m1(0, 1) == T(0));
		CHECK(m1(1, 0) == T(0));
		CHECK(m1(1, 1) == T(0));

		// Constructor with Vector2 parameters
		Matrix2x2<T> m2(
			Vector2<T>(T(1), T(2)),
			Vector2<T>(T(3), T(4)));
		CHECK(m2(0, 0) == T(1));
		CHECK(m2(0, 1) == T(2));
		CHECK(m2(1, 0) == T(3));
		CHECK(m2(1, 1) == T(4));

		// Copy constructor
		Matrix2x2<T> m3 = m2;
		CHECK(m3(0, 0) == T(1));
		CHECK(m3(0, 1) == T(2));
		CHECK(m3(1, 0) == T(3));
		CHECK(m3(1, 1) == T(4));

		// Move constructor
		Matrix2x2<T> m4 = std::move(Matrix2x2<T>(
			Vector2<T>(T(5), T(6)),
			Vector2<T>(T(7), T(8))));
		CHECK(m4(0, 0) == T(5));
		CHECK(m4(0, 1) == T(6));
		CHECK(m4(1, 0) == T(7));
		CHECK(m4(1, 1) == T(8));

		// Constructor with initialize list
		Matrix2x2<T> m5 = {
			{ T(1), T(2) },
			{ T(3), T(4) }
		};
		CHECK(m5(0, 0) == T(1));
		CHECK(m5(0, 1) == T(2));
		CHECK(m5(1, 0) == T(3));
		CHECK(m5(1, 1) == T(4));
	}

	TEST_CASE_TEMPLATE("Matrix2x2 element access and modification", T, int32_t, uint32_t, float, double)
	{
		Matrix2x2<T> m1(
			Vector2<T>(T(1), T(2)),
			Vector2<T>(T(3), T(4)));

		// Access elements
		CHECK(m1(0, 0) == T(1));
		CHECK(m1(0, 1) == T(2));
		CHECK(m1(1, 0) == T(3));
		CHECK(m1(1, 1) == T(4));

		// Modify elements
		m1(0, 0) = T(5);
		m1(0, 1) = T(6);
		m1(1, 0) = T(7);
		m1(1, 1) = T(8);

		CHECK(m1(0, 0) == T(5));
		CHECK(m1(0, 1) == T(6));
		CHECK(m1(1, 0) == T(7));
		CHECK(m1(1, 1) == T(8));
	}

	TEST_CASE_TEMPLATE("Matrix2x2 type conversion", T, float, double)
	{
		Matrix2x2<T> mf(
			Vector2<T>(T(1.5), T(2.5)),
			Vector2<T>(T(3.5), T(4.5)));

		Matrix2x2<int> mi = static_cast<Matrix2x2<int>>(mf);
		CHECK(mi(0, 0) == 1);
		CHECK(mi(0, 1) == 2);
		CHECK(mi(1, 0) == 3);
		CHECK(mi(1, 1) == 4);
	}

	TEST_CASE_TEMPLATE("Matrix2x2 determinant", T, int32_t, uint32_t, float, double)
	{
		Matrix2x2<T> m(
			Vector2<T>(T(4), T(3)),
			Vector2<T>(T(2), T(1)));

		// det = 4*1 - 3*2 = 4 - 6 = -2
		CHECK(m.Determinant() == T(-2));
	}

	TEST_CASE_TEMPLATE("Matrix2x2 transpose", T, int32_t, uint32_t, float, double)
	{
		Matrix2x2<T> m(
			Vector2<T>(T(1), T(2)),
			Vector2<T>(T(3), T(4)));

		Matrix2x2<T> transposed = m.Transpose();
		CHECK(transposed(0, 0) == T(1));
		CHECK(transposed(0, 1) == T(3));
		CHECK(transposed(1, 0) == T(2));
		CHECK(transposed(1, 1) == T(4));
	}

	TEST_CASE_TEMPLATE("Matrix2x2 adjugate", T, int32_t, uint32_t, float, double)
	{
		Matrix2x2<T> m(
			Vector2<T>(T(1), T(2)),
			Vector2<T>(T(3), T(4)));

		Matrix2x2<T> adj = m.Adjugate();
		CHECK(adj(0, 0) == T(4));
		CHECK(adj(0, 1) == T(-2));
		CHECK(adj(1, 0) == T(-3));
		CHECK(adj(1, 1) == T(1));
	}

	TEST_CASE_TEMPLATE("Matrix2x2 inverse", T, float, double)
	{
		Matrix2x2<T> m(
			Vector2<T>(T(4), T(7)),
			Vector2<T>(T(2), T(6)));

		// det = 4*6 - 7*2 = 24 - 14 = 10
		Matrix2x2<T> inv = m.Inverse();

		CHECK(inv(0, 0) == doctest::Approx(T(0.6)));
		CHECK(inv(0, 1) == doctest::Approx(T(-0.7)));
		CHECK(inv(1, 0) == doctest::Approx(T(-0.2)));
		CHECK(inv(1, 1) == doctest::Approx(T(0.4)));

		// Test multiplication with inverse = identity
		Matrix2x2<T> identity = m * inv;
		CHECK(identity(0, 0) == doctest::Approx(T(1)));
		CHECK(identity(0, 1) == doctest::Approx(T(0)));
		CHECK(identity(1, 0) == doctest::Approx(T(0)));
		CHECK(identity(1, 1) == doctest::Approx(T(1)));

		// Test inverse of zero determinant matrix
		Matrix2x2<T> singular(
			Vector2<T>(T(1), T(2)),
			Vector2<T>(T(2), T(4)));
		Matrix2x2<T> zeroInv = singular.Inverse();
		CHECK(zeroInv == Matrix2x2<T>::Zero);
	}

	TEST_CASE_TEMPLATE("Matrix2x2 static constants", T, int32_t, uint32_t, float, double)
	{
		// Test Zero matrix
		Matrix2x2<T> zero = Matrix2x2<T>::Zero;
		CHECK(zero(0, 0) == T(0));
		CHECK(zero(0, 1) == T(0));
		CHECK(zero(1, 0) == T(0));
		CHECK(zero(1, 1) == T(0));

		// Test Identity matrix
		Matrix2x2<T> identity = Matrix2x2<T>::Identity;
		CHECK(identity(0, 0) == T(1));
		CHECK(identity(0, 1) == T(0));
		CHECK(identity(1, 0) == T(0));
		CHECK(identity(1, 1) == T(1));
	}

	TEST_CASE_TEMPLATE("Matrix2x2 comparison operators", T, int32_t, uint32_t, float, double)
	{
		Matrix2x2<T> m1(
			Vector2<T>(T(1), T(2)),
			Vector2<T>(T(3), T(4)));

		Matrix2x2<T> m2(
			Vector2<T>(T(1), T(2)),
			Vector2<T>(T(3), T(4)));

		Matrix2x2<T> m3(
			Vector2<T>(T(5), T(6)),
			Vector2<T>(T(7), T(8)));

		CHECK(m1 == m2);
		CHECK(m1 != m3);
		CHECK_FALSE(m1 == m3);
		CHECK_FALSE(m1 != m2);
	}

	TEST_CASE_TEMPLATE("Matrix2x2 matrix multiplication", T, int32_t, uint32_t, float, double)
	{
		Matrix2x2<T> m1(
			Vector2<T>(T(1), T(2)),
			Vector2<T>(T(3), T(4)));

		Matrix2x2<T> m2(
			Vector2<T>(T(5), T(6)),
			Vector2<T>(T(7), T(8)));

		// [1 2] * [5 6] = [19 22]
		// [3 4]   [7 8]   [43 50]
		Matrix2x2<T> result = m1 * m2;
		CHECK(result(0, 0) == T(19)); // 1*5 + 2*7 = 19
		CHECK(result(0, 1) == T(22)); // 1*6 + 2*8 = 22
		CHECK(result(1, 0) == T(43)); // 3*5 + 4*7 = 43
		CHECK(result(1, 1) == T(50)); // 3*6 + 4*8 = 50
	}

	TEST_CASE_TEMPLATE("Matrix2x2 vector multiplication", T, int32_t, uint32_t, float, double)
	{
		Matrix2x2<T> m(
			Vector2<T>(T(1), T(2)),
			Vector2<T>(T(3), T(4)));

		Vector2<T> v(T(5), T(6));

		// [1 2] * [5] = [17]
		// [3 4]   [6]   [39]
		Vector2<T> result = m * v;
		CHECK(result.x == T(17)); // 1*5 + 2*6 = 17
		CHECK(result.y == T(39)); // 3*5 + 4*6 = 39
	}

	TEST_CASE_TEMPLATE("Matrix2x2 scalar operations", T, int32_t, uint32_t, float, double)
	{
		Matrix2x2<T> m(
			Vector2<T>(T(1), T(2)),
			Vector2<T>(T(3), T(4)));

		T scalar = T(2);

		// Matrix * Scalar
		Matrix2x2<T> mMulScalar = m * scalar;
		CHECK(mMulScalar(0, 0) == T(2));
		CHECK(mMulScalar(0, 1) == T(4));
		CHECK(mMulScalar(1, 0) == T(6));
		CHECK(mMulScalar(1, 1) == T(8));

		// Scalar * Matrix
		Matrix2x2<T> scalarMulM = scalar * m;
		CHECK(scalarMulM(0, 0) == T(2));
		CHECK(scalarMulM(0, 1) == T(4));
		CHECK(scalarMulM(1, 0) == T(6));
		CHECK(scalarMulM(1, 1) == T(8));

		// Matrix / Scalar
		Matrix2x2<T> mDivScalar = m / scalar;
		CHECK(mDivScalar(0, 0) == T(1) / T(2));
		CHECK(mDivScalar(0, 1) == T(2) / T(2));
		CHECK(mDivScalar(1, 0) == T(3) / T(2));
		CHECK(mDivScalar(1, 1) == T(4) / T(2));

		// Scalar / Matrix (element-wise)
		Matrix2x2<T> scalarDivM = scalar / m;
		CHECK(scalarDivM(0, 0) == T(2) / T(1));
		CHECK(scalarDivM(0, 1) == T(2) / T(2));
		CHECK(scalarDivM(1, 0) == T(2) / T(3));
		CHECK(scalarDivM(1, 1) == T(2) / T(4));
	}

	TEST_CASE_TEMPLATE("Matrix2x2 compound assignment with scalars", T, int32_t, uint32_t, float, double)
	{
		Matrix2x2<T> m(
			Vector2<T>(T(1), T(2)),
			Vector2<T>(T(3), T(4)));

		T scalar = T(2);

		// *=
		Matrix2x2<T> mMulEq = m;
		mMulEq *= scalar;
		CHECK(mMulEq(0, 0) == T(2));
		CHECK(mMulEq(0, 1) == T(4));
		CHECK(mMulEq(1, 0) == T(6));
		CHECK(mMulEq(1, 1) == T(8));

		// /=
		Matrix2x2<T> mDivEq = m;
		mDivEq /= scalar;
		CHECK(mDivEq(0, 0) == T(1) / T(2));
		CHECK(mDivEq(0, 1) == T(2) / T(2));
		CHECK(mDivEq(1, 0) == T(3) / T(2));
		CHECK(mDivEq(1, 1) == T(4) / T(2));
	}
}