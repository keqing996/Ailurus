#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <Ailurus/Math/Vector2.hpp>

using namespace Ailurus;

TEST_SUITE("Vector2")
{
	// Test constructors
	TEST_CASE("Vector2 constructors")
	{
		// Default constructor
		Vector2<float> v1;
		CHECK(v1.x == 0.0f);
		CHECK(v1.y == 0.0f);

		// Parameterized constructor
		Vector2<float> v2(2.0f, 3.0f);
		CHECK(v2.x == 2.0f);
		CHECK(v2.y == 3.0f);

		// Copy constructor
		Vector2<float> v3 = v2;
		CHECK(v3.x == 2.0f);
		CHECK(v3.y == 3.0f);

		// Move constructor
		Vector2<float> v4 = std::move(Vector2<float>(4.0f, 5.0f));
		CHECK(v4.x == 4.0f);
		CHECK(v4.y == 5.0f);
	}

	// Test assignment operators
	TEST_CASE("Vector2 assignment operators")
	{
		// Copy assignment
		Vector2<float> v1(1.0f, 2.0f);
		Vector2<float> v2;
		v2 = v1;
		CHECK(v2.x == 1.0f);
		CHECK(v2.y == 2.0f);

		// Move assignment
		Vector2<float> v3;
		v3 = Vector2<float>(3.0f, 4.0f);
		CHECK(v3.x == 3.0f);
		CHECK(v3.y == 4.0f);
	}

	// Test index operator
	TEST_CASE("Vector2 index operator")
	{
		Vector2<float> v(1.0f, 2.0f);
		CHECK(v[0] == 1.0f);
		CHECK(v[1] == 2.0f);

		// Test modification through index operator
		v[0] = 3.0f;
		v[1] = 4.0f;
		CHECK(v.x == 3.0f);
		CHECK(v.y == 4.0f);

		// Test const index operator
		const Vector2<float> cv(5.0f, 6.0f);
		CHECK(cv[0] == 5.0f);
		CHECK(cv[1] == 6.0f);
	}

	// Test type conversion
	TEST_CASE("Vector2 type conversion")
	{
		Vector2<float> vf(1.5f, 2.5f);
		Vector2<int> vi = static_cast<Vector2<int>>(vf);
		CHECK(vi.x == 1);
		CHECK(vi.y == 2);
	}

	// Test magnitude calculations
	TEST_CASE("Vector2 magnitude calculations")
	{
		Vector2<float> v(3.0f, 4.0f);
		CHECK(v.SquareMagnitude() == 25.0f);
		CHECK(v.Magnitude() == 5.0f);
	}

	// Test normalization
	TEST_CASE("Vector2 normalization")
	{
		Vector2<float> v(3.0f, 4.0f);
		Vector2<float> normalized = v.Normalized();
		CHECK(normalized.x == doctest::Approx(0.6f));
		CHECK(normalized.y == doctest::Approx(0.8f));
		CHECK(normalized.Magnitude() == doctest::Approx(1.0f));

		v.Normalize();
		CHECK(v.x == doctest::Approx(0.6f));
		CHECK(v.y == doctest::Approx(0.8f));
		CHECK(v.Magnitude() == doctest::Approx(1.0f));

		// Test edge case: normalizing zero vector
		Vector2<float> zeroVec;
		zeroVec.Normalize();
		CHECK(zeroVec.x == 0.0f);
		CHECK(zeroVec.y == 0.0f);
	}

	// Test dot product
	TEST_CASE("Vector2 dot product")
	{
		Vector2<float> v1(1.0f, 2.0f);
		Vector2<float> v2(3.0f, 4.0f);
		float dot = v1.Dot(v2);
		CHECK(dot == 11.0f); // 1*3 + 2*4 = 11
	}

	// Test static constants
	TEST_CASE("Vector2 static constants")
	{
		CHECK(Vector2<float>::Zero.x == 0.0f);
		CHECK(Vector2<float>::Zero.y == 0.0f);
		CHECK(Vector2<float>::One.x == 1.0f);
		CHECK(Vector2<float>::One.y == 1.0f);
	}

	// Test comparison operators
	TEST_CASE("Vector2 comparison operators")
	{
		Vector2<float> v1(1.0f, 2.0f);
		Vector2<float> v2(1.0f, 2.0f);
		Vector2<float> v3(3.0f, 4.0f);

		CHECK(v1 == v2);
		CHECK(v1 != v3);
		CHECK_FALSE(v1 == v3);
		CHECK_FALSE(v1 != v2);
	}

	// Test vector-scalar operations
	TEST_CASE("Vector2 scalar operations")
	{
		Vector2<float> v(2.0f, 3.0f);
		float scalar = 2.0f;

		// Addition
		Vector2<float> vAddScalar = v + scalar;
		CHECK(vAddScalar.x == 4.0f);
		CHECK(vAddScalar.y == 5.0f);

		Vector2<float> scalarAddV = scalar + v;
		CHECK(scalarAddV.x == 4.0f);
		CHECK(scalarAddV.y == 5.0f);

		// Subtraction
		Vector2<float> vSubScalar = v - scalar;
		CHECK(vSubScalar.x == 0.0f);
		CHECK(vSubScalar.y == 1.0f);

		Vector2<float> scalarSubV = scalar - v;
		CHECK(scalarSubV.x == 0.0f);
		CHECK(scalarSubV.y == -1.0f);

		// Multiplication
		Vector2<float> vMulScalar = v * scalar;
		CHECK(vMulScalar.x == 4.0f);
		CHECK(vMulScalar.y == 6.0f);

		Vector2<float> scalarMulV = scalar * v;
		CHECK(scalarMulV.x == 4.0f);
		CHECK(scalarMulV.y == 6.0f);

		// Division
		Vector2<float> vDivScalar = v / scalar;
		CHECK(vDivScalar.x == 1.0f);
		CHECK(vDivScalar.y == 1.5f);

		Vector2<float> scalarDivV = scalar / v;
		CHECK(scalarDivV.x == doctest::Approx(2.0f / 2.0f));
		CHECK(scalarDivV.y == doctest::Approx(2.0f / 3.0f));
	}

	// Test compound assignment operators with scalars
	TEST_CASE("Vector2 compound assignment with scalars")
	{
		Vector2<float> v(2.0f, 3.0f);
		float scalar = 2.0f;

		// +=
		Vector2<float> vAddEq = v;
		vAddEq += scalar;
		CHECK(vAddEq.x == 4.0f);
		CHECK(vAddEq.y == 5.0f);

		// -=
		Vector2<float> vSubEq = v;
		vSubEq -= scalar;
		CHECK(vSubEq.x == 0.0f);
		CHECK(vSubEq.y == 1.0f);

		// *=
		Vector2<float> vMulEq = v;
		vMulEq *= scalar;
		CHECK(vMulEq.x == 4.0f);
		CHECK(vMulEq.y == 6.0f);

		// /=
		Vector2<float> vDivEq = v;
		vDivEq /= scalar;
		CHECK(vDivEq.x == 1.0f);
		CHECK(vDivEq.y == 1.5f);
	}

	// Test vector-vector operations
	TEST_CASE("Vector2 vector operations")
	{
		Vector2<float> v1(2.0f, 3.0f);
		Vector2<float> v2(1.0f, 2.0f);

		// Addition
		Vector2<float> vAdd = v1 + v2;
		CHECK(vAdd.x == 3.0f);
		CHECK(vAdd.y == 5.0f);

		// Subtraction
		Vector2<float> vSub = v1 - v2;
		CHECK(vSub.x == 1.0f);
		CHECK(vSub.y == 1.0f);

		// Compound addition
		Vector2<float> vAddEq = v1;
		vAddEq += v2;
		CHECK(vAddEq.x == 3.0f);
		CHECK(vAddEq.y == 5.0f);

		// Compound subtraction
		Vector2<float> vSubEq = v1;
		vSubEq -= v2;
		CHECK(vSubEq.x == 1.0f);
		CHECK(vSubEq.y == 1.0f);
	}
}