#pragma once

#include <cmath>
#include <initializer_list>
#include "Vector2.hpp"

namespace Ailurus
{
	template <typename ElementType>
	class Matrix2x2
	{
	public:
		Matrix2x2()
		{
			m[0][0] = m[0][1] = m[1][0] = m[1][1] = 0;
		}

		Matrix2x2(const Vector2<ElementType>& row1, const Vector2<ElementType>& row2)
		{
			m[0][0] = row1.x;
			m[0][1] = row1.y;
			m[1][0] = row2.x;
			m[1][1] = row2.y;
		}

		Matrix2x2(std::initializer_list<Vector2<ElementType>> rows)
		{
			int i = 0;
			for (auto vec : rows)
			{
				m[i][0] = vec.x;
				m[i][1] = vec.y;
				i++;
			}
		}

		Matrix2x2(const Matrix2x2& other)
		{
			m[0][0] = other.m[0][0];
			m[0][1] = other.m[0][1];
			m[1][0] = other.m[1][0];
			m[1][1] = other.m[1][1];
		}

		Matrix2x2(Matrix2x2&& other) noexcept
		{
			m[0][0] = std::move(other.m[0][0]);
			m[0][1] = std::move(other.m[0][1]);
			m[1][0] = std::move(other.m[1][0]);
			m[1][1] = std::move(other.m[1][1]);
		}

		Matrix2x2& operator=(const Matrix2x2& other)
		{
			if (this != &other)
			{
				m[0][0] = other.m[0][0];
				m[0][1] = other.m[0][1];
				m[1][0] = other.m[1][0];
				m[1][1] = other.m[1][1];
			}
			return *this;
		}

		Matrix2x2& operator=(Matrix2x2&& other) noexcept
		{
			if (this != &other)
			{
				m[0][0] = std::move(other.m[0][0]);
				m[0][1] = std::move(other.m[0][1]);
				m[1][0] = std::move(other.m[1][0]);
				m[1][1] = std::move(other.m[1][1]);
			}
			return *this;
		}

		ElementType* operator[](std::size_t row)
		{
			return m[row];
		}

		const ElementType* operator[](std::size_t row) const
		{
			return m[row];
		}

		ElementType& operator()(std::size_t i, std::size_t j)
		{
			return m[i][j];
		}

		const ElementType& operator()(std::size_t i, std::size_t j) const
		{
			return m[i][j];
		}

		Vector2<ElementType> GetRow(std::size_t i) const
		{
			return Vector2<ElementType>(m[i][0], m[i][1]);
		}

		Vector2<ElementType> GetCol(std::size_t j) const
		{
			return Vector2<ElementType>(m[0][j], m[1][j]);
		}

		void SetRow(std::size_t i, const Vector2<ElementType>& row)
		{
			m[i][0] = row.x;
			m[i][1] = row.y;
		}

		void SetCol(std::size_t j, const Vector2<ElementType>& col)
		{
			m[0][j] = col.x;
			m[1][j] = col.y;
		}

		template <typename T>
		explicit operator Matrix2x2<T>() const
		{
			return Matrix2x2<T>(
				Vector2<T>{ static_cast<T>(m[0][0]), static_cast<T>(m[0][1]) },
				Vector2<T>{ static_cast<T>(m[1][0]), static_cast<T>(m[1][1]) });
		}

		ElementType Determinant() const
		{
			return m[0][0] * m[1][1] - m[0][1] * m[1][0];
		}

		Matrix2x2 Transpose() const
		{
			return Matrix2x2(
				Vector2<ElementType>{ m[0][0], m[1][0] },
				Vector2<ElementType>{ m[0][1], m[1][1] });
		}

		Matrix2x2 Adjugate() const
		{
			return Matrix2x2(
				Vector2<ElementType>{ m[1][1], -m[0][1] },
				Vector2<ElementType>{ -m[1][0], m[0][0] });
		}

		Matrix2x2 Inverse() const
		{
			ElementType det = Determinant();
			if (det == 0)
				return Zero;

			ElementType invDet = 1 / det;
			return Adjugate() * invDet;
		}

		const ElementType* GetRowMajorDataPtr() const
		{
			return &m[0][0];
		}

		ElementType* GetRowMajorDataPtr()
		{
			return &m[0][0];
		}

		static const Matrix2x2 Zero;
		static const Matrix2x2 Identity;

	private:
		ElementType m[2][2];
	};

	// Static constant definitions
	template <typename ElementType>
	Matrix2x2<ElementType> const Matrix2x2<ElementType>::Zero(
		Vector2<ElementType>{ ElementType(0), ElementType(0) },
		Vector2<ElementType>{ ElementType(0), ElementType(0) });

	template <typename ElementType>
	Matrix2x2<ElementType> const Matrix2x2<ElementType>::Identity(
		Vector2<ElementType>{ ElementType(1), ElementType(0) },
		Vector2<ElementType>{ ElementType(0), ElementType(1) });

	// Equality operator
	template <typename ElementType>
	bool operator==(const Matrix2x2<ElementType>& left, const Matrix2x2<ElementType>& right)
	{
		return left(0, 0) == right(0, 0) && left(0, 1) == right(0, 1)
			&& left(1, 0) == right(1, 0) && left(1, 1) == right(1, 1);
	}

	template <typename ElementType>
	bool operator!=(const Matrix2x2<ElementType>& left, const Matrix2x2<ElementType>& right)
	{
		return !(left == right);
	}

	// Matrix multiplication
	template <typename ElementType>
	Matrix2x2<ElementType> operator*(const Matrix2x2<ElementType>& left, const Matrix2x2<ElementType>& right)
	{
		Matrix2x2<ElementType> result;
		result(0, 0) = left(0, 0) * right(0, 0) + left(0, 1) * right(1, 0);
		result(0, 1) = left(0, 0) * right(0, 1) + left(0, 1) * right(1, 1);
		result(1, 0) = left(1, 0) * right(0, 0) + left(1, 1) * right(1, 0);
		result(1, 1) = left(1, 0) * right(0, 1) + left(1, 1) * right(1, 1);
		return result;
	}

	// Matrix-vector multiplication
	template <typename ElementType>
	Vector2<ElementType> operator*(const Matrix2x2<ElementType>& matrix, const Vector2<ElementType>& vector)
	{
		return Vector2<ElementType>{
			matrix(0, 0) * vector.x + matrix(0, 1) * vector.y,
			matrix(1, 0) * vector.x + matrix(1, 1) * vector.y
		};
	}

	// Matrix2x2 * Scalar
	template <typename ElementType, typename ScalarType>
	Matrix2x2<ElementType> operator*(const Matrix2x2<ElementType>& matrix, const ScalarType& scalar)
	{
		Matrix2x2<ElementType> result;
		result(0, 0) = matrix(0, 0) * scalar;
		result(0, 1) = matrix(0, 1) * scalar;
		result(1, 0) = matrix(1, 0) * scalar;
		result(1, 1) = matrix(1, 1) * scalar;
		return result;
	}

	// Scalar * Matrix2x2
	template <typename ElementType, typename ScalarType>
	Matrix2x2<ElementType> operator*(const ScalarType& scalar, const Matrix2x2<ElementType>& matrix)
	{
		return matrix * scalar;
	}

	// Matrix2x2 / Scalar
	template <typename ElementType, typename ScalarType>
	Matrix2x2<ElementType> operator/(const Matrix2x2<ElementType>& matrix, const ScalarType& scalar)
	{
		return Matrix2x2<ElementType>(
			Vector2<ElementType>{ matrix(0, 0) / scalar, matrix(0, 1) / scalar },
			Vector2<ElementType>{ matrix(1, 0) / scalar, matrix(1, 1) / scalar });
	}

	// Scalar / Matrix2x2 (element-wise division)
	template <typename ElementType, typename ScalarType>
	Matrix2x2<ElementType> operator/(const ScalarType& scalar, const Matrix2x2<ElementType>& matrix)
	{
		return Matrix2x2<ElementType>(
			Vector2<ElementType>{ scalar / matrix(0, 0), scalar / matrix(0, 1) },
			Vector2<ElementType>{ scalar / matrix(1, 0), scalar / matrix(1, 1) });
	}

	// Matrix2x2 *= Scalar
	template <typename ElementType, typename ScalarType>
	Matrix2x2<ElementType>& operator*=(Matrix2x2<ElementType>& matrix, const ScalarType& scalar)
	{
		matrix(0, 0) *= scalar;
		matrix(0, 1) *= scalar;
		matrix(1, 0) *= scalar;
		matrix(1, 1) *= scalar;
		return matrix;
	}

	// Matrix2x2 /= Scalar
	template <typename ElementType, typename ScalarType>
	Matrix2x2<ElementType>& operator/=(Matrix2x2<ElementType>& matrix, const ScalarType& scalar)
	{
		matrix(0, 0) /= scalar;
		matrix(0, 1) /= scalar;
		matrix(1, 0) /= scalar;
		matrix(1, 1) /= scalar;
		return matrix;
	}

	// Common type aliases
	using Matrix2x2i = Matrix2x2<int>;
	using Matrix2x2f = Matrix2x2<float>;
	using Matrix2x2d = Matrix2x2<double>;
	using Matrix2x2u = Matrix2x2<unsigned int>;
	using Matrix2x2l = Matrix2x2<long long>;
	using Matrix2x2ul = Matrix2x2<unsigned long long>;

} // namespace Ailurus