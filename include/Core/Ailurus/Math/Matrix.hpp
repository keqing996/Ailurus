#pragma once

#include <cstring>
#include <cmath>
#include <concepts>
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

		Vector<ElementType, Col>& operator[](size_t rowIndex)
		{
			return _elements[rowIndex];
		}

		const Vector<ElementType, Col>& operator[](size_t rowIndex) const
		{
			return _elements[rowIndex];
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
		return mat;
	}

	template <typename ElementType, size_t Row, size_t Col, typename ScalarType>
	Matrix<ElementType, Row, Col> operator*(
		const ScalarType& scalar,
		const Matrix<ElementType, Row, Col>& mat)
		requires _internal::ScalarMultiplication<ElementType, ScalarType>
	{
		return mat * scalar;
	}

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

} // namespace Ailurus