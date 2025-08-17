#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include <glm/glm.hpp>

#include <Ailurus/Math/Vector3.hpp>

using namespace Ailurus;

TEST_SUITE("Vector3")
{
	TEST_CASE_TEMPLATE("Vector3 constructors and accessors", T, int32_t, uint32_t, float, double)
	{
		// Default constructor
		Vector3<T> v1;
		CHECK(v1.x == T(0));
		CHECK(v1.y == T(0));
		CHECK(v1.z == T(0));

		// Value constructor
		Vector3<T> v2(T(1), T(2), T(3));
		CHECK(v2.x == T(1));
		CHECK(v2.y == T(2));
		CHECK(v2.z == T(3));

		// Copy constructor
		Vector3<T> v3 = v2;
		CHECK(v3.x == T(1));
		CHECK(v3.y == T(2));
		CHECK(v3.z == T(3));

		// Test index operator
		Vector3<T> v(T(3), T(4), T(5));
		CHECK(v[0] == T(3));
		CHECK(v[1] == T(4));
		CHECK(v[2] == T(5));

		// Test index operator assignment
		v[0] = T(6);
		v[1] = T(7);
		v[2] = T(8);
		CHECK(v.x == T(6));
		CHECK(v.y == T(7));
		CHECK(v.z == T(8));

		// Test const index operator
		const Vector3<T> cv(T(5), T(6), T(7));
		CHECK(cv[0] == T(5));
		CHECK(cv[1] == T(6));
		CHECK(cv[2] == T(7));
	}

	TEST_CASE_TEMPLATE("Vector3 type conversion", T, float, double)
	{
		Vector3<T> vf(T(1.5), T(2.5), T(3.5));
		Vector3<int> vi = static_cast<Vector3<int>>(vf);
		CHECK(vi.x == 1);
		CHECK(vi.y == 2);
		CHECK(vi.z == 3);
	}

	TEST_CASE_TEMPLATE("Vector3 magnitude calculations", T, int32_t, uint32_t, float, double)
	{
		Vector3<T> v(T(3), T(4), T(12));
		CHECK(v.SquareMagnitude() == T(169));
		CHECK(v.Magnitude() == T(13));
	}

	TEST_CASE_TEMPLATE("Vector3 normalization", T, float, double)
	{
		Vector3<T> v(T(3), T(4), T(0));
		Vector3<T> normalized = v.Normalized();
		CHECK(normalized.x == doctest::Approx(T(0.6)));
		CHECK(normalized.y == doctest::Approx(T(0.8)));
		CHECK(normalized.z == doctest::Approx(T(0)));
		CHECK(normalized.Magnitude() == doctest::Approx(T(1)));

		v.Normalize();
		CHECK(v.x == doctest::Approx(T(0.6)));
		CHECK(v.y == doctest::Approx(T(0.8)));
		CHECK(v.z == doctest::Approx(T(0)));
		CHECK(v.Magnitude() == doctest::Approx(T(1)));

		// Test edge case: normalizing zero vector
		Vector3<T> zeroVec;
		zeroVec.Normalize();
		CHECK(zeroVec.x == T(0));
		CHECK(zeroVec.y == T(0));
		CHECK(zeroVec.z == T(0));
	}

	TEST_CASE_TEMPLATE("Vector3 dot product", T, int32_t, uint32_t, float, double)
	{
		Vector3<T> v1(T(1), T(2), T(3));
		Vector3<T> v2(T(4), T(5), T(6));
		T dot = v1.Dot(v2);
		CHECK(dot == T(32)); // 1*4 + 2*5 + 3*6 = 32
	}

	TEST_CASE_TEMPLATE("Vector3 cross product", T, float, double)
	{
		Vector3<T> v1(T(2), T(3), T(4));
		Vector3<T> v2(T(5), T(6), T(7));

		glm::vec<3, T> glmV1(T(2), T(3), T(4));
		glm::vec<3, T> glmV2(T(5), T(6), T(7));

		// compare with glm, both right handle coordinates
		Vector3<T> crossMy = v1.Cross(v2);
		glm::vec<3, T> glmCross1 = glm::cross(glmV1, glmV2);
		CHECK(crossMy.x == glmCross1.x);
		CHECK(crossMy.y == glmCross1.y);
		CHECK(crossMy.z == glmCross1.z);

		// v1 × v2 = -(v2 × v1)
		Vector3<T> cross1 = v1.Cross(v2);
		Vector3<T> cross2 = v2.Cross(v1);
		Vector3<T> negCross2 = cross2 * T(-1);

		CHECK(cross1.x == negCross2.x);
		CHECK(cross1.y == negCross2.y);
		CHECK(cross1.z == negCross2.z);

		// v × v = 0
		Vector3<T> selfCross = v1.Cross(v1);
		CHECK(selfCross.x == T(0));
		CHECK(selfCross.y == T(0));
		CHECK(selfCross.z == T(0));

		// (s*v1) × v2 = s(v1 × v2)
		T scalar = T(3);
		Vector3<T> scaled = v1 * scalar;
		Vector3<T> crossScaled = scaled.Cross(v2);
		Vector3<T> scaledCross = v1.Cross(v2) * scalar;

		CHECK(crossScaled.x == scaledCross.x);
		CHECK(crossScaled.y == scaledCross.y);
		CHECK(crossScaled.z == scaledCross.z);
	}

	TEST_CASE_TEMPLATE("Vector3 cross product", T, int32_t, uint32_t, float, double)
	{
		Vector3<T> v1(T(1), T(0), T(0));
		Vector3<T> v2(T(0), T(1), T(0));
		Vector3<T> cross = v1.Cross(v2);
		CHECK(cross.x == T(0));
		CHECK(cross.y == T(0));
		CHECK(cross.z == T(1));

		Vector3<T> v3(T(2), T(3), T(4));
		Vector3<T> v4(T(5), T(6), T(7));
		Vector3<T> cross2 = v3.Cross(v4);
		CHECK(cross2.x == T(-3)); // 3*7 - 4*6 = -3
		CHECK(cross2.y == T(6));  // 4*5 - 2*7 = 6
		CHECK(cross2.z == T(-3)); // 2*6 - 3*5 = -3
	}

	TEST_CASE_TEMPLATE("Vector3 static constants", T, int32_t, uint32_t, float, double)
	{
		CHECK(Vector3<T>::Zero.x == T(0));
		CHECK(Vector3<T>::Zero.y == T(0));
		CHECK(Vector3<T>::Zero.z == T(0));
		CHECK(Vector3<T>::One.x == T(1));
		CHECK(Vector3<T>::One.y == T(1));
		CHECK(Vector3<T>::One.z == T(1));
		CHECK(Vector3<T>::Up.x == T(0));
		CHECK(Vector3<T>::Up.y == T(0));
		CHECK(Vector3<T>::Up.z == T(1));
		CHECK(Vector3<T>::Right.x == T(0));
		CHECK(Vector3<T>::Right.y == T(1));
		CHECK(Vector3<T>::Right.z == T(0));
		CHECK(Vector3<T>::Forward.x == T(1));
		CHECK(Vector3<T>::Forward.y == T(0));
		CHECK(Vector3<T>::Forward.z == T(0));
	}

	TEST_CASE_TEMPLATE("Vector3 comparison operators", T, int32_t, uint32_t, float, double)
	{
		Vector3<T> v1(T(1), T(2), T(3));
		Vector3<T> v2(T(1), T(2), T(3));
		Vector3<T> v3(T(4), T(5), T(6));

		CHECK(v1 == v2);
		CHECK(v1 != v3);
		CHECK_FALSE(v1 == v3);
		CHECK_FALSE(v1 != v2);
	}

	TEST_CASE_TEMPLATE("Vector3 scalar operations", T, float, double)
	{
		Vector3<T> v(T(2), T(3), T(4));
		T scalar = T(2);

		// Addition
		Vector3<T> vAddScalar = v + scalar;
		CHECK(vAddScalar.x == T(4));
		CHECK(vAddScalar.y == T(5));
		CHECK(vAddScalar.z == T(6));

		Vector3<T> scalarAddV = scalar + v;
		CHECK(scalarAddV.x == T(4));
		CHECK(scalarAddV.y == T(5));
		CHECK(scalarAddV.z == T(6));

		// Subtraction
		Vector3<T> vSubScalar = v - scalar;
		CHECK(vSubScalar.x == T(0));
		CHECK(vSubScalar.y == T(1));
		CHECK(vSubScalar.z == T(2));

		Vector3<T> scalarSubV = scalar - v;
		CHECK(scalarSubV.x == T(0));
		CHECK(scalarSubV.y == T(-1));
		CHECK(scalarSubV.z == T(-2));

		// Multiplication
		Vector3<T> vMulScalar = v * scalar;
		CHECK(vMulScalar.x == T(4));
		CHECK(vMulScalar.y == T(6));
		CHECK(vMulScalar.z == T(8));

		Vector3<T> scalarMulV = scalar * v;
		CHECK(scalarMulV.x == T(4));
		CHECK(scalarMulV.y == T(6));
		CHECK(scalarMulV.z == T(8));

		// Division
		Vector3<T> vDivScalar = v / scalar;
		CHECK(vDivScalar.x == T(1));
		CHECK(vDivScalar.y == T(1.5));
		CHECK(vDivScalar.z == T(2));

		Vector3<T> scalarDivV = scalar / v;
		CHECK(scalarDivV.x == doctest::Approx(T(2) / T(2)));
		CHECK(scalarDivV.y == doctest::Approx(T(2) / T(3)));
		CHECK(scalarDivV.z == doctest::Approx(T(2) / T(4)));
	}

	TEST_CASE_TEMPLATE("Vector3 compound assignment with scalars", T, float, double)
	{
		Vector3<T> v(T(2), T(3), T(4));
		T scalar = T(2);

		// +=
		Vector3<T> vAddEq = v;
		vAddEq += scalar;
		CHECK(vAddEq.x == T(4));
		CHECK(vAddEq.y == T(5));
		CHECK(vAddEq.z == T(6));

		// -=
		Vector3<T> vSubEq = v;
		vSubEq -= scalar;
		CHECK(vSubEq.x == T(0));
		CHECK(vSubEq.y == T(1));
		CHECK(vSubEq.z == T(2));

		// *=
		Vector3<T> vMulEq = v;
		vMulEq *= scalar;
		CHECK(vMulEq.x == T(4));
		CHECK(vMulEq.y == T(6));
		CHECK(vMulEq.z == T(8));

		// /=
		Vector3<T> vDivEq = v;
		vDivEq /= scalar;
		CHECK(vDivEq.x == T(1));
		CHECK(vDivEq.y == T(1.5));
		CHECK(vDivEq.z == T(2));
	}

	TEST_CASE_TEMPLATE("Vector3 vector operations", T, int32_t, uint32_t, float, double)
	{
		Vector3<T> v1(T(2), T(3), T(4));
		Vector3<T> v2(T(1), T(2), T(3));

		// Addition
		Vector3<T> vAdd = v1 + v2;
		CHECK(vAdd.x == T(3));
		CHECK(vAdd.y == T(5));
		CHECK(vAdd.z == T(7));

		// Subtraction
		Vector3<T> vSub = v1 - v2;
		CHECK(vSub.x == T(1));
		CHECK(vSub.y == T(1));
		CHECK(vSub.z == T(1));

		// Compound addition
		Vector3<T> vAddEq = v1;
		vAddEq += v2;
		CHECK(vAddEq.x == T(3));
		CHECK(vAddEq.y == T(5));
		CHECK(vAddEq.z == T(7));

		// Compound subtraction
		Vector3<T> vSubEq = v1;
		vSubEq -= v2;
		CHECK(vSubEq.x == T(1));
		CHECK(vSubEq.y == T(1));
		CHECK(vSubEq.z == T(1));
	}
}