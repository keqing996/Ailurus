#pragma once

#include <cstddef>
#include <initializer_list>
#include <glm/glm.hpp>
#include <glm/ext/matrix_integer.hpp>
#include "Vector3.hpp"

namespace Ailurus
{
	template <typename ElementType>
	class Matrix3x3
	{
	public:
		using GlmType = glm::mat<3, 3, ElementType>;

		Matrix3x3()
			: _m(ElementType(0)) {}

		explicit Matrix3x3(const GlmType& m)
			: _m(m) {}

		Matrix3x3(const Vector3<ElementType>& row0, const Vector3<ElementType>& row1, const Vector3<ElementType>& row2)
		{
			// Store rows into column-major: _m[col][row]
			_m[0][0] = row0.x; _m[1][0] = row0.y; _m[2][0] = row0.z;
			_m[0][1] = row1.x; _m[1][1] = row1.y; _m[2][1] = row1.z;
			_m[0][2] = row2.x; _m[1][2] = row2.y; _m[2][2] = row2.z;
		}

		Matrix3x3(std::initializer_list<Vector3<ElementType>> rows)
		{
			int r = 0;
			for (auto& vec : rows)
			{
				if (r < 3)
				{
					_m[0][r] = vec.x;
					_m[1][r] = vec.y;
					_m[2][r] = vec.z;
					r++;
				}
			}
		}

		Matrix3x3(const Matrix3x3&) = default;
		Matrix3x3(Matrix3x3&&) noexcept = default;
		Matrix3x3& operator=(const Matrix3x3&) = default;
		Matrix3x3& operator=(Matrix3x3&&) noexcept = default;

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

		Vector3<ElementType> GetRow(std::size_t i) const
		{
			return Vector3<ElementType>(_m[0][i], _m[1][i], _m[2][i]);
		}

		Vector3<ElementType> GetCol(std::size_t j) const
		{
			return Vector3<ElementType>(_m[j][0], _m[j][1], _m[j][2]);
		}

		void SetRow(std::size_t i, const Vector3<ElementType>& row)
		{
			_m[0][i] = row.x;
			_m[1][i] = row.y;
			_m[2][i] = row.z;
		}

		void SetCol(std::size_t j, const Vector3<ElementType>& col)
		{
			_m[j][0] = col.x;
			_m[j][1] = col.y;
			_m[j][2] = col.z;
		}

		template <typename T>
		explicit operator Matrix3x3<T>() const
		{
			return Matrix3x3<T>(
				Vector3<T>{ static_cast<T>((*this)(0,0)), static_cast<T>((*this)(0,1)), static_cast<T>((*this)(0,2)) },
				Vector3<T>{ static_cast<T>((*this)(1,0)), static_cast<T>((*this)(1,1)), static_cast<T>((*this)(1,2)) },
				Vector3<T>{ static_cast<T>((*this)(2,0)), static_cast<T>((*this)(2,1)), static_cast<T>((*this)(2,2)) });
		}

		ElementType Determinant() const
		{
			return glm::determinant(_m);
		}

		Matrix3x3 Transpose() const
		{
			return Matrix3x3(glm::transpose(_m));
		}

		Matrix3x3 Adjugate() const
		{
			// Cofactor matrix transposed, using (row,col) semantics
			auto e = [&](int r, int c) { return (*this)(r, c); };
			return Matrix3x3(
				Vector3<ElementType>{
					e(1,1) * e(2,2) - e(1,2) * e(2,1),
					e(0,2) * e(2,1) - e(0,1) * e(2,2),
					e(0,1) * e(1,2) - e(0,2) * e(1,1) },
				Vector3<ElementType>{
					e(1,2) * e(2,0) - e(1,0) * e(2,2),
					e(0,0) * e(2,2) - e(0,2) * e(2,0),
					e(0,2) * e(1,0) - e(0,0) * e(1,2) },
				Vector3<ElementType>{
					e(1,0) * e(2,1) - e(1,1) * e(2,0),
					e(0,1) * e(2,0) - e(0,0) * e(2,1),
					e(0,0) * e(1,1) - e(0,1) * e(1,0) });
		}

		Matrix3x3 Inverse() const
		{
			ElementType det = Determinant();
			if (det == 0)
				return Zero;

			return Matrix3x3(glm::inverse(_m));
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

		static const Matrix3x3 Zero;
		static const Matrix3x3 Identity;

	private:
		GlmType _m;
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

	// Matrix multiplication
	template <typename ElementType>
	Matrix3x3<ElementType> operator*(const Matrix3x3<ElementType>& left, const Matrix3x3<ElementType>& right)
	{
		return Matrix3x3<ElementType>(left.Glm() * right.Glm());
	}

	// Matrix-vector multiplication
	template <typename ElementType>
	Vector3<ElementType> operator*(const Matrix3x3<ElementType>& matrix, const Vector3<ElementType>& vector)
	{
		auto result = matrix.Glm() * vector.Glm();
		return Vector3<ElementType>(result.x, result.y, result.z);
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
