
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include <Ailurus/Math/Quaternion.hpp>

TEST_SUITE("Quaternion")
{
	TEST_CASE("Quaternion default constructor")
	{
		Ailurus::Quaternion<float> q;
		CHECK(q.w == 1.0f);
		CHECK(q.x == 0.0f);
		CHECK(q.y == 0.0f);
		CHECK(q.z == 0.0f);
	}

	TEST_CASE("Quaternion parameterized constructor")
	{
		Ailurus::Quaternion q(1.0f, 2.0f, 3.0f, 4.0f);
		CHECK(q.w == 1.0f);
		CHECK(q.x == 2.0f);
		CHECK(q.y == 3.0f);
		CHECK(q.z == 4.0f);
	}

	TEST_CASE("Quaternion addition")
	{
		Ailurus::Quaternion q1(1.0f, 2.0f, 3.0f, 4.0f);
		Ailurus::Quaternion q2(0.5f, 1.5f, 2.5f, 3.5f);
		Ailurus::Quaternion<float> q3 = q1 + q2;
		CHECK(q3.w == 1.5f);
		CHECK(q3.x == 3.5f);
		CHECK(q3.y == 5.5f);
		CHECK(q3.z == 7.5f);
	}

	TEST_CASE("Quaternion subtraction")
	{
		Ailurus::Quaternion q1(1.0f, 2.0f, 3.0f, 4.0f);
		Ailurus::Quaternion q2(0.5f, 1.5f, 2.5f, 3.5f);
		Ailurus::Quaternion<float> q3 = q1 - q2;
		CHECK(q3.w == 0.5f);
		CHECK(q3.x == 0.5f);
		CHECK(q3.y == 0.5f);
		CHECK(q3.z == 0.5f);
	}

	TEST_CASE("Quaternion multiplication by scalar")
	{
		Ailurus::Quaternion q(1.0f, 2.0f, 3.0f, 4.0f);
		Ailurus::Quaternion<float> q2 = q * 2.0f;
		CHECK(q2.w == 2.0f);
		CHECK(q2.x == 4.0f);
		CHECK(q2.y == 6.0f);
		CHECK(q2.z == 8.0f);
	}

	TEST_CASE("Quaternion multiplication by another quaternion")
	{
		Ailurus::Quaternion q1(1.0f, 2.0f, 3.0f, 4.0f);
		Ailurus::Quaternion q2(0.5f, 1.5f, 2.5f, 3.5f);
		Ailurus::Quaternion<float> q3 = q1 * q2;
		CHECK(q3.w == doctest::Approx(-26.0f));
		CHECK(q3.x == doctest::Approx(6.0f));
		CHECK(q3.y == doctest::Approx(12.0f));
		CHECK(q3.z == doctest::Approx(12.0f));
	}

	TEST_CASE("Quaternion normalization")
	{
		Ailurus::Quaternion q(1.0f, 2.0f, 3.0f, 4.0f);
		q.Normalize();
		float mag = std::sqrt(1.0f + 4.0f + 9.0f + 16.0f);
		CHECK(q.w == doctest::Approx(1.0f / mag));
		CHECK(q.x == doctest::Approx(2.0f / mag));
		CHECK(q.y == doctest::Approx(3.0f / mag));
		CHECK(q.z == doctest::Approx(4.0f / mag));
	}

	TEST_CASE("Quaternion dot product")
	{
		Ailurus::Quaternion q1(1.0f, 2.0f, 3.0f, 4.0f);
		Ailurus::Quaternion q2(0.5f, 1.5f, 2.5f, 3.5f);
		float dot = q1.Dot(q2);
		CHECK(dot == doctest::Approx(30.0f));
	}

	TEST_CASE("Quaternion magnitude")
	{
		Ailurus::Quaternion q(1.0f, 2.0f, 3.0f, 4.0f);
		float mag = q.Magnitude();
		CHECK(mag == doctest::Approx(std::sqrt(30.0f)));
	}

	TEST_CASE("Quaternion conjugate")
	{
		Ailurus::Quaternion q(1.0f, 2.0f, 3.0f, 4.0f);
		Ailurus::Quaternion<float> qc = q.Conjugate();
		CHECK(qc.w == 1.0f);
		CHECK(qc.x == -2.0f);
		CHECK(qc.y == -3.0f);
		CHECK(qc.z == -4.0f);
	}

	TEST_CASE("Quaternion inverse")
	{
		Ailurus::Quaternion q(1.0f, 2.0f, 3.0f, 4.0f);
		Ailurus::Quaternion<float> qi = q.Inverse();
		float norm = q.Magnitude();
		norm = norm * norm;
		CHECK(qi.w == doctest::Approx(1.0f / norm));
		CHECK(qi.x == doctest::Approx(-2.0f / norm));
		CHECK(qi.y == doctest::Approx(-3.0f / norm));
		CHECK(qi.z == doctest::Approx(-4.0f / norm));
	}
}