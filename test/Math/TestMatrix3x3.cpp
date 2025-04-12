#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <Ailurus/Math/Matrix3x3.hpp>
#include <Ailurus/Math/Vector3.hpp>

using namespace Ailurus;

TEST_SUITE("Matrix3x3")
{
	TEST_CASE_TEMPLATE("Matrix3x3 constructors", T, int32_t, uint32_t, float, double)
	{
		// Default constructor
		Matrix3x3<T> m1;
		CHECK(m1(0, 0) == T(0));
		CHECK(m1(0, 1) == T(0));
		CHECK(m1(0, 2) == T(0));
		CHECK(m1(1, 0) == T(0));
		CHECK(m1(1, 1) == T(0));
		CHECK(m1(1, 2) == T(0));
		CHECK(m1(2, 0) == T(0));
		CHECK(m1(2, 1) == T(0));
		CHECK(m1(2, 2) == T(0));

		// Constructor with Vector3 parameters
		Matrix3x3<T> m2(
			Vector3<T>(T(1), T(2), T(3)),
			Vector3<T>(T(4), T(5), T(6)),
			Vector3<T>(T(7), T(8), T(9)));
		CHECK(m2(0, 0) == T(1));
		CHECK(m2(0, 1) == T(2));
		CHECK(m2(0, 2) == T(3));
		CHECK(m2(1, 0) == T(4));
		CHECK(m2(1, 1) == T(5));
		CHECK(m2(1, 2) == T(6));
		CHECK(m2(2, 0) == T(7));
		CHECK(m2(2, 1) == T(8));
		CHECK(m2(2, 2) == T(9));

		// Copy constructor
		Matrix3x3<T> m3 = m2;
		CHECK(m3(0, 0) == T(1));
		CHECK(m3(0, 1) == T(2));
		CHECK(m3(0, 2) == T(3));
		CHECK(m3(1, 0) == T(4));
		CHECK(m3(1, 1) == T(5));
		CHECK(m3(1, 2) == T(6));
		CHECK(m3(2, 0) == T(7));
		CHECK(m3(2, 1) == T(8));
		CHECK(m3(2, 2) == T(9));

		// Move constructor
		Matrix3x3<T> m4 = std::move(Matrix3x3<T>(
			Vector3<T>(T(9), T(8), T(7)),
			Vector3<T>(T(6), T(5), T(4)),
			Vector3<T>(T(3), T(2), T(1))));
		CHECK(m4(0, 0) == T(9));
		CHECK(m4(0, 1) == T(8));
		CHECK(m4(0, 2) == T(7));
		CHECK(m4(1, 0) == T(6));
		CHECK(m4(1, 1) == T(5));
		CHECK(m4(1, 2) == T(4));
		CHECK(m4(2, 0) == T(3));
		CHECK(m4(2, 1) == T(2));
		CHECK(m4(2, 2) == T(1));

		// Constructor with initialize list
		Matrix3x3<T> m5 = {
			{ T(1), T(2), T(3) },
			{ T(4), T(5), T(6) },
			{ T(7), T(8), T(9) }
		};
		CHECK(m5(0, 0) == T(1));
		CHECK(m5(0, 1) == T(2));
		CHECK(m5(0, 2) == T(3));
		CHECK(m5(1, 0) == T(4));
		CHECK(m5(1, 1) == T(5));
		CHECK(m5(1, 2) == T(6));
		CHECK(m5(2, 0) == T(7));
		CHECK(m5(2, 1) == T(8));
		CHECK(m5(2, 2) == T(9));
	}

	TEST_CASE_TEMPLATE("Matrix3x3 element access and modification", T, int32_t, uint32_t, float, double)
	{
		Matrix3x3<T> m1(
			Vector3<T>(T(1), T(2), T(3)),
			Vector3<T>(T(4), T(5), T(6)),
			Vector3<T>(T(7), T(8), T(9)));

		// Access elements
		CHECK(m1(0, 0) == T(1));
		CHECK(m1(0, 1) == T(2));
		CHECK(m1(0, 2) == T(3));
		CHECK(m1(1, 0) == T(4));
		CHECK(m1(1, 1) == T(5));
		CHECK(m1(1, 2) == T(6));
		CHECK(m1(2, 0) == T(7));
		CHECK(m1(2, 1) == T(8));
		CHECK(m1(2, 2) == T(9));

		// Modify elements
		m1(0, 0) = T(9);
		m1(0, 1) = T(8);
		m1(0, 2) = T(7);
		m1(1, 0) = T(6);
		m1(1, 1) = T(5);
		m1(1, 2) = T(4);
		m1(2, 0) = T(3);
		m1(2, 1) = T(2);
		m1(2, 2) = T(1);

		CHECK(m1(0, 0) == T(9));
		CHECK(m1(0, 1) == T(8));
		CHECK(m1(0, 2) == T(7));
		CHECK(m1(1, 0) == T(6));
		CHECK(m1(1, 1) == T(5));
		CHECK(m1(1, 2) == T(4));
		CHECK(m1(2, 0) == T(3));
		CHECK(m1(2, 1) == T(2));
		CHECK(m1(2, 2) == T(1));
	}

	TEST_CASE_TEMPLATE("Matrix3x3 get/set row and column", T, int32_t, uint32_t, float, double)
	{
		Matrix3x3<T> m(
			Vector3<T>(T(1), T(2), T(3)),
			Vector3<T>(T(4), T(5), T(6)),
			Vector3<T>(T(7), T(8), T(9)));

		// Get row
		Vector3<T> row0 = m.GetRow(0);
		CHECK(row0.x == T(1));
		CHECK(row0.y == T(2));
		CHECK(row0.z == T(3));

		// Get column
		Vector3<T> col0 = m.GetCol(0);
		CHECK(col0.x == T(1));
		CHECK(col0.y == T(4));
		CHECK(col0.z == T(7));

		// Set row
		m.SetRow(0, Vector3<T>(T(10), T(11), T(12)));
		CHECK(m(0, 0) == T(10));
		CHECK(m(0, 1) == T(11));
		CHECK(m(0, 2) == T(12));

		// Set column
		m.SetCol(0, Vector3<T>(T(13), T(14), T(15)));
		CHECK(m(0, 0) == T(13));
		CHECK(m(1, 0) == T(14));
		CHECK(m(2, 0) == T(15));
	}

	TEST_CASE_TEMPLATE("Matrix3x3 type conversion", T, float, double)
	{
		Matrix3x3<T> mf(
			Vector3<T>(T(1.5), T(2.5), T(3.5)),
			Vector3<T>(T(4.5), T(5.5), T(6.5)),
			Vector3<T>(T(7.5), T(8.5), T(9.5)));

		Matrix3x3<int> mi = static_cast<Matrix3x3<int>>(mf);
		CHECK(mi(0, 0) == 1);
		CHECK(mi(0, 1) == 2);
		CHECK(mi(0, 2) == 3);
		CHECK(mi(1, 0) == 4);
		CHECK(mi(1, 1) == 5);
		CHECK(mi(1, 2) == 6);
		CHECK(mi(2, 0) == 7);
		CHECK(mi(2, 1) == 8);
		CHECK(mi(2, 2) == 9);
	}

	TEST_CASE_TEMPLATE("Matrix3x3 determinant", T, int32_t, uint32_t, float, double)
	{
		// Test with identity matrix (determinant should be 1)
		Matrix3x3<T> m1 = Matrix3x3<T>::Identity;
		CHECK(m1.Determinant() == T(1));

		// Test with a custom matrix
		Matrix3x3<T> m2(
			Vector3<T>(T(3), T(0), T(2)),
			Vector3<T>(T(2), T(0), T(-2)),
			Vector3<T>(T(0), T(1), T(1)));

		// Determinant = 3*(0*1 - (-2)*1) - 0*(2*1 - (-2)*0) + 2*(2*1 - 0*0) = 3*(2) + 0 + 2*(2) = 6 + 4 = 10
		CHECK(m2.Determinant() == T(10));
	}

	TEST_CASE_TEMPLATE("Matrix3x3 transpose", T, int32_t, uint32_t, float, double)
	{
		Matrix3x3<T> m(
			Vector3<T>(T(1), T(2), T(3)),
			Vector3<T>(T(4), T(5), T(6)),
			Vector3<T>(T(7), T(8), T(9)));

		Matrix3x3<T> transposed = m.Transpose();
		CHECK(transposed(0, 0) == T(1));
		CHECK(transposed(0, 1) == T(4));
		CHECK(transposed(0, 2) == T(7));
		CHECK(transposed(1, 0) == T(2));
		CHECK(transposed(1, 1) == T(5));
		CHECK(transposed(1, 2) == T(8));
		CHECK(transposed(2, 0) == T(3));
		CHECK(transposed(2, 1) == T(6));
		CHECK(transposed(2, 2) == T(9));
	}

	TEST_CASE_TEMPLATE("Matrix3x3 adjugate", T, int32_t, float, double)
	{
		Matrix3x3<T> m{
			{ 1, 2, 3 },
			{ 0, 1, 4 },
			{ 5, 6, 0 }
		};

		Matrix3x3<T> mAdj{
			{ -24, 18, 5 },
			{ 20, -15, -4 },
			{ -5, 4, 1 }
		};

		Matrix3x3<T> adj = m.Adjugate();
		CHECK(adj == mAdj);
	}

	TEST_CASE_TEMPLATE("Matrix3x3 inverse", T, float, double)
	{
		Matrix3x3<T> m(
			{ 2, 1, 1 },
			{ 1, 2, 1 },
			{ 2, 3, 1 });

		Matrix3x3<T> inv = m.Inverse();

		CHECK(inv(0, 0) == doctest::Approx(T(0.5)));
		CHECK(inv(0, 1) == doctest::Approx(T(-1)));
		CHECK(inv(0, 2) == doctest::Approx(T(0.5)));
		CHECK(inv(1, 0) == doctest::Approx(T(-0.5)));
		CHECK(inv(1, 1) == doctest::Approx(T(0)));
		CHECK(inv(1, 2) == doctest::Approx(T(0.5)));
		CHECK(inv(2, 0) == doctest::Approx(T(0.5)));
		CHECK(inv(2, 1) == doctest::Approx(T(2)));
		CHECK(inv(2, 2) == doctest::Approx(T(-1.5)));

		// Test multiplication with inverse = identity
		Matrix3x3<T> identity = m * inv;
		CHECK(identity(0, 0) == doctest::Approx(T(1)));
		CHECK(identity(0, 1) == doctest::Approx(T(0)));
		CHECK(identity(0, 2) == doctest::Approx(T(0)));
		CHECK(identity(1, 0) == doctest::Approx(T(0)));
		CHECK(identity(1, 1) == doctest::Approx(T(1)));
		CHECK(identity(1, 2) == doctest::Approx(T(0)));
		CHECK(identity(2, 0) == doctest::Approx(T(0)));
		CHECK(identity(2, 1) == doctest::Approx(T(0)));
		CHECK(identity(2, 2) == doctest::Approx(T(1)));

		// Test inverse of zero determinant matrix
		Matrix3x3<T> singular(
			Vector3<T>(T(1), T(2), T(3)),
			Vector3<T>(T(2), T(4), T(6)),
			Vector3<T>(T(3), T(6), T(9)));
		Matrix3x3<T> zeroInv = singular.Inverse();
		CHECK(zeroInv == Matrix3x3<T>::Zero);
	}

	TEST_CASE_TEMPLATE("Matrix3x3 static constants", T, int32_t, uint32_t, float, double)
	{
		// Test Zero matrix
		Matrix3x3<T> zero = Matrix3x3<T>::Zero;
		CHECK(zero(0, 0) == T(0));
		CHECK(zero(0, 1) == T(0));
		CHECK(zero(0, 2) == T(0));
		CHECK(zero(1, 0) == T(0));
		CHECK(zero(1, 1) == T(0));
		CHECK(zero(1, 2) == T(0));
		CHECK(zero(2, 0) == T(0));
		CHECK(zero(2, 1) == T(0));
		CHECK(zero(2, 2) == T(0));

		// Test Identity matrix
		Matrix3x3<T> identity = Matrix3x3<T>::Identity;
		CHECK(identity(0, 0) == T(1));
		CHECK(identity(0, 1) == T(0));
		CHECK(identity(0, 2) == T(0));
		CHECK(identity(1, 0) == T(0));
		CHECK(identity(1, 1) == T(1));
		CHECK(identity(1, 2) == T(0));
		CHECK(identity(2, 0) == T(0));
		CHECK(identity(2, 1) == T(0));
		CHECK(identity(2, 2) == T(1));
	}

	TEST_CASE_TEMPLATE("Matrix3x3 comparison operators", T, int32_t, uint32_t, float, double)
	{
		Matrix3x3<T> m1(
			Vector3<T>(T(1), T(2), T(3)),
			Vector3<T>(T(4), T(5), T(6)),
			Vector3<T>(T(7), T(8), T(9)));

		Matrix3x3<T> m2(
			Vector3<T>(T(1), T(2), T(3)),
			Vector3<T>(T(4), T(5), T(6)),
			Vector3<T>(T(7), T(8), T(9)));

		Matrix3x3<T> m3(
			Vector3<T>(T(9), T(8), T(7)),
			Vector3<T>(T(6), T(5), T(4)),
			Vector3<T>(T(3), T(2), T(1)));

		CHECK(m1 == m2);
		CHECK(m1 != m3);
		CHECK_FALSE(m1 == m3);
		CHECK_FALSE(m1 != m2);
	}

	TEST_CASE_TEMPLATE("Matrix3x3 matrix multiplication", T, int32_t, uint32_t, float, double)
	{
		Matrix3x3<T> m1(
			Vector3<T>(T(1), T(2), T(3)),
			Vector3<T>(T(4), T(5), T(6)),
			Vector3<T>(T(7), T(8), T(9)));

		Matrix3x3<T> m2(
			Vector3<T>(T(9), T(8), T(7)),
			Vector3<T>(T(6), T(5), T(4)),
			Vector3<T>(T(3), T(2), T(1)));

		// [1 2 3]   [9 8 7]   [30 24 18]
		// [4 5 6] * [6 5 4] = [84 69 54]
		// [7 8 9]   [3 2 1]   [138 114 90]
		Matrix3x3<T> result = m1 * m2;
		CHECK(result(0, 0) == T(30));  // 1*9 + 2*6 + 3*3 = 30
		CHECK(result(0, 1) == T(24));  // 1*8 + 2*5 + 3*2 = 24
		CHECK(result(0, 2) == T(18));  // 1*7 + 2*4 + 3*1 = 18
		CHECK(result(1, 0) == T(84));  // 4*9 + 5*6 + 6*3 = 84
		CHECK(result(1, 1) == T(69));  // 4*8 + 5*5 + 6*2 = 69
		CHECK(result(1, 2) == T(54));  // 4*7 + 5*4 + 6*1 = 54
		CHECK(result(2, 0) == T(138)); // 7*9 + 8*6 + 9*3 = 138
		CHECK(result(2, 1) == T(114)); // 7*8 + 8*5 + 9*2 = 114
		CHECK(result(2, 2) == T(90));  // 7*7 + 8*4 + 9*1 = 90
	}

	TEST_CASE_TEMPLATE("Matrix3x3 vector multiplication", T, int32_t, uint32_t, float, double)
	{
		Matrix3x3<T> m(
			Vector3<T>(T(1), T(2), T(3)),
			Vector3<T>(T(4), T(5), T(6)),
			Vector3<T>(T(7), T(8), T(9)));

		Vector3<T> v(T(2), T(3), T(4));

		// [1 2 3]   [2]   [20]
		// [4 5 6] * [3] = [47]
		// [7 8 9]   [4]   [74]
		Vector3<T> result = m * v;
		CHECK(result.x == T(20)); // 1*2 + 2*3 + 3*4 = 20
		CHECK(result.y == T(47)); // 4*2 + 5*3 + 6*4 = 47
		CHECK(result.z == T(74)); // 7*2 + 8*3 + 9*4 = 74
	}

	TEST_CASE_TEMPLATE("Matrix3x3 scalar operations", T, int32_t, uint32_t, float, double)
	{
		Matrix3x3<T> m(
			Vector3<T>(T(1), T(2), T(3)),
			Vector3<T>(T(4), T(5), T(6)),
			Vector3<T>(T(7), T(8), T(9)));

		T scalar = T(2);

		// Matrix * Scalar
		Matrix3x3<T> mMulScalar = m * scalar;
		CHECK(mMulScalar(0, 0) == T(2));
		CHECK(mMulScalar(0, 1) == T(4));
		CHECK(mMulScalar(0, 2) == T(6));
		CHECK(mMulScalar(1, 0) == T(8));
		CHECK(mMulScalar(1, 1) == T(10));
		CHECK(mMulScalar(1, 2) == T(12));
		CHECK(mMulScalar(2, 0) == T(14));
		CHECK(mMulScalar(2, 1) == T(16));
		CHECK(mMulScalar(2, 2) == T(18));

		// Scalar * Matrix
		Matrix3x3<T> scalarMulM = scalar * m;
		CHECK(scalarMulM(0, 0) == T(2));
		CHECK(scalarMulM(0, 1) == T(4));
		CHECK(scalarMulM(0, 2) == T(6));
		CHECK(scalarMulM(1, 0) == T(8));
		CHECK(scalarMulM(1, 1) == T(10));
		CHECK(scalarMulM(1, 2) == T(12));
		CHECK(scalarMulM(2, 0) == T(14));
		CHECK(scalarMulM(2, 1) == T(16));
		CHECK(scalarMulM(2, 2) == T(18));

		// Matrix / Scalar
		Matrix3x3<T> mDivScalar = m / scalar;
		CHECK(mDivScalar(0, 0) == T(1) / T(2));
		CHECK(mDivScalar(0, 1) == T(2) / T(2));
		CHECK(mDivScalar(0, 2) == T(3) / T(2));
		CHECK(mDivScalar(1, 0) == T(4) / T(2));
		CHECK(mDivScalar(1, 1) == T(5) / T(2));
		CHECK(mDivScalar(1, 2) == T(6) / T(2));
		CHECK(mDivScalar(2, 0) == T(7) / T(2));
		CHECK(mDivScalar(2, 1) == T(8) / T(2));
		CHECK(mDivScalar(2, 2) == T(9) / T(2));

		// Scalar / Matrix (element-wise)
		Matrix3x3<T> scalarDivM = scalar / m;
		CHECK(scalarDivM(0, 0) == T(2) / T(1));
		CHECK(scalarDivM(0, 1) == T(2) / T(2));
		CHECK(scalarDivM(0, 2) == T(2) / T(3));
		CHECK(scalarDivM(1, 0) == T(2) / T(4));
		CHECK(scalarDivM(1, 1) == T(2) / T(5));
		CHECK(scalarDivM(1, 2) == T(2) / T(6));
		CHECK(scalarDivM(2, 0) == T(2) / T(7));
		CHECK(scalarDivM(2, 1) == T(2) / T(8));
		CHECK(scalarDivM(2, 2) == T(2) / T(9));
	}

	TEST_CASE_TEMPLATE("Matrix3x3 compound assignment with scalars", T, int32_t, uint32_t, float, double)
	{
		Matrix3x3<T> m(
			Vector3<T>(T(1), T(2), T(3)),
			Vector3<T>(T(4), T(5), T(6)),
			Vector3<T>(T(7), T(8), T(9)));

		T scalar = T(2);

		// *=
		Matrix3x3<T> mMulEq = m;
		mMulEq *= scalar;
		CHECK(mMulEq(0, 0) == T(2));
		CHECK(mMulEq(0, 1) == T(4));
		CHECK(mMulEq(0, 2) == T(6));
		CHECK(mMulEq(1, 0) == T(8));
		CHECK(mMulEq(1, 1) == T(10));
		CHECK(mMulEq(1, 2) == T(12));
		CHECK(mMulEq(2, 0) == T(14));
		CHECK(mMulEq(2, 1) == T(16));
		CHECK(mMulEq(2, 2) == T(18));

		// /=
		Matrix3x3<T> mDivEq = m;
		mDivEq /= scalar;
		CHECK(mDivEq(0, 0) == T(1) / T(2));
		CHECK(mDivEq(0, 1) == T(2) / T(2));
		CHECK(mDivEq(0, 2) == T(3) / T(2));
		CHECK(mDivEq(1, 0) == T(4) / T(2));
		CHECK(mDivEq(1, 1) == T(5) / T(2));
		CHECK(mDivEq(1, 2) == T(6) / T(2));
		CHECK(mDivEq(2, 0) == T(7) / T(2));
		CHECK(mDivEq(2, 1) == T(8) / T(2));
		CHECK(mDivEq(2, 2) == T(9) / T(2));
	}
}
