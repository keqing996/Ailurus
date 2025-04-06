#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <Ailurus/Math/Matrix.hpp>

using namespace Ailurus;

TEST_SUITE("Matrix")
{
	TEST_CASE_TEMPLATE("Default constructor", T, int32_t, uint32_t, float, double)
	{
		Matrix<T, 3, 3> mat;
		for (size_t i = 0; i < 3; ++i)
		{
			for (size_t j = 0; j < 3; ++j)
			{
				CHECK_EQ(mat[i][j], 0);
			}
		}
	}

	TEST_CASE_TEMPLATE("Initializer list constructor", T, int32_t, uint32_t, float, double)
	{
		Matrix<T, 3, 3> mat = { { 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9 } };
		CHECK_EQ(mat[0][0], 1);
		CHECK_EQ(mat[0][1], 2);
		CHECK_EQ(mat[0][2], 3);
		CHECK_EQ(mat[1][0], 4);
		CHECK_EQ(mat[1][1], 5);
		CHECK_EQ(mat[1][2], 6);
		CHECK_EQ(mat[2][0], 7);
		CHECK_EQ(mat[2][1], 8);
		CHECK_EQ(mat[2][2], 9);
	}

	TEST_CASE_TEMPLATE("Copy constructor", T, int32_t, uint32_t, float, double)
	{
		Matrix<T, 3, 3> mat1 = { { 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9 } };
		Matrix<T, 3, 3> mat2(mat1);
		CHECK_EQ(mat2[0][0], 1);
		CHECK_EQ(mat2[0][1], 2);
		CHECK_EQ(mat2[0][2], 3);
		CHECK_EQ(mat2[1][0], 4);
		CHECK_EQ(mat2[1][1], 5);
		CHECK_EQ(mat2[1][2], 6);
		CHECK_EQ(mat2[2][0], 7);
		CHECK_EQ(mat2[2][1], 8);
		CHECK_EQ(mat2[2][2], 9);
	}

	TEST_CASE_TEMPLATE("Move constructor", T, int32_t, uint32_t, float, double)
	{
		Matrix<T, 3, 3> mat1 = { { 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9 } };
		Matrix<T, 3, 3> mat2(std::move(mat1));
		CHECK_EQ(mat2[0][0], 1);
		CHECK_EQ(mat2[0][1], 2);
		CHECK_EQ(mat2[0][2], 3);
		CHECK_EQ(mat2[1][0], 4);
		CHECK_EQ(mat2[1][1], 5);
		CHECK_EQ(mat2[1][2], 6);
		CHECK_EQ(mat2[2][0], 7);
		CHECK_EQ(mat2[2][1], 8);
		CHECK_EQ(mat2[2][2], 9);
	}

	TEST_CASE_TEMPLATE("Copy assignment operator", T, int32_t, uint32_t, float, double)
	{
		Matrix<T, 3, 3> mat1 = { { 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9 } };
		Matrix<T, 3, 3> mat2;
		mat2 = mat1;
		CHECK_EQ(mat2[0][0], 1);
		CHECK_EQ(mat2[0][1], 2);
		CHECK_EQ(mat2[0][2], 3);
		CHECK_EQ(mat2[1][0], 4);
		CHECK_EQ(mat2[1][1], 5);
		CHECK_EQ(mat2[1][2], 6);
		CHECK_EQ(mat2[2][0], 7);
		CHECK_EQ(mat2[2][1], 8);
		CHECK_EQ(mat2[2][2], 9);
	}

	TEST_CASE_TEMPLATE("Move assignment operator", T, int32_t, uint32_t, float, double)
	{
		Matrix<T, 3, 3> mat1 = { { 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9 } };
		Matrix<T, 3, 3> mat2;
		mat2 = std::move(mat1);
		CHECK_EQ(mat2[0][0], 1);
		CHECK_EQ(mat2[0][1], 2);
		CHECK_EQ(mat2[0][2], 3);
		CHECK_EQ(mat2[1][0], 4);
		CHECK_EQ(mat2[1][1], 5);
		CHECK_EQ(mat2[1][2], 6);
		CHECK_EQ(mat2[2][0], 7);
		CHECK_EQ(mat2[2][1], 8);
		CHECK_EQ(mat2[2][2], 9);
	}

	TEST_CASE_TEMPLATE("Matrix equality operator", T, int32_t, uint32_t, float, double)
	{
		Matrix<T, 2, 2> mat1 = { { 1, 2 }, { 3, 4 } };
		Matrix<T, 2, 2> mat2 = { { 1, 2 }, { 3, 4 } };
		Matrix<T, 2, 2> mat3 = { { 5, 6 }, { 7, 8 } };
		CHECK(mat1 == mat2);
		CHECK_FALSE(mat1 == mat3);
	}

	TEST_CASE_TEMPLATE("Matrix inequality operator", T, int32_t, uint32_t, float, double)
	{
		Matrix<T, 2, 2> mat1 = { { 1, 2 }, { 3, 4 } };
		Matrix<T, 2, 2> mat2 = { { 1, 2 }, { 3, 4 } };
		Matrix<T, 2, 2> mat3 = { { 5, 6 }, { 7, 8 } };
		CHECK(mat1 != mat3);
		CHECK_FALSE(mat1 != mat2);
	}

	TEST_CASE_TEMPLATE("Explicit conversion operator", T, int32_t, uint32_t, float, double)
	{
		Matrix<T, 3, 3> mat = { { 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9 } };
		Matrix<int, 3, 3> intMat = static_cast<Matrix<int, 3, 3>>(mat);
		CHECK_EQ(intMat[0][0], 1);
		CHECK_EQ(intMat[0][1], 2);
		CHECK_EQ(intMat[0][2], 3);
		CHECK_EQ(intMat[1][0], 4);
		CHECK_EQ(intMat[1][1], 5);
		CHECK_EQ(intMat[1][2], 6);
		CHECK_EQ(intMat[2][0], 7);
		CHECK_EQ(intMat[2][1], 8);
		CHECK_EQ(intMat[2][2], 9);
	}

	TEST_CASE_TEMPLATE("Matrix multiplication", T, int32_t, uint32_t, float, double)
	{
		{
			Matrix<T, 2, 2> mat1 = { { 1, 2 }, { 3, 4 } };
			Matrix<T, 2, 2> mat2 = { { 5, 6 }, { 7, 8 } };
			Matrix<T, 2, 2> result = mat1 * mat2;
			CHECK_EQ(result[0][0], 19);
			CHECK_EQ(result[0][1], 22);
			CHECK_EQ(result[1][0], 43);
			CHECK_EQ(result[1][1], 50);
		}
		{
			Matrix<T, 2, 3> mat1 = { { 1, 2, 3 }, { 4, 5, 6 } };
			Matrix<T, 3, 2> mat2 = { { 7, 8 }, { 9, 10 }, { 11, 12 } };
			Matrix<T, 2, 2> result = mat1 * mat2;
			CHECK_EQ(result[0][0], 58);
			CHECK_EQ(result[0][1], 64);
			CHECK_EQ(result[1][0], 139);
			CHECK_EQ(result[1][1], 154);
		}
	}

	TEST_CASE_TEMPLATE("Matrix addition", T, int32_t, uint32_t, float, double)
	{
		{
			Matrix<T, 2, 2> mat1 = { { 1, 2 }, { 3, 4 } };
			Matrix<T, 2, 2> mat2 = { { 5, 6 }, { 7, 8 } };
			Matrix<T, 2, 2> result = mat1 + mat2;
			CHECK_EQ(result[0][0], 6);
			CHECK_EQ(result[0][1], 8);
			CHECK_EQ(result[1][0], 10);
			CHECK_EQ(result[1][1], 12);
		}
		{
			Matrix<T, 2, 2> mat1 = { { 1, 2 }, { 3, 4 } };
			Matrix<T, 2, 2> mat2 = { { 5, 6 }, { 7, 8 } };
			mat1 += mat2;
			CHECK_EQ(mat1[0][0], 6);
			CHECK_EQ(mat1[0][1], 8);
			CHECK_EQ(mat1[1][0], 10);
			CHECK_EQ(mat1[1][1], 12);
		}
	}

	TEST_CASE_TEMPLATE("Matrix subtraction", T, int32_t, uint32_t, float, double)
	{
		{
			Matrix<T, 2, 2> mat1 = { { 5, 6 }, { 7, 8 } };
			Matrix<T, 2, 2> mat2 = { { 1, 2 }, { 3, 4 } };
			Matrix<T, 2, 2> result = mat1 - mat2;
			CHECK_EQ(result[0][0], 4);
			CHECK_EQ(result[0][1], 4);
			CHECK_EQ(result[1][0], 4);
			CHECK_EQ(result[1][1], 4);
		}
		{
			Matrix<T, 2, 2> mat1 = { { 5, 6 }, { 7, 8 } };
			Matrix<T, 2, 2> mat2 = { { 1, 2 }, { 3, 4 } };
			mat1 -= mat2;
			CHECK_EQ(mat1[0][0], 4);
			CHECK_EQ(mat1[0][1], 4);
			CHECK_EQ(mat1[1][0], 4);
			CHECK_EQ(mat1[1][1], 4);
		}
	}

	TEST_CASE_TEMPLATE("Matrix scalar multiplication", T, int32_t, uint32_t, float, double)
	{
		{
			Matrix<T, 2, 2> mat = { { 1, 2 }, { 3, 4 } };
			Matrix<T, 2, 2> result = mat * 2;
			CHECK_EQ(result[0][0], 2);
			CHECK_EQ(result[0][1], 4);
			CHECK_EQ(result[1][0], 6);
			CHECK_EQ(result[1][1], 8);
		}
		{
			Matrix<T, 2, 2> mat = { { 1, 2 }, { 3, 4 } };
			mat *= 2;
			CHECK_EQ(mat[0][0], 2);
			CHECK_EQ(mat[0][1], 4);
			CHECK_EQ(mat[1][0], 6);
			CHECK_EQ(mat[1][1], 8);
		}
	}

	TEST_CASE_TEMPLATE("Matrix and Vector multiplication", T, int32_t, uint32_t, float, double)
	{
		{
			Matrix<T, 2, 2> mat = { { 1, 2 }, { 3, 4 } };
			Vector<T, 2> vec = { 5, 6 };
			Vector<T, 2> result = mat * vec;
			CHECK_EQ(result[0], 17);
			CHECK_EQ(result[1], 39);
		}
		{
			Vector<T, 2> vec = { 5, 6 };
			Matrix<T, 2, 2> mat = { { 1, 2 }, { 3, 4 } };
			Vector<T, 2> result = vec * mat;
			CHECK_EQ(result[0], 23);
			CHECK_EQ(result[1], 34);
		}
	}

	TEST_CASE_TEMPLATE("Matrix determinant", T, int32_t, uint32_t, float, double)
	{
		{
			Matrix<T, 2, 2> mat = { { 1, 2 }, { 3, 4 } };
			T det = mat.Determinant();
			CHECK_EQ(det, -2);
		}
		{
			Matrix<T, 3, 3> mat = { { 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9 } };
			T det = mat.Determinant();
			CHECK_EQ(det, 0);
		}
	}

	TEST_CASE_TEMPLATE("Matrix transpose", T, int32_t, uint32_t, float, double)
	{
		{
			Matrix<T, 2, 3> mat = { { 1, 2, 3 }, { 4, 5, 6 } };
			Matrix<T, 3, 2> transposed = mat.Transpose();
			CHECK_EQ(transposed[0][0], 1);
			CHECK_EQ(transposed[0][1], 4);
			CHECK_EQ(transposed[1][0], 2);
			CHECK_EQ(transposed[1][1], 5);
			CHECK_EQ(transposed[2][0], 3);
			CHECK_EQ(transposed[2][1], 6);
		}
		{
			Matrix<T, 3, 3> mat = { { 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9 } };
			Matrix<T, 3, 3> transposed = mat.Transpose();
			CHECK_EQ(transposed[0][0], 1);
			CHECK_EQ(transposed[0][1], 4);
			CHECK_EQ(transposed[0][2], 7);
			CHECK_EQ(transposed[1][0], 2);
			CHECK_EQ(transposed[1][1], 5);
			CHECK_EQ(transposed[1][2], 8);
			CHECK_EQ(transposed[2][0], 3);
			CHECK_EQ(transposed[2][1], 6);
			CHECK_EQ(transposed[2][2], 9);
		}
	}
}