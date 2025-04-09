#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <Ailurus/Math/Vector4.hpp>

using namespace Ailurus;

TEST_SUITE("Vector4")
{
	TEST_CASE_TEMPLATE("Vector4 constructors", T, int32_t, uint32_t, float, double)
	{
		// Default constructor
		Vector4<T> v1;
		CHECK(v1.x == T(0));
		CHECK(v1.y == T(0));
		CHECK(v1.z == T(0));
		CHECK(v1.w == T(0));

		// Parameterized constructor
		Vector4<T> v2(T(2), T(3), T(4), T(5));
		CHECK(v2.x == T(2));
		CHECK(v2.y == T(3));
		CHECK(v2.z == T(4));
		CHECK(v2.w == T(5));

		// Copy constructor
		Vector4<T> v3 = v2;
		CHECK(v3.x == T(2));
		CHECK(v3.y == T(3));
		CHECK(v3.z == T(4));
		CHECK(v3.w == T(5));

		// Move constructor
		Vector4<T> v4 = std::move(Vector4<T>(T(6), T(7), T(8), T(9)));
		CHECK(v4.x == T(6));
		CHECK(v4.y == T(7));
		CHECK(v4.z == T(8));
		CHECK(v4.w == T(9));
	}

	TEST_CASE_TEMPLATE("Vector4 assignment operators", T, int32_t, uint32_t, float, double)
	{
		// Copy assignment
		Vector4<T> v1(T(1), T(2), T(3), T(4));
		Vector4<T> v2;
		v2 = v1;
		CHECK(v2.x == T(1));
		CHECK(v2.y == T(2));
		CHECK(v2.z == T(3));
		CHECK(v2.w == T(4));

		// Move assignment
		Vector4<T> v3;
		v3 = Vector4<T>(T(5), T(6), T(7), T(8));
		CHECK(v3.x == T(5));
		CHECK(v3.y == T(6));
		CHECK(v3.z == T(7));
		CHECK(v3.w == T(8));
	}

	TEST_CASE_TEMPLATE("Vector4 index operator", T, int32_t, uint32_t, float, double)
	{
		Vector4<T> v(T(1), T(2), T(3), T(4));
		CHECK(v[0] == T(1));
		CHECK(v[1] == T(2));
		CHECK(v[2] == T(3));
		CHECK(v[3] == T(4));

		// Test modification through index operator
		v[0] = T(5);
		v[1] = T(6);
		v[2] = T(7);
		v[3] = T(8);
		CHECK(v.x == T(5));
		CHECK(v.y == T(6));
		CHECK(v.z == T(7));
		CHECK(v.w == T(8));

		// Test const index operator
		const Vector4<T> cv(T(9), T(10), T(11), T(12));
		CHECK(cv[0] == T(9));
		CHECK(cv[1] == T(10));
		CHECK(cv[2] == T(11));
		CHECK(cv[3] == T(12));
	}

	TEST_CASE_TEMPLATE("Vector4 type conversion", T, float, double)
	{
		Vector4<T> vf(T(1.5), T(2.5), T(3.5), T(4.5));
		Vector4<int> vi = static_cast<Vector4<int>>(vf);
		CHECK(vi.x == 1);
		CHECK(vi.y == 2);
		CHECK(vi.z == 3);
		CHECK(vi.w == 4);
	}

	TEST_CASE_TEMPLATE("Vector4 magnitude calculations", T, int32_t, uint32_t, float, double)
	{
		Vector4<T> v(T(2), T(3), T(6), T(6));
		CHECK(v.SquareMagnitude() == T(85)); // 2*2 + 3*3 + 6*6 + 6*6 = 85
		CHECK(v.Magnitude() == T(std::sqrt(85)));
	}

	TEST_CASE_TEMPLATE("Vector4 normalization", T, float, double)
	{
		Vector4<T> v(T(2), T(3), T(6), T(6));
		T mag = std::sqrt(T(85));
		Vector4<T> normalized = v.Normalized();
		CHECK(normalized.x == doctest::Approx(T(2) / mag));
		CHECK(normalized.y == doctest::Approx(T(3) / mag));
		CHECK(normalized.z == doctest::Approx(T(6) / mag));
		CHECK(normalized.w == doctest::Approx(T(6) / mag));
		CHECK(normalized.Magnitude() == doctest::Approx(T(1)));

		v.Normalize();
		CHECK(v.x == doctest::Approx(T(2) / mag));
		CHECK(v.y == doctest::Approx(T(3) / mag));
		CHECK(v.z == doctest::Approx(T(6) / mag));
		CHECK(v.w == doctest::Approx(T(6) / mag));
		CHECK(v.Magnitude() == doctest::Approx(T(1)));

		// Test edge case: normalizing zero vector
		Vector4<T> zeroVec;
		zeroVec.Normalize();
		CHECK(zeroVec.x == T(0));
		CHECK(zeroVec.y == T(0));
		CHECK(zeroVec.z == T(0));
		CHECK(zeroVec.w == T(0));
	}

	TEST_CASE_TEMPLATE("Vector4 dot product", T, int32_t, uint32_t, float, double)
	{
		Vector4<T> v1(T(1), T(2), T(3), T(4));
		Vector4<T> v2(T(5), T(6), T(7), T(8));
		T dot = v1.Dot(v2);
		CHECK(dot == T(70)); // 1*5 + 2*6 + 3*7 + 4*8 = 70
	}

	TEST_CASE_TEMPLATE("Vector4 static constants", T, int32_t, uint32_t, float, double)
	{
		CHECK(Vector4<T>::Zero.x == T(0));
		CHECK(Vector4<T>::Zero.y == T(0));
		CHECK(Vector4<T>::Zero.z == T(0));
		CHECK(Vector4<T>::Zero.w == T(0));
		CHECK(Vector4<T>::One.x == T(1));
		CHECK(Vector4<T>::One.y == T(1));
		CHECK(Vector4<T>::One.z == T(1));
		CHECK(Vector4<T>::One.w == T(1));
	}

	TEST_CASE_TEMPLATE("Vector4 comparison operators", T, int32_t, uint32_t, float, double)
	{
		Vector4<T> v1(T(1), T(2), T(3), T(4));
		Vector4<T> v2(T(1), T(2), T(3), T(4));
		Vector4<T> v3(T(5), T(6), T(7), T(8));

		CHECK(v1 == v2);
		CHECK(v1 != v3);
		CHECK_FALSE(v1 == v3);
		CHECK_FALSE(v1 != v2);
	}

	TEST_CASE_TEMPLATE("Vector4 scalar operations", T, float, double)
	{
		Vector4<T> v(T(2), T(3), T(4), T(5));
		T scalar = T(2);

		// Addition
		Vector4<T> vAddScalar = v + scalar;
		CHECK(vAddScalar.x == T(4));
		CHECK(vAddScalar.y == T(5));
		CHECK(vAddScalar.z == T(6));
		CHECK(vAddScalar.w == T(7));

		Vector4<T> scalarAddV = scalar + v;
		CHECK(scalarAddV.x == T(4));
		CHECK(scalarAddV.y == T(5));
		CHECK(scalarAddV.z == T(6));
		CHECK(scalarAddV.w == T(7));

		// Subtraction
		Vector4<T> vSubScalar = v - scalar;
		CHECK(vSubScalar.x == T(0));
		CHECK(vSubScalar.y == T(1));
		CHECK(vSubScalar.z == T(2));
		CHECK(vSubScalar.w == T(3));

		Vector4<T> scalarSubV = scalar - v;
		CHECK(scalarSubV.x == T(0));
		CHECK(scalarSubV.y == T(-1));
		CHECK(scalarSubV.z == T(-2));
		CHECK(scalarSubV.w == T(-3));

		// Multiplication
		Vector4<T> vMulScalar = v * scalar;
		CHECK(vMulScalar.x == T(4));
		CHECK(vMulScalar.y == T(6));
		CHECK(vMulScalar.z == T(8));
		CHECK(vMulScalar.w == T(10));

		Vector4<T> scalarMulV = scalar * v;
		CHECK(scalarMulV.x == T(4));
		CHECK(scalarMulV.y == T(6));
		CHECK(scalarMulV.z == T(8));
		CHECK(scalarMulV.w == T(10));

		// Division
		Vector4<T> vDivScalar = v / scalar;
		CHECK(vDivScalar.x == T(1));
		CHECK(vDivScalar.y == T(1.5));
		CHECK(vDivScalar.z == T(2));
		CHECK(vDivScalar.w == T(2.5));

		Vector4<T> scalarDivV = scalar / v;
		CHECK(scalarDivV.x == doctest::Approx(T(2) / T(2)));
		CHECK(scalarDivV.y == doctest::Approx(T(2) / T(3)));
		CHECK(scalarDivV.z == doctest::Approx(T(2) / T(4)));
		CHECK(scalarDivV.w == doctest::Approx(T(2) / T(5)));
	}

	TEST_CASE_TEMPLATE("Vector4 compound assignment with scalars", T, float, double)
	{
		Vector4<T> v(T(2), T(3), T(4), T(5));
		T scalar = T(2);

		// +=
		Vector4<T> vAddEq = v;
		vAddEq += scalar;
		CHECK(vAddEq.x == T(4));
		CHECK(vAddEq.y == T(5));
		CHECK(vAddEq.z == T(6));
		CHECK(vAddEq.w == T(7));

		// -=
		Vector4<T> vSubEq = v;
		vSubEq -= scalar;
		CHECK(vSubEq.x == T(0));
		CHECK(vSubEq.y == T(1));
		CHECK(vSubEq.z == T(2));
		CHECK(vSubEq.w == T(3));

		// *=
		Vector4<T> vMulEq = v;
		vMulEq *= scalar;
		CHECK(vMulEq.x == T(4));
		CHECK(vMulEq.y == T(6));
		CHECK(vMulEq.z == T(8));
		CHECK(vMulEq.w == T(10));

		// /=
		Vector4<T> vDivEq = v;
		vDivEq /= scalar;
		CHECK(vDivEq.x == T(1));
		CHECK(vDivEq.y == T(1.5));
		CHECK(vDivEq.z == T(2));
		CHECK(vDivEq.w == T(2.5));
	}

	TEST_CASE_TEMPLATE("Vector4 vector operations", T, int32_t, uint32_t, float, double)
	{
		Vector4<T> v1(T(2), T(3), T(4), T(5));
		Vector4<T> v2(T(1), T(2), T(3), T(4));

		// Addition
		Vector4<T> vAdd = v1 + v2;
		CHECK(vAdd.x == T(3));
		CHECK(vAdd.y == T(5));
		CHECK(vAdd.z == T(7));
		CHECK(vAdd.w == T(9));

		// Subtraction
		Vector4<T> vSub = v1 - v2;
		CHECK(vSub.x == T(1));
		CHECK(vSub.y == T(1));
		CHECK(vSub.z == T(1));
		CHECK(vSub.w == T(1));

		// Compound addition
		Vector4<T> vAddEq = v1;
		vAddEq += v2;
		CHECK(vAddEq.x == T(3));
		CHECK(vAddEq.y == T(5));
		CHECK(vAddEq.z == T(7));
		CHECK(vAddEq.w == T(9));

		// Compound subtraction
		Vector4<T> vSubEq = v1;
		vSubEq -= v2;
		CHECK(vSubEq.x == T(1));
		CHECK(vSubEq.y == T(1));
		CHECK(vSubEq.z == T(1));
		CHECK(vSubEq.w == T(1));
	}
}