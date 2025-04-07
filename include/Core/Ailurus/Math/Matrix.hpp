#pragma once

#include <cmath>
#include <cstdint>
#include "Vector.hpp"

namespace Ailurus
{
	template <typename ElementType, size_t Row, size_t Col>
	class Matrix
	{
	public:
		constexpr static size_t RowSize = Row;
		constexpr static size_t ColSize = Col;
		constexpr static bool IsSquare = (Row == Col);

		Matrix() = default;

		Matrix(std::initializer_list<std::initializer_list<ElementType>> init)
		{
			size_t rowIndex = 0;
			for (const auto& rowList : init)
			{
				size_t colIndex = 0;
				for (const auto& value : rowList)
					_elements[rowIndex][colIndex++] = value;

				rowIndex++;
			}
		}

		Matrix(const Matrix& other)
		{
			_elements = other._elements;
		}

		Matrix(Matrix&& other) noexcept
		{
			_elements = std::move(other._elements);
		}

		Matrix& operator=(const Matrix& other)
		{
			if (this != &other)
				_elements = other._elements;

			return *this;
		}

		Matrix& operator=(Matrix&& other) noexcept
		{
			if (this != &other)
				_elements = std::move(other._elements);

			return *this;
		}

		Vector<ElementType, Col> GetRow(size_t rowIndex) const
		{
			return _elements[rowIndex];
		}

		void SetRow(size_t rowIndex, const Vector<ElementType, Col>& row)
		{
			_elements[rowIndex] = row;
		}

		Vector<ElementType, Row> GetCol(size_t colIndex) const
		{
			Vector<ElementType, Row> col;
			for (size_t i = 0; i < Row; ++i)
			{
				col[i] = _elements[i][colIndex];
			}
			return col;
		}

		void SetCol(size_t colIndex, const Vector<ElementType, Row>& col)
		{
			for (size_t i = 0; i < Row; ++i)
				_elements[i][colIndex] = col[i];
		}

		template <typename U>
		explicit operator Matrix<U, Row, Col>() const
		{
			Matrix<U, Row, Col> result;
			for (size_t i = 0; i < Row; ++i)
			{
				for (size_t j = 0; j < Col; ++j)
				{
					result[i][j] = static_cast<U>(_elements[i][j]);
				}
			}
			return result;
		}

		Vector<ElementType, Col>& operator[](size_t rowIndex)
		{
			return _elements[rowIndex];
		}

		const Vector<ElementType, Col>& operator[](size_t rowIndex) const
		{
			return _elements[rowIndex];
		}

		Matrix<ElementType, Col, Row> Transpose() const
		{
			Matrix<ElementType, Col, Row> result;
			for (size_t i = 0; i < Row; ++i)
			{
				for (size_t j = 0; j < Col; ++j)
				{
					result[j][i] = _elements[i][j];
				}
			}
			return result;
		}

		ElementType Determinant() const
			requires IsSquare
		{
			if constexpr (Row == 1)
			{
				return _elements[0][0];
			}
			else if constexpr (Row == 2)
			{
				return _elements[0][0] * _elements[1][1] - _elements[0][1] * _elements[1][0];
			}
			else
			{
				ElementType det = 0;
				for (size_t i = 0; i < Col; ++i)
				{
					Matrix<ElementType, Row - 1, Col - 1> subMatrix;
					for (size_t subRow = 1; subRow < Row; ++subRow)
					{
						size_t subColIndex = 0;
						for (size_t subCol = 0; subCol < Col; ++subCol)
						{
							if (subCol == i)
								continue;
							subMatrix[subRow - 1][subColIndex++] = _elements[subRow][subCol];
						}
					}
					det += (i % 2 == 0 ? 1 : -1) * _elements[0][i] * subMatrix.Determinant();
				}
				return det;
			}
		}

		static Matrix Identity()
			requires IsSquare
		{
			Matrix result;
			for (size_t i = 0; i < Row; ++i)
			{
				for (size_t j = 0; j < Col; ++j)
					result[i][j] = (i == j) ? ElementType(1) : ElementType(0);
			}
			return result;
		}

	private:
		Vector<Vector<ElementType, Col>, Row> _elements;
	};

	namespace _internal
	{
		template <typename MatrixElementType, typename ScalarType>
		concept ScalarMultiplication = requires(MatrixElementType t, ScalarType u) {
			{ t* u } -> std::convertible_to<MatrixElementType>;
		};

	} // namespace _internal

	template <typename ElementType, size_t Row, size_t Col>
	bool operator==(const Matrix<ElementType, Row, Col>& left, const Matrix<ElementType, Row, Col>& right)
	{
		for (size_t i = 0; i < Row; ++i)
		{
			for (size_t j = 0; j < Col; ++j)
			{
				if (left[i][j] != right[i][j])
					return false;
			}
		}
		return true;
	}

	template <typename ElementType, size_t Row, size_t Col>
	bool operator!=(const Matrix<ElementType, Row, Col>& left, const Matrix<ElementType, Row, Col>& right)
	{
		return !(left == right);
	}

	// Matrix * Scalar
	template <typename ElementType, size_t Row, size_t Col, typename ScalarType>
	Matrix<ElementType, Row, Col> operator*(
		const Matrix<ElementType, Row, Col>& mat,
		const ScalarType& scalar)
		requires _internal::ScalarMultiplication<ElementType, ScalarType>
	{
		Matrix<ElementType, Row, Col> result;
		for (size_t i = 0; i < Row; ++i)
		{
			for (size_t j = 0; j < Col; ++j)
			{
				result[i][j] = mat[i][j] * scalar;
			}
		}
		return result;
	}

	// Scalar * Matrix
	template <typename ElementType, size_t Row, size_t Col, typename ScalarType>
	Matrix<ElementType, Row, Col> operator*(
		const ScalarType& scalar,
		const Matrix<ElementType, Row, Col>& mat)
		requires _internal::ScalarMultiplication<ElementType, ScalarType>
	{
		return mat * scalar;
	}

	// Matrix *= Scalar
	template <typename ElementType, size_t Row, size_t Col, typename ScalarType>
	Matrix<ElementType, Row, Col>& operator*=(
		Matrix<ElementType, Row, Col>& mat,
		const ScalarType& scalar)
		requires _internal::ScalarMultiplication<ElementType, ScalarType>
	{
		for (size_t i = 0; i < Row; ++i)
		{
			for (size_t j = 0; j < Col; ++j)
			{
				mat[i][j] *= scalar;
			}
		}
		return mat;
	}

	// Matrix * Matrix
	template <typename ElementType, size_t Row, size_t Col, size_t OtherCol>
	Matrix<ElementType, Row, OtherCol> operator*(
		const Matrix<ElementType, Row, Col>& left,
		const Matrix<ElementType, Col, OtherCol>& right)
	{
		Matrix<ElementType, Row, OtherCol> result;
		for (size_t i = 0; i < Row; ++i)
		{
			for (size_t j = 0; j < OtherCol; ++j)
			{
				ElementType sum = ElementType{};
				for (size_t k = 0; k < Col; ++k)
					sum += left[i][k] * right[k][j];

				result[i][j] = sum;
			}
		}
		return result;
	}

	// Matrix + Matrix
	template <typename ElementType, size_t Row, size_t Col>
	Matrix<ElementType, Row, Col> operator+(
		const Matrix<ElementType, Row, Col>& left,
		const Matrix<ElementType, Row, Col>& right)
	{
		Matrix<ElementType, Row, Col> result;
		for (size_t i = 0; i < Row; ++i)
		{
			for (size_t j = 0; j < Col; ++j)
			{
				result[i][j] = left[i][j] + right[i][j];
			}
		}
		return result;
	}

	// Matrix += Matrix
	template <typename ElementType, size_t Row, size_t Col>
	Matrix<ElementType, Row, Col>& operator+=(
		Matrix<ElementType, Row, Col>& left,
		const Matrix<ElementType, Row, Col>& right)
	{
		for (size_t i = 0; i < Row; ++i)
		{
			for (size_t j = 0; j < Col; ++j)
			{
				left[i][j] += right[i][j];
			}
		}
		return left;
	}

	// Matrix - Matrix
	template <typename ElementType, size_t Row, size_t Col>
	Matrix<ElementType, Row, Col> operator-(
		const Matrix<ElementType, Row, Col>& left,
		const Matrix<ElementType, Row, Col>& right)
	{
		Matrix<ElementType, Row, Col> result;
		for (size_t i = 0; i < Row; ++i)
		{
			for (size_t j = 0; j < Col; ++j)
			{
				result[i][j] = left[i][j] - right[i][j];
			}
		}
		return result;
	}

	// Matrix -= Matrix
	template <typename ElementType, size_t Row, size_t Col>
	Matrix<ElementType, Row, Col>& operator-=(
		Matrix<ElementType, Row, Col>& left,
		const Matrix<ElementType, Row, Col>& right)
	{
		for (size_t i = 0; i < Row; ++i)
		{
			for (size_t j = 0; j < Col; ++j)
			{
				left[i][j] -= right[i][j];
			}
		}
		return left;
	}

	// Matrix * Vector
	template <typename ElementType, size_t Row, size_t Col>
	Vector<ElementType, Row> operator*(
		const Matrix<ElementType, Row, Col>& left,
		const Vector<ElementType, Col>& right)
	{
		Vector<ElementType, Row> result;
		for (auto i = 0; i < Row; ++i)
		{
			ElementType sum = ElementType{};
			for (auto j = 0; j < Col; ++j)
			{
				sum += left[i][j] * right[j];
			}

			result[i] = sum;
		}

		return result;
	}

	// Vector * Matrix
	template <typename ElementType, size_t Row, size_t Col>
	Vector<ElementType, Col> operator*(
		const Vector<ElementType, Row>& left,
		const Matrix<ElementType, Row, Col>& right)
	{
		Vector<ElementType, Col> result;
		for (auto i = 0; i < Col; ++i)
		{
			ElementType sum = ElementType{};
			for (auto j = 0; j < Row; ++j)
			{
				sum += left[j] * right[j][i];
			}

			result[i] = sum;
		}

		return result;
	}

	template <typename ElementType>
	using Matrix3x3 = Matrix<ElementType, 3, 3>;

	template <typename ElementType>
	using Matrix4x4 = Matrix<ElementType, 4, 4>;

	using Matrix3x3i = Matrix<int32_t, 3, 3>;
	using Matrix3x3u = Matrix<uint32_t, 3, 3>;
	using Matrix3x3f = Matrix<float, 3, 3>;
	using Matrix3x3d = Matrix<double, 3, 3>;

	using Matrix4x4i = Matrix<int32_t, 4, 4>;
	using Matrix4x4u = Matrix<uint32_t, 4, 4>;
	using Matrix4x4f = Matrix<float, 4, 4>;
	using Matrix4x4d = Matrix<double, 4, 4>;

} // namespace Ailurus