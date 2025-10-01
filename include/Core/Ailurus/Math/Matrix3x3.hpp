#pragma once

#include <cmath>
#include <initializer_list>
#include <utility>
#include "Vector3.hpp"

namespace Ailurus
{
	template <typename ElementType>
	class Matrix3x3
	{
	public:
		Matrix3x3()
		{
			m[0][0] = m[0][1] = m[0][2] = 0;
			m[1][0] = m[1][1] = m[1][2] = 0;
			m[2][0] = m[2][1] = m[2][2] = 0;
		}

		Matrix3x3(const Vector3<ElementType>& row1, const Vector3<ElementType>& row2, const Vector3<ElementType>& row3)
		{
			m[0][0] = row1.x;
			m[0][1] = row1.y;
			m[0][2] = row1.z;
			m[1][0] = row2.x;
			m[1][1] = row2.y;
			m[1][2] = row2.z;
			m[2][0] = row3.x;
			m[2][1] = row3.y;
			m[2][2] = row3.z;
		}

		Matrix3x3(std::initializer_list<Vector3<ElementType>> rows)
		{
			int i = 0;
			for (auto vec : rows)
			{
				m[i][0] = vec.x;
				m[i][1] = vec.y;
				m[i][2] = vec.z;
				i++;
			}
		}

		Matrix3x3(const Matrix3x3& other)
		{
			for (int i = 0; i < 3; ++i)
				for (int j = 0; j < 3; ++j)
					m[i][j] = other.m[i][j];
		}

		Matrix3x3(Matrix3x3&& other) noexcept
		{
			for (int i = 0; i < 3; ++i)
				for (int j = 0; j < 3; ++j)
					m[i][j] = std::move(other.m[i][j]);
		}

		Matrix3x3& operator=(const Matrix3x3& other)
		{
			if (this != &other)
			{
				for (int i = 0; i < 3; ++i)
					for (int j = 0; j < 3; ++j)
						m[i][j] = other.m[i][j];
			}
			return *this;
		}

		Matrix3x3& operator=(Matrix3x3&& other) noexcept
		{
			if (this != &other)
			{
				for (int i = 0; i < 3; ++i)
					for (int j = 0; j < 3; ++j)
						m[i][j] = std::move(other.m[i][j]);
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

		Vector3<ElementType> GetRow(std::size_t i) const
		{
			return Vector3<ElementType>(m[i][0], m[i][1], m[i][2]);
		}

		Vector3<ElementType> GetCol(std::size_t j) const
		{
			return Vector3<ElementType>(m[0][j], m[1][j], m[2][j]);
		}

		void SetRow(std::size_t i, const Vector3<ElementType>& row)
		{
			m[i][0] = row.x;
			m[i][1] = row.y;
			m[i][2] = row.z;
		}

		void SetCol(std::size_t j, const Vector3<ElementType>& col)
		{
			m[0][j] = col.x;
			m[1][j] = col.y;
			m[2][j] = col.z;
		}

		template <typename T>
		explicit operator Matrix3x3<T>() const
		{
			return Matrix3x3<T>(
				Vector3<T>{ static_cast<T>(m[0][0]), static_cast<T>(m[0][1]), static_cast<T>(m[0][2]) },
				Vector3<T>{ static_cast<T>(m[1][0]), static_cast<T>(m[1][1]), static_cast<T>(m[1][2]) },
				Vector3<T>{ static_cast<T>(m[2][0]), static_cast<T>(m[2][1]), static_cast<T>(m[2][2]) });
		}

		ElementType Determinant() const
		{
			return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
				- m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
				+ m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
		}

		Matrix3x3 Transpose() const
		{
			return Matrix3x3(
				Vector3<ElementType>{ m[0][0], m[1][0], m[2][0] },
				Vector3<ElementType>{ m[0][1], m[1][1], m[2][1] },
				Vector3<ElementType>{ m[0][2], m[1][2], m[2][2] });
		}

		Matrix3x3 Adjugate() const
		{
			return Matrix3x3(
				Vector3<ElementType>{
					m[1][1] * m[2][2] - m[1][2] * m[2][1],
					m[0][2] * m[2][1] - m[0][1] * m[2][2],
					m[0][1] * m[1][2] - m[0][2] * m[1][1] },
				Vector3<ElementType>{
					m[1][2] * m[2][0] - m[1][0] * m[2][2],
					m[0][0] * m[2][2] - m[0][2] * m[2][0],
					m[0][2] * m[1][0] - m[0][0] * m[1][2] },
				Vector3<ElementType>{
					m[1][0] * m[2][1] - m[1][1] * m[2][0],
					m[0][1] * m[2][0] - m[0][0] * m[2][1],
					m[0][0] * m[1][1] - m[0][1] * m[1][0] });
		}

		Matrix3x3 Inverse() const
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

		static const Matrix3x3 Zero;
		static const Matrix3x3 Identity;

	private:
		ElementType m[3][3];
	};

	template <typename ElementType>
	Matrix3x3<ElementType> const Matrix3x3<ElementType>::Zero(
		Vector3<ElementType>{ ElementType(0), ElementType(0), ElementType(0) },
		Vector3<ElementType>{ ElementType(0), ElementType(0), ElementType(0) },
		Vector3<ElementType>{ ElementType(0), ElementType(0), ElementType(0) });

	template <typename ElementType>
	Matrix3x3<ElementType> const Matrix3x3<ElementType>::Identity(
		Vector3<ElementType>{ ElementType(1), ElementType(0), ElementType(0) },
		Vector3<ElementType>{ ElementType(0), ElementType(1), ElementType(0) },
		Vector3<ElementType>{ ElementType(0), ElementType(0), ElementType(1) });

	template <typename ElementType>
	bool operator==(const Matrix3x3<ElementType>& left, const Matrix3x3<ElementType>& right)
	{
		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j)
				if (left(i, j) != right(i, j))
					return false;
		return true;
	}

	template <typename ElementType>
	bool operator!=(const Matrix3x3<ElementType>& left, const Matrix3x3<ElementType>& right)
	{
		return !(left == right);
	}

	template <typename ElementType>
	Matrix3x3<ElementType> operator*(const Matrix3x3<ElementType>& left, const Matrix3x3<ElementType>& right)
	{
		Matrix3x3<ElementType> result;
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				result(i, j) = ElementType(0);
				for (int k = 0; k < 3; ++k)
				{
					result(i, j) += left(i, k) * right(k, j);
				}
			}
		}
		return result;
	}

	template <typename ElementType>
	Vector3<ElementType> operator*(const Matrix3x3<ElementType>& matrix, const Vector3<ElementType>& vector)
	{
		return Vector3<ElementType>{
			matrix(0, 0) * vector.x + matrix(0, 1) * vector.y + matrix(0, 2) * vector.z,
			matrix(1, 0) * vector.x + matrix(1, 1) * vector.y + matrix(1, 2) * vector.z,
			matrix(2, 0) * vector.x + matrix(2, 1) * vector.y + matrix(2, 2) * vector.z
		};
	}

	template <typename ElementType, typename ScalarType>
	Matrix3x3<ElementType> operator*(const Matrix3x3<ElementType>& matrix, const ScalarType& scalar)
	{
		Matrix3x3<ElementType> result;
		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j)
				result(i, j) = matrix(i, j) * scalar;
		return result;
	}

	template <typename ElementType, typename ScalarType>
	Matrix3x3<ElementType> operator*(const ScalarType& scalar, const Matrix3x3<ElementType>& matrix)
	{
		return matrix * scalar;
	}

	template <typename ElementType, typename ScalarType>
	Matrix3x3<ElementType> operator/(const Matrix3x3<ElementType>& matrix, const ScalarType& scalar)
	{
		Matrix3x3<ElementType> result;
		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j)
				result(i, j) = matrix(i, j) / scalar;
		return result;
	}

	template <typename ElementType, typename ScalarType>
	Matrix3x3<ElementType> operator/(const ScalarType& scalar, const Matrix3x3<ElementType>& matrix)
	{
		Matrix3x3<ElementType> result;
		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j)
				result(i, j) = scalar / matrix(i, j);
		return result;
	}

	template <typename ElementType, typename ScalarType>
	Matrix3x3<ElementType>& operator*=(Matrix3x3<ElementType>& matrix, const ScalarType& scalar)
	{
		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j)
				matrix(i, j) *= scalar;
		return matrix;
	}

	template <typename ElementType, typename ScalarType>
	Matrix3x3<ElementType>& operator/=(Matrix3x3<ElementType>& matrix, const ScalarType& scalar)
	{
		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j)
				matrix(i, j) /= scalar;
		return matrix;
	}

	using Matrix3x3i = Matrix3x3<int>;
	using Matrix3x3f = Matrix3x3<float>;
	using Matrix3x3d = Matrix3x3<double>;
	using Matrix3x3u = Matrix3x3<unsigned int>;
	using Matrix3x3l = Matrix3x3<long long>;
	using Matrix3x3ul = Matrix3x3<unsigned long long>;

} // namespace Ailurus