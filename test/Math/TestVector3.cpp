#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <Ailurus/Math/Vector3.hpp>

using namespace Ailurus;

TEST_SUITE("Vector3")
{
    // Test constructors and accessors
    TEST_CASE("Vector3 constructors and accessors")
    {
        // Default constructor
        Vector3<float> v1;
        CHECK(v1.x == 0.0f);
        CHECK(v1.y == 0.0f);
        CHECK(v1.z == 0.0f);

        // Value constructor
        Vector3<float> v2(1.0f, 2.0f, 3.0f);
        CHECK(v2.x == 1.0f);
        CHECK(v2.y == 2.0f);
        CHECK(v2.z == 3.0f);

        // Copy constructor
        Vector3<float> v3 = v2;
        CHECK(v3.x == 1.0f);
        CHECK(v3.y == 2.0f);
        CHECK(v3.z == 3.0f);

        // Test index operator
        Vector3<float> v(3.0f, 4.0f, 5.0f);
        CHECK(v[0] == 3.0f);
        CHECK(v[1] == 4.0f);
        CHECK(v[2] == 5.0f);

        // Test index operator assignment
        v[0] = 6.0f;
        v[1] = 7.0f;
        v[2] = 8.0f;
        CHECK(v.x == 6.0f);
        CHECK(v.y == 7.0f);
        CHECK(v.z == 8.0f);

        // Test const index operator
        const Vector3<float> cv(5.0f, 6.0f, 7.0f);
        CHECK(cv[0] == 5.0f);
        CHECK(cv[1] == 6.0f);
        CHECK(cv[2] == 7.0f);
    }

    // Test type conversion
    TEST_CASE("Vector3 type conversion")
    {
        Vector3<float> vf(1.5f, 2.5f, 3.5f);
        Vector3<int> vi = static_cast<Vector3<int>>(vf);
        CHECK(vi.x == 1);
        CHECK(vi.y == 2);
        CHECK(vi.z == 3);
    }

    // Test magnitude calculations
    TEST_CASE("Vector3 magnitude calculations")
    {
        Vector3<float> v(3.0f, 4.0f, 12.0f);
        CHECK(v.SquareMagnitude() == 169.0f);
        CHECK(v.Magnitude() == 13.0f);
    }

    // Test normalization
    TEST_CASE("Vector3 normalization")
    {
        Vector3<float> v(3.0f, 4.0f, 0.0f);
        Vector3<float> normalized = v.Normalized();
        CHECK(normalized.x == doctest::Approx(0.6f));
        CHECK(normalized.y == doctest::Approx(0.8f));
        CHECK(normalized.z == doctest::Approx(0.0f));
        CHECK(normalized.Magnitude() == doctest::Approx(1.0f));

        v.Normalize();
        CHECK(v.x == doctest::Approx(0.6f));
        CHECK(v.y == doctest::Approx(0.8f));
        CHECK(v.z == doctest::Approx(0.0f));
        CHECK(v.Magnitude() == doctest::Approx(1.0f));

        // Test edge case: normalizing zero vector
        Vector3<float> zeroVec;
        zeroVec.Normalize();
        CHECK(zeroVec.x == 0.0f);
        CHECK(zeroVec.y == 0.0f);
        CHECK(zeroVec.z == 0.0f);
    }

    // Test dot product
    TEST_CASE("Vector3 dot product")
    {
        Vector3<float> v1(1.0f, 2.0f, 3.0f);
        Vector3<float> v2(4.0f, 5.0f, 6.0f);
        float dot = v1.Dot(v2);
        CHECK(dot == 32.0f); // 1*4 + 2*5 + 3*6 = 32
    }

    // Test cross product (Vector3 specific)
    TEST_CASE("Vector3 cross product")
    {
        Vector3<float> v1(1.0f, 0.0f, 0.0f);
        Vector3<float> v2(0.0f, 1.0f, 0.0f);
        Vector3<float> cross = v1.Cross(v2);
        CHECK(cross.x == 0.0f);
        CHECK(cross.y == 0.0f);
        CHECK(cross.z == 1.0f);

        Vector3<float> v3(2.0f, 3.0f, 4.0f);
        Vector3<float> v4(5.0f, 6.0f, 7.0f);
        Vector3<float> cross2 = v3.Cross(v4);
        CHECK(cross2.x == -3.0f);  // 3*7 - 4*6 = -3
        CHECK(cross2.y == 6.0f);   // 4*5 - 2*7 = 6
        CHECK(cross2.z == -3.0f);  // 2*6 - 3*5 = -3
    }

    // Test static constants
    TEST_CASE("Vector3 static constants")
    {
        CHECK(Vector3<float>::Zero.x == 0.0f);
        CHECK(Vector3<float>::Zero.y == 0.0f);
        CHECK(Vector3<float>::Zero.z == 0.0f);
        CHECK(Vector3<float>::One.x == 1.0f);
        CHECK(Vector3<float>::One.y == 1.0f);
        CHECK(Vector3<float>::One.z == 1.0f);
        CHECK(Vector3<float>::Up.x == 0.0f);
        CHECK(Vector3<float>::Up.y == 1.0f);
        CHECK(Vector3<float>::Up.z == 0.0f);
        CHECK(Vector3<float>::Right.x == 1.0f);
        CHECK(Vector3<float>::Right.y == 0.0f);
        CHECK(Vector3<float>::Right.z == 0.0f);
        CHECK(Vector3<float>::Forward.x == 0.0f);
        CHECK(Vector3<float>::Forward.y == 0.0f);
        CHECK(Vector3<float>::Forward.z == 1.0f);
    }

    // Test comparison operators
    TEST_CASE("Vector3 comparison operators")
    {
        Vector3<float> v1(1.0f, 2.0f, 3.0f);
        Vector3<float> v2(1.0f, 2.0f, 3.0f);
        Vector3<float> v3(4.0f, 5.0f, 6.0f);

        CHECK(v1 == v2);
        CHECK(v1 != v3);
        CHECK_FALSE(v1 == v3);
        CHECK_FALSE(v1 != v2);
    }

    // Test vector-scalar operations
    TEST_CASE("Vector3 scalar operations")
    {
        Vector3<float> v(2.0f, 3.0f, 4.0f);
        float scalar = 2.0f;

        // Addition
        Vector3<float> vAddScalar = v + scalar;
        CHECK(vAddScalar.x == 4.0f);
        CHECK(vAddScalar.y == 5.0f);
        CHECK(vAddScalar.z == 6.0f);

        Vector3<float> scalarAddV = scalar + v;
        CHECK(scalarAddV.x == 4.0f);
        CHECK(scalarAddV.y == 5.0f);
        CHECK(scalarAddV.z == 6.0f);

        // Subtraction
        Vector3<float> vSubScalar = v - scalar;
        CHECK(vSubScalar.x == 0.0f);
        CHECK(vSubScalar.y == 1.0f);
        CHECK(vSubScalar.z == 2.0f);

        Vector3<float> scalarSubV = scalar - v;
        CHECK(scalarSubV.x == 0.0f);
        CHECK(scalarSubV.y == -1.0f);
        CHECK(scalarSubV.z == -2.0f);

        // Multiplication
        Vector3<float> vMulScalar = v * scalar;
        CHECK(vMulScalar.x == 4.0f);
        CHECK(vMulScalar.y == 6.0f);
        CHECK(vMulScalar.z == 8.0f);

        Vector3<float> scalarMulV = scalar * v;
        CHECK(scalarMulV.x == 4.0f);
        CHECK(scalarMulV.y == 6.0f);
        CHECK(scalarMulV.z == 8.0f);

        // Division
        Vector3<float> vDivScalar = v / scalar;
        CHECK(vDivScalar.x == 1.0f);
        CHECK(vDivScalar.y == 1.5f);
        CHECK(vDivScalar.z == 2.0f);

        Vector3<float> scalarDivV = scalar / v;
        CHECK(scalarDivV.x == doctest::Approx(2.0f / 2.0f));
        CHECK(scalarDivV.y == doctest::Approx(2.0f / 3.0f));
        CHECK(scalarDivV.z == doctest::Approx(2.0f / 4.0f));
    }

    // Test compound assignment operators with scalars
    TEST_CASE("Vector3 compound assignment with scalars")
    {
        Vector3<float> v(2.0f, 3.0f, 4.0f);
        float scalar = 2.0f;

        // +=
        Vector3<float> vAddEq = v;
        vAddEq += scalar;
        CHECK(vAddEq.x == 4.0f);
        CHECK(vAddEq.y == 5.0f);
        CHECK(vAddEq.z == 6.0f);

        // -=
        Vector3<float> vSubEq = v;
        vSubEq -= scalar;
        CHECK(vSubEq.x == 0.0f);
        CHECK(vSubEq.y == 1.0f);
        CHECK(vSubEq.z == 2.0f);

        // *=
        Vector3<float> vMulEq = v;
        vMulEq *= scalar;
        CHECK(vMulEq.x == 4.0f);
        CHECK(vMulEq.y == 6.0f);
        CHECK(vMulEq.z == 8.0f);

        // /=
        Vector3<float> vDivEq = v;
        vDivEq /= scalar;
        CHECK(vDivEq.x == 1.0f);
        CHECK(vDivEq.y == 1.5f);
        CHECK(vDivEq.z == 2.0f);
    }

    // Test vector-vector operations
    TEST_CASE("Vector3 vector operations")
    {
        Vector3<float> v1(2.0f, 3.0f, 4.0f);
        Vector3<float> v2(1.0f, 2.0f, 3.0f);

        // Addition
        Vector3<float> vAdd = v1 + v2;
        CHECK(vAdd.x == 3.0f);
        CHECK(vAdd.y == 5.0f);
        CHECK(vAdd.z == 7.0f);

        // Subtraction
        Vector3<float> vSub = v1 - v2;
        CHECK(vSub.x == 1.0f);
        CHECK(vSub.y == 1.0f);
        CHECK(vSub.z == 1.0f);

        // Compound addition
        Vector3<float> vAddEq = v1;
        vAddEq += v2;
        CHECK(vAddEq.x == 3.0f);
        CHECK(vAddEq.y == 5.0f);
        CHECK(vAddEq.z == 7.0f);

        // Compound subtraction
        Vector3<float> vSubEq = v1;
        vSubEq -= v2;
        CHECK(vSubEq.x == 1.0f);
        CHECK(vSubEq.y == 1.0f);
        CHECK(vSubEq.z == 1.0f);
    }
}