#pragma once

#include <cmath>
#include <initializer_list>
#include "Vector4.hpp"
#include "Matrix3x3.hpp"

namespace Ailurus
{
	template <typename ElementType>
	class Matrix4x4
	{
	public:
		// Default constructor - initialize to zero matrix
		Matrix4x4()
		{
			for (int i = 0; i < 4; ++i)
				for (int j = 0; j < 4; ++j)
					m[i][j] = 0;
		}

		// Constructor from 4 row vectors
		Matrix4x4(const Vector4<ElementType>& row1, const Vector4<ElementType>& row2,
			const Vector4<ElementType>& row3, const Vector4<ElementType>& row4)
		{
			m[0][0] = row1.x;
			m[0][1] = row1.y;
			m[0][2] = row1.z;
			m[0][3] = row1.w;
			m[1][0] = row2.x;
			m[1][1] = row2.y;
			m[1][2] = row2.z;
			m[1][3] = row2.w;
			m[2][0] = row3.x;
			m[2][1] = row3.y;
			m[2][2] = row3.z;
			m[2][3] = row3.w;
			m[3][0] = row4.x;
			m[3][1] = row4.y;
			m[3][2] = row4.z;
			m[3][3] = row4.w;
		}

		// Constructor from initializer list of vectors
		Matrix4x4(std::initializer_list<Vector4<ElementType>> rows)
		{
			int i = 0;
			for (auto vec : rows)
			{
				if (i < 4)
				{
					m[i][0] = vec.x;
					m[i][1] = vec.y;
					m[i][2] = vec.z;
					m[i][3] = vec.w;
					i++;
				}
			}
		}

		// Copy constructor
		Matrix4x4(const Matrix4x4& other)
		{
			for (int i = 0; i < 4; ++i)
				for (int j = 0; j < 4; ++j)
					m[i][j] = other.m[i][j];
		}

		// Move constructor
		Matrix4x4(Matrix4x4&& other) noexcept
		{
			for (int i = 0; i < 4; ++i)
				for (int j = 0; j < 4; ++j)
					m[i][j] = std::move(other.m[i][j]);
		}

		// Copy assignment operator
		Matrix4x4& operator=(const Matrix4x4& other)
		{
			if (this != &other)
			{
				for (int i = 0; i < 4; ++i)
					for (int j = 0; j < 4; ++j)
						m[i][j] = other.m[i][j];
			}
			return *this;
		}

		// Move assignment operator
		Matrix4x4& operator=(Matrix4x4&& other) noexcept
		{
			if (this != &other)
			{
				for (int i = 0; i < 4; ++i)
					for (int j = 0; j < 4; ++j)
						m[i][j] = std::move(other.m[i][j]);
			}
			return *this;
		}

		// Array subscript operator for row access
		ElementType* operator[](std::size_t row)
		{
			return m[row];
		}

		// Const array subscript operator for row access
		const ElementType* operator[](std::size_t row) const
		{
			return m[row];
		}

		// Element access using (row, column) notation
		ElementType& operator()(std::size_t i, std::size_t j)
		{
			return m[i][j];
		}

		// Const element access using (row, column) notation
		const ElementType& operator()(std::size_t i, std::size_t j) const
		{
			return m[i][j];
		}

		// Get a row vector
		Vector4<ElementType> GetRow(std::size_t i) const
		{
			return Vector4<ElementType>(m[i][0], m[i][1], m[i][2], m[i][3]);
		}

		// Get a column vector
		Vector4<ElementType> GetCol(std::size_t j) const
		{
			return Vector4<ElementType>(m[0][j], m[1][j], m[2][j], m[3][j]);
		}

		// Set a row
		void SetRow(std::size_t i, const Vector4<ElementType>& row)
		{
			m[i][0] = row.x;
			m[i][1] = row.y;
			m[i][2] = row.z;
			m[i][3] = row.w;
		}

		// Set a column
		void SetCol(std::size_t j, const Vector4<ElementType>& col)
		{
			m[0][j] = col.x;
			m[1][j] = col.y;
			m[2][j] = col.z;
			m[3][j] = col.w;
		}

		// Explicit type conversion
		template <typename T>
		explicit operator Matrix4x4<T>() const
		{
			return Matrix4x4<T>(
				Vector4<T>{ static_cast<T>(m[0][0]), static_cast<T>(m[0][1]), static_cast<T>(m[0][2]), static_cast<T>(m[0][3]) },
				Vector4<T>{ static_cast<T>(m[1][0]), static_cast<T>(m[1][1]), static_cast<T>(m[1][2]), static_cast<T>(m[1][3]) },
				Vector4<T>{ static_cast<T>(m[2][0]), static_cast<T>(m[2][1]), static_cast<T>(m[2][2]), static_cast<T>(m[2][3]) },
				Vector4<T>{ static_cast<T>(m[3][0]), static_cast<T>(m[3][1]), static_cast<T>(m[3][2]), static_cast<T>(m[3][3]) });
		}

		// Calculate the determinant
		ElementType Determinant() const
		{
			// Using the cofactor expansion along the first row
			ElementType det = 0;

			// Create 3x3 submatrices for cofactor calculation
			for (int i = 0; i < 4; ++i)
			{
				// Get the 3x3 submatrix by removing row 0 and column i
				Matrix3x3<ElementType> submatrix;
				for (int row = 1; row < 4; ++row)
				{
					int subRow = row - 1;
					for (int col = 0; col < 4; ++col)
					{
						if (col != i)
						{
							int subCol = (col < i) ? col : col - 1;
							submatrix(subRow, subCol) = m[row][col];
						}
					}
				}

				// Calculate cofactor: (-1)^(0+i) * m[0][i] * det(submatrix)
				ElementType sign = (i % 2 == 0) ? ElementType(1) : ElementType(-1);
				det += sign * m[0][i] * submatrix.Determinant();
			}

			return det;
		}

		// Transpose the matrix
		Matrix4x4 Transpose() const
		{
			return Matrix4x4(
				Vector4<ElementType>{ m[0][0], m[1][0], m[2][0], m[3][0] },
				Vector4<ElementType>{ m[0][1], m[1][1], m[2][1], m[3][1] },
				Vector4<ElementType>{ m[0][2], m[1][2], m[2][2], m[3][2] },
				Vector4<ElementType>{ m[0][3], m[1][3], m[2][3], m[3][3] });
		}

		// Calculate the adjugate matrix
		Matrix4x4 Adjugate() const
		{
			Matrix4x4 result;

			for (int i = 0; i < 4; ++i)
			{
				for (int j = 0; j < 4; ++j)
				{
					// Create 3x3 minor by excluding row i and column j
					Matrix3x3<ElementType> minor;
					int minorRow = 0;

					for (int row = 0; row < 4; ++row)
					{
						if (row != i)
						{
							int minorCol = 0;
							for (int col = 0; col < 4; ++col)
							{
								if (col != j)
								{
									minor(minorRow, minorCol) = m[row][col];
									minorCol++;
								}
							}
							minorRow++;
						}
					}

					// Cofactor = (-1)^(i+j) * determinant of minor
					ElementType sign = ((i + j) % 2 == 0) ? ElementType(1) : ElementType(-1);
					result(j, i) = sign * minor.Determinant(); // Note: j,i for transpose
				}
			}

			return result;
		}

		// Calculate the inverse of the matrix
		Matrix4x4 Inverse() const
		{
			ElementType det = Determinant();
			if (det == 0)
				return Zero;

			ElementType invDet = ElementType(1) / det;
			return Adjugate() * invDet;
		}

		// Predefined constants
		static const Matrix4x4 Zero;
		static const Matrix4x4 Identity;

	private:
		ElementType m[4][4]; // Matrix elements
	};

	// Initialize static Zero matrix
	template <typename ElementType>
	Matrix4x4<ElementType> const Matrix4x4<ElementType>::Zero(
		Vector4<ElementType>(ElementType(0), ElementType(0), ElementType(0), ElementType(0)),
		Vector4<ElementType>(ElementType(0), ElementType(0), ElementType(0), ElementType(0)),
		Vector4<ElementType>(ElementType(0), ElementType(0), ElementType(0), ElementType(0)),
		Vector4<ElementType>(ElementType(0), ElementType(0), ElementType(0), ElementType(0)));

	// Initialize static Identity matrix
	template <typename ElementType>
	Matrix4x4<ElementType> const Matrix4x4<ElementType>::Identity(
		Vector4<ElementType>( ElementType(1), ElementType(0), ElementType(0), ElementType(0) ),
		Vector4<ElementType>( ElementType(0), ElementType(1), ElementType(0), ElementType(0) ),
		Vector4<ElementType>( ElementType(0), ElementType(0), ElementType(1), ElementType(0) ),
		Vector4<ElementType>( ElementType(0), ElementType(0), ElementType(0), ElementType(1) ));

	// Equality operator
	template <typename ElementType>
	bool operator==(const Matrix4x4<ElementType>& left, const Matrix4x4<ElementType>& right)
	{
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
				if (left(i, j) != right(i, j))
					return false;
		return true;
	}

	// Inequality operator
	template <typename ElementType>
	bool operator!=(const Matrix4x4<ElementType>& left, const Matrix4x4<ElementType>& right)
	{
		return !(left == right);
	}

	// Matrix multiplication
	template <typename ElementType>
	Matrix4x4<ElementType> operator*(const Matrix4x4<ElementType>& left, const Matrix4x4<ElementType>& right)
	{
		Matrix4x4<ElementType> result;
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				result(i, j) = ElementType(0);
				for (int k = 0; k < 4; ++k)
				{
					result(i, j) += left(i, k) * right(k, j);
				}
			}
		}
		return result;
	}

	// Matrix-Vector multiplication
	template <typename ElementType>
	Vector4<ElementType> operator*(const Matrix4x4<ElementType>& matrix, const Vector4<ElementType>& vector)
	{
		return Vector4<ElementType>{
			matrix(0, 0) * vector.x + matrix(0, 1) * vector.y + matrix(0, 2) * vector.z + matrix(0, 3) * vector.w,
			matrix(1, 0) * vector.x + matrix(1, 1) * vector.y + matrix(1, 2) * vector.z + matrix(1, 3) * vector.w,
			matrix(2, 0) * vector.x + matrix(2, 1) * vector.y + matrix(2, 2) * vector.z + matrix(2, 3) * vector.w,
			matrix(3, 0) * vector.x + matrix(3, 1) * vector.y + matrix(3, 2) * vector.z + matrix(3, 3) * vector.w
		};
	}

	// Matrix-Scalar multiplication
	template <typename ElementType, typename ScalarType>
	Matrix4x4<ElementType> operator*(const Matrix4x4<ElementType>& matrix, const ScalarType& scalar)
	{
		Matrix4x4<ElementType> result;
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
				result(i, j) = matrix(i, j) * scalar;
		return result;
	}

	// Scalar-Matrix multiplication
	template <typename ElementType, typename ScalarType>
	Matrix4x4<ElementType> operator*(const ScalarType& scalar, const Matrix4x4<ElementType>& matrix)
	{
		return matrix * scalar;
	}

	// Matrix-Scalar division
	template <typename ElementType, typename ScalarType>
	Matrix4x4<ElementType> operator/(const Matrix4x4<ElementType>& matrix, const ScalarType& scalar)
	{
		Matrix4x4<ElementType> result;
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
				result(i, j) = matrix(i, j) / scalar;
		return result;
	}

	// Scalar-Matrix division (element-wise)
	template <typename ElementType, typename ScalarType>
	Matrix4x4<ElementType> operator/(const ScalarType& scalar, const Matrix4x4<ElementType>& matrix)
	{
		Matrix4x4<ElementType> result;
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
				result(i, j) = scalar / matrix(i, j);
		return result;
	}

	// Compound assignment: matrix-scalar multiplication
	template <typename ElementType, typename ScalarType>
	Matrix4x4<ElementType>& operator*=(Matrix4x4<ElementType>& matrix, const ScalarType& scalar)
	{
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
				matrix(i, j) *= scalar;
		return matrix;
	}

	// Compound assignment: matrix-scalar division
	template <typename ElementType, typename ScalarType>
	Matrix4x4<ElementType>& operator/=(Matrix4x4<ElementType>& matrix, const ScalarType& scalar)
	{
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
				matrix(i, j) /= scalar;
		return matrix;
	}

	// Type aliases
	using Matrix4x4i = Matrix4x4<int>;
	using Matrix4x4f = Matrix4x4<float>;
	using Matrix4x4d = Matrix4x4<double>;
	using Matrix4x4u = Matrix4x4<unsigned int>;
	using Matrix4x4l = Matrix4x4<long long>;
	using Matrix4x4ul = Matrix4x4<unsigned long long>;

} // namespace Ailurus