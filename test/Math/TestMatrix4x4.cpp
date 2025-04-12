#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <Ailurus/Math/Matrix4x4.hpp>
#include <Ailurus/Math/Vector4.hpp>
#include <Ailurus/Math/Matrix3x3.hpp>

using namespace Ailurus;

TEST_SUITE("Matrix4x4")
{
	TEST_CASE_TEMPLATE("Matrix4x4 constructors", T, int32_t, uint32_t, float, double)
	{
		// Default constructor
		Matrix4x4<T> m1;
		CHECK(m1(0, 0) == T(0));
		CHECK(m1(0, 1) == T(0));
		CHECK(m1(0, 2) == T(0));
		CHECK(m1(0, 3) == T(0));
		CHECK(m1(1, 0) == T(0));
		CHECK(m1(1, 1) == T(0));
		CHECK(m1(1, 2) == T(0));
		CHECK(m1(1, 3) == T(0));
		CHECK(m1(2, 0) == T(0));
		CHECK(m1(2, 1) == T(0));
		CHECK(m1(2, 2) == T(0));
		CHECK(m1(2, 3) == T(0));
		CHECK(m1(3, 0) == T(0));
		CHECK(m1(3, 1) == T(0));
		CHECK(m1(3, 2) == T(0));
		CHECK(m1(3, 3) == T(0));

		// Constructor with Vector4 parameters
		Matrix4x4<T> m2(
			Vector4<T>(T(1), T(2), T(3), T(4)),
			Vector4<T>(T(5), T(6), T(7), T(8)),
			Vector4<T>(T(9), T(10), T(11), T(12)),
			Vector4<T>(T(13), T(14), T(15), T(16)));
		CHECK(m2(0, 0) == T(1));
		CHECK(m2(0, 1) == T(2));
		CHECK(m2(0, 2) == T(3));
		CHECK(m2(0, 3) == T(4));
		CHECK(m2(1, 0) == T(5));
		CHECK(m2(1, 1) == T(6));
		CHECK(m2(1, 2) == T(7));
		CHECK(m2(1, 3) == T(8));
		CHECK(m2(2, 0) == T(9));
		CHECK(m2(2, 1) == T(10));
		CHECK(m2(2, 2) == T(11));
		CHECK(m2(2, 3) == T(12));
		CHECK(m2(3, 0) == T(13));
		CHECK(m2(3, 1) == T(14));
		CHECK(m2(3, 2) == T(15));
		CHECK(m2(3, 3) == T(16));

		// Copy constructor
		Matrix4x4<T> m3 = m2;
		CHECK(m3(0, 0) == T(1));
		CHECK(m3(0, 1) == T(2));
		CHECK(m3(0, 2) == T(3));
		CHECK(m3(0, 3) == T(4));
		CHECK(m3(1, 0) == T(5));
		CHECK(m3(1, 1) == T(6));
		CHECK(m3(1, 2) == T(7));
		CHECK(m3(1, 3) == T(8));
		CHECK(m3(2, 0) == T(9));
		CHECK(m3(2, 1) == T(10));
		CHECK(m3(2, 2) == T(11));
		CHECK(m3(2, 3) == T(12));
		CHECK(m3(3, 0) == T(13));
		CHECK(m3(3, 1) == T(14));
		CHECK(m3(3, 2) == T(15));
		CHECK(m3(3, 3) == T(16));

		// Move constructor
		Matrix4x4<T> m4 = std::move(Matrix4x4<T>(
			Vector4<T>(T(1), T(2), T(3), T(4)),
			Vector4<T>(T(5), T(6), T(7), T(8)),
			Vector4<T>(T(9), T(10), T(11), T(12)),
			Vector4<T>(T(13), T(14), T(15), T(16))));
		CHECK(m4(0, 0) == T(1));
		CHECK(m4(0, 1) == T(2));
		CHECK(m4(0, 2) == T(3));
		CHECK(m4(0, 3) == T(4));
		CHECK(m4(1, 0) == T(5));
		CHECK(m4(1, 1) == T(6));
		CHECK(m4(1, 2) == T(7));
		CHECK(m4(1, 3) == T(8));
		CHECK(m4(2, 0) == T(9));
		CHECK(m4(2, 1) == T(10));
		CHECK(m4(2, 2) == T(11));
		CHECK(m4(2, 3) == T(12));
		CHECK(m4(3, 0) == T(13));
		CHECK(m4(3, 1) == T(14));
		CHECK(m4(3, 2) == T(15));
		CHECK(m4(3, 3) == T(16));

		// Constructor with initializer list
		Matrix4x4<T> m5 = {
			Vector4<T>(T(1), T(2), T(3), T(4)),
			Vector4<T>(T(5), T(6), T(7), T(8)),
			Vector4<T>(T(9), T(10), T(11), T(12)),
			Vector4<T>(T(13), T(14), T(15), T(16))
		};
		CHECK(m5(0, 0) == T(1));
		CHECK(m5(0, 1) == T(2));
		CHECK(m5(0, 2) == T(3));
		CHECK(m5(0, 3) == T(4));
		CHECK(m5(1, 0) == T(5));
		CHECK(m5(1, 1) == T(6));
		CHECK(m5(1, 2) == T(7));
		CHECK(m5(1, 3) == T(8));
		CHECK(m5(2, 0) == T(9));
		CHECK(m5(2, 1) == T(10));
		CHECK(m5(2, 2) == T(11));
		CHECK(m5(2, 3) == T(12));
		CHECK(m5(3, 0) == T(13));
		CHECK(m5(3, 1) == T(14));
		CHECK(m5(3, 2) == T(15));
		CHECK(m5(3, 3) == T(16));
	}

	TEST_CASE_TEMPLATE("Matrix4x4 element access and modification", T, int32_t, uint32_t, float, double)
	{
		Matrix4x4<T> m(
			Vector4<T>(T(1), T(2), T(3), T(4)),
			Vector4<T>(T(5), T(6), T(7), T(8)),
			Vector4<T>(T(9), T(10), T(11), T(12)),
			Vector4<T>(T(13), T(14), T(15), T(16)));

		// Access elements
		CHECK(m(0, 0) == T(1));
		CHECK(m(0, 1) == T(2));
		CHECK(m(0, 2) == T(3));
		CHECK(m(0, 3) == T(4));
		CHECK(m(1, 0) == T(5));
		CHECK(m(1, 1) == T(6));
		CHECK(m(1, 2) == T(7));
		CHECK(m(1, 3) == T(8));
		CHECK(m(2, 0) == T(9));
		CHECK(m(2, 1) == T(10));
		CHECK(m(2, 2) == T(11));
		CHECK(m(2, 3) == T(12));
		CHECK(m(3, 0) == T(13));
		CHECK(m(3, 1) == T(14));
		CHECK(m(3, 2) == T(15));
		CHECK(m(3, 3) == T(16));

		// Access elements with array syntax
		CHECK(m[0][0] == T(1));
		CHECK(m[0][1] == T(2));
		CHECK(m[0][2] == T(3));
		CHECK(m[0][3] == T(4));
		CHECK(m[1][0] == T(5));
		CHECK(m[1][1] == T(6));
		CHECK(m[1][2] == T(7));
		CHECK(m[1][3] == T(8));
		CHECK(m[2][0] == T(9));
		CHECK(m[2][1] == T(10));
		CHECK(m[2][2] == T(11));
		CHECK(m[2][3] == T(12));
		CHECK(m[3][0] == T(13));
		CHECK(m[3][1] == T(14));
		CHECK(m[3][2] == T(15));
		CHECK(m[3][3] == T(16));

		// Modify elements
		m(0, 0) = T(17);
		m(1, 1) = T(18);
		m(2, 2) = T(19);
		m(3, 3) = T(20);

		CHECK(m(0, 0) == T(17));
		CHECK(m(1, 1) == T(18));
		CHECK(m(2, 2) == T(19));
		CHECK(m(3, 3) == T(20));

		// Modify with array syntax
		m[0][1] = T(21);
		m[1][2] = T(22);
		m[2][3] = T(23);
		m[3][0] = T(24);

		CHECK(m[0][1] == T(21));
		CHECK(m[1][2] == T(22));
		CHECK(m[2][3] == T(23));
		CHECK(m[3][0] == T(24));
	}

	TEST_CASE_TEMPLATE("Matrix4x4 get/set row and column", T, int32_t, uint32_t, float, double)
	{
		Matrix4x4<T> m(
			Vector4<T>(T(1), T(2), T(3), T(4)),
			Vector4<T>(T(5), T(6), T(7), T(8)),
			Vector4<T>(T(9), T(10), T(11), T(12)),
			Vector4<T>(T(13), T(14), T(15), T(16)));

		// Get rows
		Vector4<T> row0 = m.GetRow(0);
		CHECK(row0.x == T(1));
		CHECK(row0.y == T(2));
		CHECK(row0.z == T(3));
		CHECK(row0.w == T(4));

		Vector4<T> row1 = m.GetRow(1);
		CHECK(row1.x == T(5));
		CHECK(row1.y == T(6));
		CHECK(row1.z == T(7));
		CHECK(row1.w == T(8));

		// Get columns
		Vector4<T> col0 = m.GetCol(0);
		CHECK(col0.x == T(1));
		CHECK(col0.y == T(5));
		CHECK(col0.z == T(9));
		CHECK(col0.w == T(13));

		Vector4<T> col1 = m.GetCol(1);
		CHECK(col1.x == T(2));
		CHECK(col1.y == T(6));
		CHECK(col1.z == T(10));
		CHECK(col1.w == T(14));

		// Set rows
		m.SetRow(0, Vector4<T>(T(17), T(18), T(19), T(20)));
		CHECK(m(0, 0) == T(17));
		CHECK(m(0, 1) == T(18));
		CHECK(m(0, 2) == T(19));
		CHECK(m(0, 3) == T(20));

		// Set columns
		m.SetCol(0, Vector4<T>(T(21), T(22), T(23), T(24)));
		CHECK(m(0, 0) == T(21));
		CHECK(m(1, 0) == T(22));
		CHECK(m(2, 0) == T(23));
		CHECK(m(3, 0) == T(24));
	}

	TEST_CASE_TEMPLATE("Matrix4x4 type conversion", T, float, double)
	{
		Matrix4x4<T> mf(
			Vector4<T>(T(1.5), T(2.5), T(3.5), T(4.5)),
			Vector4<T>(T(5.5), T(6.5), T(7.5), T(8.5)),
			Vector4<T>(T(9.5), T(10.5), T(11.5), T(12.5)),
			Vector4<T>(T(13.5), T(14.5), T(15.5), T(16.5)));

		Matrix4x4<int> mi = static_cast<Matrix4x4<int>>(mf);
		CHECK(mi(0, 0) == 1);
		CHECK(mi(0, 1) == 2);
		CHECK(mi(0, 2) == 3);
		CHECK(mi(0, 3) == 4);
		CHECK(mi(1, 0) == 5);
		CHECK(mi(1, 1) == 6);
		CHECK(mi(1, 2) == 7);
		CHECK(mi(1, 3) == 8);
		CHECK(mi(2, 0) == 9);
		CHECK(mi(2, 1) == 10);
		CHECK(mi(2, 2) == 11);
		CHECK(mi(2, 3) == 12);
		CHECK(mi(3, 0) == 13);
		CHECK(mi(3, 1) == 14);
		CHECK(mi(3, 2) == 15);
		CHECK(mi(3, 3) == 16);
	}

	TEST_CASE_TEMPLATE("Matrix4x4 determinant", T, float, double)
	{
		// Simple identity matrix - determinant should be 1
		Matrix4x4<T> identity = Matrix4x4<T>::Identity;
		CHECK(identity.Determinant() == T(1));

		// Zero matrix - determinant should be 0
		Matrix4x4<T> zero = Matrix4x4<T>::Zero;
		CHECK(zero.Determinant() == T(0));

		// Test with a matrix whose determinant is known
		Matrix4x4<T> m(
			Vector4<T>(T(3), T(0), T(2), T(-1)),
			Vector4<T>(T(1), T(2), T(0), T(-2)),
			Vector4<T>(T(4), T(0), T(6), T(-3)),
			Vector4<T>(T(5), T(0), T(2), T(0)));

		// Determinant of this matrix should be 20
		CHECK(m.Determinant() == doctest::Approx(T(20)));
	}

	TEST_CASE_TEMPLATE("Matrix4x4 transpose", T, int32_t, uint32_t, float, double)
	{
		Matrix4x4<T> m(
			Vector4<T>(T(1), T(2), T(3), T(4)),
			Vector4<T>(T(5), T(6), T(7), T(8)),
			Vector4<T>(T(9), T(10), T(11), T(12)),
			Vector4<T>(T(13), T(14), T(15), T(16)));

		Matrix4x4<T> transposed = m.Transpose();
		CHECK(transposed(0, 0) == T(1));
		CHECK(transposed(0, 1) == T(5));
		CHECK(transposed(0, 2) == T(9));
		CHECK(transposed(0, 3) == T(13));
		CHECK(transposed(1, 0) == T(2));
		CHECK(transposed(1, 1) == T(6));
		CHECK(transposed(1, 2) == T(10));
		CHECK(transposed(1, 3) == T(14));
		CHECK(transposed(2, 0) == T(3));
		CHECK(transposed(2, 1) == T(7));
		CHECK(transposed(2, 2) == T(11));
		CHECK(transposed(2, 3) == T(15));
		CHECK(transposed(3, 0) == T(4));
		CHECK(transposed(3, 1) == T(8));
		CHECK(transposed(3, 2) == T(12));
		CHECK(transposed(3, 3) == T(16));
	}

	TEST_CASE_TEMPLATE("Matrix4x4 adjugate", T, float, double)
	{
		// Identity matrix - adjugate should be identity too
		Matrix4x4<T> identity = Matrix4x4<T>::Identity;
		Matrix4x4<T> adjIdentity = identity.Adjugate();

		CHECK(adjIdentity(0, 0) == doctest::Approx(T(1)));
		CHECK(adjIdentity(1, 1) == doctest::Approx(T(1)));
		CHECK(adjIdentity(2, 2) == doctest::Approx(T(1)));
		CHECK(adjIdentity(3, 3) == doctest::Approx(T(1)));

		// Test with a matrix with known adjugate
		Matrix4x4<T> m(
			Vector4<T>(T(1), T(0), T(0), T(0)),
			Vector4<T>(T(0), T(2), T(0), T(0)),
			Vector4<T>(T(0), T(0), T(3), T(0)),
			Vector4<T>(T(0), T(0), T(0), T(4)));

		Matrix4x4<T> adj = m.Adjugate();
		CHECK(adj(0, 0) == doctest::Approx(T(24))); // 2*3*4
		CHECK(adj(1, 1) == doctest::Approx(T(12))); // 1*3*4
		CHECK(adj(2, 2) == doctest::Approx(T(8)));	// 1*2*4
		CHECK(adj(3, 3) == doctest::Approx(T(6)));	// 1*2*3
	}

	TEST_CASE_TEMPLATE("Matrix4x4 inverse", T, float, double)
	{
		// Test inverse of identity is identity
		Matrix4x4<T> identity = Matrix4x4<T>::Identity;
		Matrix4x4<T> invIdentity = identity.Inverse();

		CHECK(invIdentity(0, 0) == doctest::Approx(T(1)));
		CHECK(invIdentity(1, 1) == doctest::Approx(T(1)));
		CHECK(invIdentity(2, 2) == doctest::Approx(T(1)));
		CHECK(invIdentity(3, 3) == doctest::Approx(T(1)));

		// Test inverse of diagonal matrix
		Matrix4x4<T> diagonal(
			Vector4<T>(T(2), T(0), T(0), T(0)),
			Vector4<T>(T(0), T(3), T(0), T(0)),
			Vector4<T>(T(0), T(0), T(4), T(0)),
			Vector4<T>(T(0), T(0), T(0), T(5)));

		Matrix4x4<T> invDiagonal = diagonal.Inverse();
		CHECK(invDiagonal(0, 0) == doctest::Approx(T(0.5))); // 1/2
		CHECK(invDiagonal(1, 1) == doctest::Approx(T(1.0 / 3)));
		CHECK(invDiagonal(2, 2) == doctest::Approx(T(0.25))); // 1/4
		CHECK(invDiagonal(3, 3) == doctest::Approx(T(0.2)));  // 1/5

		// Test matrix * inverse = identity
		Matrix4x4<T> m(
			Vector4<T>(T(4), T(0), T(0), T(0)),
			Vector4<T>(T(0), T(3), T(0), T(2)),
			Vector4<T>(T(0), T(0), T(2), T(3)),
			Vector4<T>(T(0), T(1), T(1), T(1)));

		Matrix4x4<T> invM = m.Inverse();
		Matrix4x4<T> result = m * invM;

		CHECK(result(0, 0) == doctest::Approx(T(1)));
		CHECK(result(0, 1) == doctest::Approx(T(0)));
		CHECK(result(0, 2) == doctest::Approx(T(0)));
		CHECK(result(0, 3) == doctest::Approx(T(0)));
		CHECK(result(1, 0) == doctest::Approx(T(0)));
		CHECK(result(1, 1) == doctest::Approx(T(1)));
		CHECK(result(1, 2) == doctest::Approx(T(0)));
		CHECK(result(1, 3) == doctest::Approx(T(0)));
		CHECK(result(2, 0) == doctest::Approx(T(0)));
		CHECK(result(2, 1) == doctest::Approx(T(0)));
		CHECK(result(2, 2) == doctest::Approx(T(1)));
		CHECK(result(2, 3) == doctest::Approx(T(0)));
		CHECK(result(3, 0) == doctest::Approx(T(0)));
		CHECK(result(3, 1) == doctest::Approx(T(0)));
		CHECK(result(3, 2) == doctest::Approx(T(0)));
		CHECK(result(3, 3) == doctest::Approx(T(1)));

		// Test singular matrix returns Zero
		Matrix4x4<T> singular(
			Vector4<T>(T(1), T(2), T(3), T(4)),
			Vector4<T>(T(2), T(4), T(6), T(8)),
			Vector4<T>(T(0), T(0), T(0), T(0)),
			Vector4<T>(T(0), T(0), T(0), T(0)));

		CHECK(singular.Inverse() == Matrix4x4<T>::Zero);
	}

	TEST_CASE_TEMPLATE("Matrix4x4 static constants", T, int32_t, uint32_t, float, double)
	{
		// Test Zero matrix
		Matrix4x4<T> zero = Matrix4x4<T>::Zero;
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				CHECK(zero(i, j) == T(0));
			}
		}

		// Test Identity matrix
		Matrix4x4<T> identity = Matrix4x4<T>::Identity;
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				if (i == j)
					CHECK(identity(i, j) == T(1));
				else
					CHECK(identity(i, j) == T(0));
			}
		}
	}

	TEST_CASE_TEMPLATE("Matrix4x4 comparison operators", T, int32_t, uint32_t, float, double)
	{
		Matrix4x4<T> m1(
			Vector4<T>(T(1), T(2), T(3), T(4)),
			Vector4<T>(T(5), T(6), T(7), T(8)),
			Vector4<T>(T(9), T(10), T(11), T(12)),
			Vector4<T>(T(13), T(14), T(15), T(16)));

		Matrix4x4<T> m2(
			Vector4<T>(T(1), T(2), T(3), T(4)),
			Vector4<T>(T(5), T(6), T(7), T(8)),
			Vector4<T>(T(9), T(10), T(11), T(12)),
			Vector4<T>(T(13), T(14), T(15), T(16)));

		Matrix4x4<T> m3(
			Vector4<T>(T(16), T(15), T(14), T(13)),
			Vector4<T>(T(12), T(11), T(10), T(9)),
			Vector4<T>(T(8), T(7), T(6), T(5)),
			Vector4<T>(T(4), T(3), T(2), T(1)));

		CHECK(m1 == m2);
		CHECK(m1 != m3);
		CHECK_FALSE(m1 == m3);
		CHECK_FALSE(m1 != m2);
	}

	TEST_CASE_TEMPLATE("Matrix4x4 matrix multiplication", T, int32_t, float, double)
	{
		Matrix4x4<T> m1(
			Vector4<T>(T(1), T(2), T(3), T(4)),
			Vector4<T>(T(5), T(6), T(7), T(8)),
			Vector4<T>(T(9), T(10), T(11), T(12)),
			Vector4<T>(T(13), T(14), T(15), T(16)));

		Matrix4x4<T> m2(
			Vector4<T>(T(16), T(15), T(14), T(13)),
			Vector4<T>(T(12), T(11), T(10), T(9)),
			Vector4<T>(T(8), T(7), T(6), T(5)),
			Vector4<T>(T(4), T(3), T(2), T(1)));

		Matrix4x4<T> result = m1 * m2;

		// Expected result of multiplication
		CHECK(result(0, 0) == T(80));
		CHECK(result(0, 1) == T(70));
		CHECK(result(0, 2) == T(60));
		CHECK(result(0, 3) == T(50));
		CHECK(result(1, 0) == T(240));
		CHECK(result(1, 1) == T(214));
		CHECK(result(1, 2) == T(188));
		CHECK(result(1, 3) == T(162));
		CHECK(result(2, 0) == T(400));
		CHECK(result(2, 1) == T(358));
		CHECK(result(2, 2) == T(316));
		CHECK(result(2, 3) == T(274));
		CHECK(result(3, 0) == T(560));
		CHECK(result(3, 1) == T(502));
		CHECK(result(3, 2) == T(444));
		CHECK(result(3, 3) == T(386));

		// Identity property
		Matrix4x4<T> identity = Matrix4x4<T>::Identity;
		Matrix4x4<T> m3 = m1 * identity;
		CHECK(m3 == m1);
	}

	TEST_CASE_TEMPLATE("Matrix4x4 matrix-vector multiplication", T, int32_t, float, double)
	{
		Matrix4x4<T> m(
			Vector4<T>(T(1), T(2), T(3), T(4)),
			Vector4<T>(T(5), T(6), T(7), T(8)),
			Vector4<T>(T(9), T(10), T(11), T(12)),
			Vector4<T>(T(13), T(14), T(15), T(16)));

		Vector4<T> v(T(4), T(3), T(2), T(1));

		Vector4<T> result = m * v;

		CHECK(result.x == T(20));  // 1*4 + 2*3 + 3*2 + 4*1
		CHECK(result.y == T(60));  // 5*4 + 6*3 + 7*2 + 8*1
		CHECK(result.z == T(100)); // 9*4 + 10*3 + 11*2 + 12*1
		CHECK(result.w == T(140)); // 13*4 + 14*3 + 15*2 + 16*1
	}

	TEST_CASE_TEMPLATE("Matrix4x4 scalar operations", T, int32_t, float, double)
	{
		Matrix4x4<T> m(
			Vector4<T>(T(1), T(2), T(3), T(4)),
			Vector4<T>(T(5), T(6), T(7), T(8)),
			Vector4<T>(T(9), T(10), T(11), T(12)),
			Vector4<T>(T(13), T(14), T(15), T(16)));

		// Matrix-Scalar multiplication
		Matrix4x4<T> mulResult = m * T(2);
		CHECK(mulResult(0, 0) == T(2));
		CHECK(mulResult(0, 1) == T(4));
		CHECK(mulResult(0, 2) == T(6));
		CHECK(mulResult(0, 3) == T(8));
		CHECK(mulResult(1, 0) == T(10));
		CHECK(mulResult(1, 1) == T(12));
		CHECK(mulResult(1, 2) == T(14));
		CHECK(mulResult(1, 3) == T(16));
		CHECK(mulResult(2, 0) == T(18));
		CHECK(mulResult(2, 1) == T(20));
		CHECK(mulResult(2, 2) == T(22));
		CHECK(mulResult(2, 3) == T(24));
		CHECK(mulResult(3, 0) == T(26));
		CHECK(mulResult(3, 1) == T(28));
		CHECK(mulResult(3, 2) == T(30));
		CHECK(mulResult(3, 3) == T(32));

		// Scalar-Matrix multiplication
		Matrix4x4<T> mulResult2 = T(3) * m;
		CHECK(mulResult2(0, 0) == T(3));
		CHECK(mulResult2(1, 1) == T(18));
		CHECK(mulResult2(2, 2) == T(33));
		CHECK(mulResult2(3, 3) == T(48));

		// Matrix-Scalar division
		Matrix4x4<T> divResult = m / T(2);
		if constexpr (std::is_floating_point_v<T>)
		{
			CHECK(divResult(0, 0) == doctest::Approx(T(0.5)));
			CHECK(divResult(1, 1) == doctest::Approx(T(3)));
			CHECK(divResult(2, 2) == doctest::Approx(T(5.5)));
			CHECK(divResult(3, 3) == doctest::Approx(T(8)));
		}
		else
		{
			CHECK(divResult(0, 0) == T(0));
			CHECK(divResult(1, 1) == T(3));
			CHECK(divResult(2, 2) == T(5));
			CHECK(divResult(3, 3) == T(8));
		}

		// Scalar-Matrix division
		if constexpr (std::is_floating_point_v<T>)
		{
			Matrix4x4<T> divResult2 = T(12) / m;
			CHECK(divResult2(0, 0) == doctest::Approx(T(12)));
			CHECK(divResult2(0, 1) == doctest::Approx(T(6)));
			CHECK(divResult2(1, 1) == doctest::Approx(T(2)));
			CHECK(divResult2(2, 2) == doctest::Approx(T(12.0 / 11)));
			CHECK(divResult2(3, 3) == doctest::Approx(T(0.75)));
		}
	}

	TEST_CASE_TEMPLATE("Matrix4x4 compound assignments", T, int32_t, float, double)
	{
		Matrix4x4<T> m(
			Vector4<T>(T(1), T(2), T(3), T(4)),
			Vector4<T>(T(5), T(6), T(7), T(8)),
			Vector4<T>(T(9), T(10), T(11), T(12)),
			Vector4<T>(T(13), T(14), T(15), T(16)));

		// Compound multiplication
		Matrix4x4<T> mCopy = m;
		mCopy *= T(2);

		CHECK(mCopy(0, 0) == T(2));
		CHECK(mCopy(0, 1) == T(4));
		CHECK(mCopy(0, 2) == T(6));
		CHECK(mCopy(0, 3) == T(8));
		CHECK(mCopy(1, 0) == T(10));
		CHECK(mCopy(1, 1) == T(12));
		CHECK(mCopy(1, 2) == T(14));
		CHECK(mCopy(1, 3) == T(16));
		CHECK(mCopy(2, 0) == T(18));
		CHECK(mCopy(2, 1) == T(20));
		CHECK(mCopy(2, 2) == T(22));
		CHECK(mCopy(2, 3) == T(24));
		CHECK(mCopy(3, 0) == T(26));
		CHECK(mCopy(3, 1) == T(28));
		CHECK(mCopy(3, 2) == T(30));
		CHECK(mCopy(3, 3) == T(32));

		// Compound division
		mCopy /= T(2);
		CHECK(mCopy == m);
	}
}