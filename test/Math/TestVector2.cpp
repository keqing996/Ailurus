#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <Ailurus/Math/Vector2.hpp>

using namespace Ailurus;

TEST_SUITE("Vector2")
{
	TEST_CASE_TEMPLATE("Vector2 constructors", T, int32_t, uint32_t, float, double)
	{
		// Default constructor
		Vector2<T> v1;
		CHECK(v1.x == T(0));
		CHECK(v1.y == T(0));

		// Parameterized constructor
		Vector2<T> v2(T(2), T(3));
		CHECK(v2.x == T(2));
		CHECK(v2.y == T(3));

		// Copy constructor
		Vector2<T> v3 = v2;
		CHECK(v3.x == T(2));
		CHECK(v3.y == T(3));

		// Move constructor
		Vector2<T> v4 = std::move(Vector2<T>(T(4), T(5)));
		CHECK(v4.x == T(4));
		CHECK(v4.y == T(5));
	}

	TEST_CASE_TEMPLATE("Vector2 assignment operators", T, int32_t, uint32_t, float, double)
	{
		// Copy assignment
		Vector2<T> v1(T(1), T(2));
		Vector2<T> v2;
		v2 = v1;
		CHECK(v2.x == T(1));
		CHECK(v2.y == T(2));

		// Move assignment
		Vector2<T> v3;
		v3 = Vector2<T>(T(3), T(4));
		CHECK(v3.x == T(3));
		CHECK(v3.y == T(4));
	}

	TEST_CASE_TEMPLATE("Vector2 index operator", T, int32_t, uint32_t, float, double)
	{
		Vector2<T> v(T(1), T(2));
		CHECK(v[0] == T(1));
		CHECK(v[1] == T(2));

		// Test modification through index operator
		v[0] = T(3);
		v[1] = T(4);
		CHECK(v.x == T(3));
		CHECK(v.y == T(4));

		// Test const index operator
		const Vector2<T> cv(T(5), T(6));
		CHECK(cv[0] == T(5));
		CHECK(cv[1] == T(6));
	}

	TEST_CASE_TEMPLATE("Vector2 type conversion", T, float, double)
	{
		Vector2<T> vf(T(1.5), T(2.5));
		Vector2<int> vi = static_cast<Vector2<int>>(vf);
		CHECK(vi.x == 1);
		CHECK(vi.y == 2);
	}

	TEST_CASE_TEMPLATE("Vector2 magnitude calculations", T, int32_t, uint32_t, float, double)
	{
		Vector2<T> v(T(3), T(4));
		CHECK(v.SquareMagnitude() == T(25));
		CHECK(v.Magnitude() == T(5));
	}

	TEST_CASE_TEMPLATE("Vector2 normalization", T, float, double)
	{
		Vector2<T> v(T(3), T(4));
		Vector2<T> normalized = v.Normalized();
		CHECK(normalized.x == doctest::Approx(T(0.6)));
		CHECK(normalized.y == doctest::Approx(T(0.8)));
		CHECK(normalized.Magnitude() == doctest::Approx(T(1)));

		v.Normalize();
		CHECK(v.x == doctest::Approx(T(0.6)));
		CHECK(v.y == doctest::Approx(T(0.8)));
		CHECK(v.Magnitude() == doctest::Approx(T(1)));

		// Test edge case: normalizing zero vector
		Vector2<T> zeroVec;
		zeroVec.Normalize();
		CHECK(zeroVec.x == T(0));
		CHECK(zeroVec.y == T(0));
	}

	TEST_CASE_TEMPLATE("Vector2 dot product", T, int32_t, uint32_t, float, double)
	{
		Vector2<T> v1(T(1), T(2));
		Vector2<T> v2(T(3), T(4));
		T dot = v1.Dot(v2);
		CHECK(dot == T(11)); // 1*3 + 2*4 = 11
	}

	TEST_CASE_TEMPLATE("Vector2 static constants", T, int32_t, uint32_t, float, double)
	{
		CHECK(Vector2<T>::Zero.x == T(0));
		CHECK(Vector2<T>::Zero.y == T(0));
		CHECK(Vector2<T>::One.x == T(1));
		CHECK(Vector2<T>::One.y == T(1));
	}

	TEST_CASE_TEMPLATE("Vector2 comparison operators", T, int32_t, uint32_t, float, double)
	{
		Vector2<T> v1(T(1), T(2));
		Vector2<T> v2(T(1), T(2));
		Vector2<T> v3(T(3), T(4));

		CHECK(v1 == v2);
		CHECK(v1 != v3);
		CHECK_FALSE(v1 == v3);
		CHECK_FALSE(v1 != v2);
	}

	TEST_CASE_TEMPLATE("Vector2 scalar operations", T, float, double)
	{
		Vector2<T> v(T(2), T(3));
		T scalar = T(2);

		// Addition
		Vector2<T> vAddScalar = v + scalar;
		CHECK(vAddScalar.x == T(4));
		CHECK(vAddScalar.y == T(5));

		Vector2<T> scalarAddV = scalar + v;
		CHECK(scalarAddV.x == T(4));
		CHECK(scalarAddV.y == T(5));

		// Subtraction
		Vector2<T> vSubScalar = v - scalar;
		CHECK(vSubScalar.x == T(0));
		CHECK(vSubScalar.y == T(1));

		Vector2<T> scalarSubV = scalar - v;
		CHECK(scalarSubV.x == T(0));
		CHECK(scalarSubV.y == T(-1));

		// Multiplication
		Vector2<T> vMulScalar = v * scalar;
		CHECK(vMulScalar.x == T(4));
		CHECK(vMulScalar.y == T(6));

		Vector2<T> scalarMulV = scalar * v;
		CHECK(scalarMulV.x == T(4));
		CHECK(scalarMulV.y == T(6));

		// Division
		Vector2<T> vDivScalar = v / scalar;
		CHECK(vDivScalar.x == T(1));
		CHECK(vDivScalar.y == T(1.5));

		Vector2<T> scalarDivV = scalar / v;
		CHECK(scalarDivV.x == doctest::Approx(T(2) / T(2)));
		CHECK(scalarDivV.y == doctest::Approx(T(2) / T(3)));
	}

	TEST_CASE_TEMPLATE("Vector2 compound assignment with scalars", T, float, double)
	{
		Vector2<T> v(T(2), T(3));
		T scalar = T(2);

		// +=
		Vector2<T> vAddEq = v;
		vAddEq += scalar;
		CHECK(vAddEq.x == T(4));
		CHECK(vAddEq.y == T(5));

		// -=
		Vector2<T> vSubEq = v;
		vSubEq -= scalar;
		CHECK(vSubEq.x == T(0));
		CHECK(vSubEq.y == T(1));

		// *=
		Vector2<T> vMulEq = v;
		vMulEq *= scalar;
		CHECK(vMulEq.x == T(4));
		CHECK(vMulEq.y == T(6));

		// /=
		Vector2<T> vDivEq = v;
		vDivEq /= scalar;
		CHECK(vDivEq.x == T(1));
		CHECK(vDivEq.y == T(1.5));
	}

	TEST_CASE_TEMPLATE("Vector2 vector operations", T, int32_t, uint32_t, float, double)
	{
		Vector2<T> v1(T(2), T(3));
		Vector2<T> v2(T(1), T(2));

		// Addition
		Vector2<T> vAdd = v1 + v2;
		CHECK(vAdd.x == T(3));
		CHECK(vAdd.y == T(5));

		// Subtraction
		Vector2<T> vSub = v1 - v2;
		CHECK(vSub.x == T(1));
		CHECK(vSub.y == T(1));

		// Compound addition
		Vector2<T> vAddEq = v1;
		vAddEq += v2;
		CHECK(vAddEq.x == T(3));
		CHECK(vAddEq.y == T(5));

		// Compound subtraction
		Vector2<T> vSubEq = v1;
		vSubEq -= v2;
		CHECK(vSubEq.x == T(1));
		CHECK(vSubEq.y == T(1));
	}
}