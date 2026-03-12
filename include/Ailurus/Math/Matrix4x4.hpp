#pragma once

#include <cstddef>
#include <initializer_list>
#include <glm/glm.hpp>
#include <glm/ext/matrix_integer.hpp>
#include "Vector4.hpp"
#include "Matrix3x3.hpp"

namespace Ailurus
{
	template <typename ElementType>
	class Matrix4x4
	{
	public:
		using GlmType = glm::mat<4, 4, ElementType>;

		// Default constructor - zero matrix
		Matrix4x4()
			: _m(ElementType(0)) {}

		// Construct from GLM matrix directly
		explicit Matrix4x4(const GlmType& m)
			: _m(m) {}

		// Constructor from 4 row vectors (stores column-major internally)
		Matrix4x4(const Vector4<ElementType>& row0, const Vector4<ElementType>& row1,
			const Vector4<ElementType>& row2, const Vector4<ElementType>& row3)
		{
			// _m[col][row]
			_m[0][0] = row0.x; _m[1][0] = row0.y; _m[2][0] = row0.z; _m[3][0] = row0.w;
			_m[0][1] = row1.x; _m[1][1] = row1.y; _m[2][1] = row1.z; _m[3][1] = row1.w;
			_m[0][2] = row2.x; _m[1][2] = row2.y; _m[2][2] = row2.z; _m[3][2] = row2.w;
			_m[0][3] = row3.x; _m[1][3] = row3.y; _m[2][3] = row3.z; _m[3][3] = row3.w;
		}

		// Constructor from initializer list of row vectors
		Matrix4x4(std::initializer_list<Vector4<ElementType>> rows)
		{
			int r = 0;
			for (auto& vec : rows)
			{
				if (r < 4)
				{
					_m[0][r] = vec.x;
					_m[1][r] = vec.y;
					_m[2][r] = vec.z;
					_m[3][r] = vec.w;
					r++;
				}
			}
		}

		Matrix4x4(const Matrix4x4&) = default;
		Matrix4x4(Matrix4x4&&) noexcept = default;
		Matrix4x4& operator=(const Matrix4x4&) = default;
		Matrix4x4& operator=(Matrix4x4&&) noexcept = default;

		// Column access (GLM convention: operator[](col) returns column vector)
		auto operator[](std::size_t col) -> typename GlmType::col_type&
		{
			return _m[static_cast<typename GlmType::length_type>(col)];
		}

		auto operator[](std::size_t col) const -> typename GlmType::col_type const&
		{
			return _m[static_cast<typename GlmType::length_type>(col)];
		}

		// Element access using (row, column) notation
		ElementType& operator()(std::size_t row, std::size_t col)
		{
			return _m[col][row];
		}

		const ElementType& operator()(std::size_t row, std::size_t col) const
		{
			return _m[col][row];
		}

		// Get a row vector
		Vector4<ElementType> GetRow(std::size_t i) const
		{
			return Vector4<ElementType>(_m[0][i], _m[1][i], _m[2][i], _m[3][i]);
		}

		// Get a column vector
		Vector4<ElementType> GetCol(std::size_t j) const
		{
			return Vector4<ElementType>(_m[j][0], _m[j][1], _m[j][2], _m[j][3]);
		}

		// Set a row
		void SetRow(std::size_t i, const Vector4<ElementType>& row)
		{
			_m[0][i] = row.x;
			_m[1][i] = row.y;
			_m[2][i] = row.z;
			_m[3][i] = row.w;
		}

		// Set a column
		void SetCol(std::size_t j, const Vector4<ElementType>& col)
		{
			_m[j][0] = col.x;
			_m[j][1] = col.y;
			_m[j][2] = col.z;
			_m[j][3] = col.w;
		}

		// Explicit type conversion
		template <typename T>
		explicit operator Matrix4x4<T>() const
		{
			Matrix4x4<T> result;
			for (int i = 0; i < 4; ++i)
				for (int j = 0; j < 4; ++j)
					result(i, j) = static_cast<T>((*this)(i, j));
			return result;
		}

		// Calculate the determinant
		ElementType Determinant() const
		{
			return glm::determinant(_m);
		}

		// Transpose the matrix
		Matrix4x4 Transpose() const
		{
			return Matrix4x4(glm::transpose(_m));
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
									minor(minorRow, minorCol) = (*this)(row, col);
									minorCol++;
								}
							}
							minorRow++;
						}
					}

					// Cofactor: (-1)^(i+j) * det(minor), transposed for adjugate
					ElementType sign = ((i + j) % 2 == 0) ? ElementType(1) : ElementType(-1);
					result(j, i) = sign * minor.Determinant();
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

			return Matrix4x4(glm::inverse(_m));
		}

		// Column-major data pointer
		const ElementType* GetDataPtr() const
		{
			return &_m[0][0];
		}

		ElementType* GetDataPtr()
		{
			return &_m[0][0];
		}

		// GLM interop
		GlmType& Glm() { return _m; }
		const GlmType& Glm() const { return _m; }

		// Predefined constants
		static const Matrix4x4 Zero;
		static const Matrix4x4 Identity;

	private:
		GlmType _m;
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

	// Matrix multiplication (delegated to GLM)
	template <typename ElementType>
	Matrix4x4<ElementType> operator*(const Matrix4x4<ElementType>& left, const Matrix4x4<ElementType>& right)
	{
		return Matrix4x4<ElementType>(left.Glm() * right.Glm());
	}

	// Matrix-Vector multiplication (delegated to GLM)
	template <typename ElementType>
	Vector4<ElementType> operator*(const Matrix4x4<ElementType>& matrix, const Vector4<ElementType>& vector)
	{
		auto result = matrix.Glm() * vector.Glm();
		return Vector4<ElementType>(result.x, result.y, result.z, result.w);
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
