#pragma once

#include <cstddef>
#include <initializer_list>
#include <glm/glm.hpp>
#include <glm/ext/matrix_integer.hpp>
#include "Vector2.hpp"

namespace Ailurus
{
	template <typename ElementType>
	class Matrix2x2
	{
	public:
		using GlmType = glm::mat<2, 2, ElementType>;

		Matrix2x2()
			: _m(ElementType(0)) {}

		explicit Matrix2x2(const GlmType& m)
			: _m(m) {}

		Matrix2x2(const Vector2<ElementType>& row0, const Vector2<ElementType>& row1)
		{
			// Store rows into column-major: _m[col][row]
			_m[0][0] = row0.x; _m[1][0] = row0.y;
			_m[0][1] = row1.x; _m[1][1] = row1.y;
		}

		Matrix2x2(std::initializer_list<Vector2<ElementType>> rows)
		{
			int r = 0;
			for (auto& vec : rows)
			{
				if (r < 2)
				{
					_m[0][r] = vec.x;
					_m[1][r] = vec.y;
					r++;
				}
			}
		}

		Matrix2x2(const Matrix2x2&) = default;
		Matrix2x2(Matrix2x2&&) noexcept = default;
		Matrix2x2& operator=(const Matrix2x2&) = default;
		Matrix2x2& operator=(Matrix2x2&&) noexcept = default;

		// Column access (GLM convention)
		auto operator[](std::size_t col) -> typename GlmType::col_type&
		{
			return _m[static_cast<typename GlmType::length_type>(col)];
		}

		auto operator[](std::size_t col) const -> typename GlmType::col_type const&
		{
			return _m[static_cast<typename GlmType::length_type>(col)];
		}

		// Element access: (row, col)
		ElementType& operator()(std::size_t row, std::size_t col)
		{
			return _m[col][row];
		}

		const ElementType& operator()(std::size_t row, std::size_t col) const
		{
			return _m[col][row];
		}

		Vector2<ElementType> GetRow(std::size_t i) const
		{
			return Vector2<ElementType>(_m[0][i], _m[1][i]);
		}

		Vector2<ElementType> GetCol(std::size_t j) const
		{
			return Vector2<ElementType>(_m[j][0], _m[j][1]);
		}

		void SetRow(std::size_t i, const Vector2<ElementType>& row)
		{
			_m[0][i] = row.x;
			_m[1][i] = row.y;
		}

		void SetCol(std::size_t j, const Vector2<ElementType>& col)
		{
			_m[j][0] = col.x;
			_m[j][1] = col.y;
		}

		template <typename T>
		explicit operator Matrix2x2<T>() const
		{
			return Matrix2x2<T>(
				Vector2<T>{ static_cast<T>(_m[0][0]), static_cast<T>(_m[1][0]) },
				Vector2<T>{ static_cast<T>(_m[0][1]), static_cast<T>(_m[1][1]) });
		}

		ElementType Determinant() const
		{
			return glm::determinant(_m);
		}

		Matrix2x2 Transpose() const
		{
			return Matrix2x2(glm::transpose(_m));
		}

		Matrix2x2 Adjugate() const
		{
			// adj(A) for 2x2: swap diagonal, negate off-diagonal
			// Using (row,col) semantics
			Matrix2x2 result;
			result(0, 0) = (*this)(1, 1);
			result(0, 1) = -(*this)(0, 1);
			result(1, 0) = -(*this)(1, 0);
			result(1, 1) = (*this)(0, 0);
			return result;
		}

		Matrix2x2 Inverse() const
		{
			ElementType det = Determinant();
			if (det == 0)
				return Zero;

			return Matrix2x2(glm::inverse(_m));
		}

		const ElementType* GetDataPtr() const
		{
			return &_m[0][0];
		}

		ElementType* GetDataPtr()
		{
			return &_m[0][0];
		}

		GlmType& Glm() { return _m; }
		const GlmType& Glm() const { return _m; }

		static const Matrix2x2 Zero;
		static const Matrix2x2 Identity;

	private:
		GlmType _m;
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
		return Matrix2x2<ElementType>(left.Glm() * right.Glm());
	}

	// Matrix-vector multiplication
	template <typename ElementType>
	Vector2<ElementType> operator*(const Matrix2x2<ElementType>& matrix, const Vector2<ElementType>& vector)
	{
		auto result = matrix.Glm() * vector.Glm();
		return Vector2<ElementType>(result.x, result.y);
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