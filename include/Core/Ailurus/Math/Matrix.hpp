#pragma once

#include <cstring>
#include <cmath>
#include <cstdint>

namespace Ailurus
{
	template <typename ElementType, size_t Row, size_t Col>
	class Matrix
	{
	public:
		constexpr size_t RowSize = Row;
		constexpr size_t ColSize = Col;

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

		std::array<ElementType, Col>& operator[](size_t rowIndex)
		{
			return _elements[rowIndex];
		}

		const std::array<ElementType, Col>& operator[](size_t rowIndex) const
		{
			return _elements[rowIndex];
		}

		template <size_t OtherCol>
		Matrix<ElementType, Row, OtherCol> operator*(const Matrix<ElementType, Col, OtherCol>& other) const
		{
			Matrix<ElementType, Row, OtherCol> result;
			for (size_t i = 0; i < Row; ++i)
			{
				for (size_t j = 0; j < OtherCol; ++j)
				{
					ElementType sum = ElementType{};
					for (size_t k = 0; k < Col; ++k)
						sum += _elements[i][k] * other[k][j];

					result[i][j] = sum;
				}
			}
			return result;
		}

	private:
		std::array<std::array<ElementType, Col>, Row> _elements;
	};
} // namespace Ailurus