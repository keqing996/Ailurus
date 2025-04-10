#pragma once

#include <cmath>

namespace Ailurus
{
	template <typename ElementType>
	struct Vector2
	{
		ElementType x, y;

		Vector2()
			: x(0), y(0) {}

		Vector2(ElementType x, ElementType y)
			: x(x), y(y) {}

		Vector2(const Vector2& other)
			: x(other.x), y(other.y) {}

		Vector2(Vector2&& other) noexcept
			: x(std::move(other.x)), y(std::move(other.y)) {}

		Vector2& operator=(const Vector2& other)
		{
			if (this != &other)
			{
				x = other.x;
				y = other.y;
			}
			return *this;
		}

		Vector2& operator=(Vector2&& other) noexcept
		{
			if (this != &other)
			{
				x = std::move(other.x);
				y = std::move(other.y);
			}
			return *this;
		}

		ElementType& operator[](std::size_t index)
		{
			return (index == 0) ? x : y;
		}

		const ElementType& operator[](std::size_t index) const
		{
			return (index == 0) ? x : y;
		}

		template <typename T>
		explicit operator Vector2<T>() const
		{
			return { static_cast<T>(x), static_cast<T>(y) };
		}

		ElementType SquareMagnitude() const
		{
			return x * x + y * y;
		}

		ElementType Magnitude() const
		{
			return std::sqrt(x * x + y * y);
		}

		Vector2& Normalize()
		{
			ElementType mag = Magnitude();
			if (mag > 0)
			{
				x /= mag;
				y /= mag;
			}
			return *this;
		}

		Vector2 Normalized() const
		{
			Vector2 result(*this);
			result.Normalize();
			return result;
		}

		ElementType Dot(const Vector2& other) const
		{
			return x * other.x + y * other.y;
		}

		static const Vector2 Zero;
		static const Vector2 One;
	};

	template<typename ElementType>
	Vector2<ElementType> const Vector2<ElementType>::Zero(ElementType(0), ElementType(0));

	template<typename ElementType>
	Vector2<ElementType> const Vector2<ElementType>::One(ElementType(1), ElementType(1));

	// == operator
	template <typename ElementType>
	bool operator==(const Vector2<ElementType>& left, const Vector2<ElementType>& right)
	{
		return left.x == right.x && left.y == right.y;
	}

	// != operator
	template <typename ElementType>
	bool operator!=(const Vector2<ElementType>& left, const Vector2<ElementType>& right)
	{
		return !(left == right);
	}

	// Vector2 + Scalar
	template <typename ElementType, typename ScalarType>
	Vector2<ElementType> operator+(const Vector2<ElementType>& vec, const ScalarType& scalar)
	{
		return { vec.x + scalar, vec.y + scalar };
	}

	// Scalar + Vector2
	template <typename ElementType, typename ScalarType>
	Vector2<ElementType> operator+(const ScalarType& scalar, const Vector2<ElementType>& vec)
	{
		return vec + scalar;
	}

	// Vector2 - Scalar
	template <typename ElementType, typename ScalarType>
	Vector2<ElementType> operator-(const Vector2<ElementType>& vec, const ScalarType& scalar)
	{
		return { vec.x - scalar, vec.y - scalar };
	}

	// Scalar - Vector2
	template <typename ElementType, typename ScalarType>
	Vector2<ElementType> operator-(const ScalarType& scalar, const Vector2<ElementType>& vec)
	{
		return { scalar - vec.x, scalar - vec.y };
	}

	// Vector2 * Scalar
	template <typename ElementType, typename ScalarType>
	Vector2<ElementType> operator*(const Vector2<ElementType>& vec, const ScalarType& scalar)
	{
		return { vec.x * scalar, vec.y * scalar };
	}

	// Scalar * Vector2
	template <typename ElementType, typename ScalarType>
	Vector2<ElementType> operator*(const ScalarType& scalar, const Vector2<ElementType>& vec)
	{
		return vec * scalar;
	}

	// Vector2 / Scalar
	template <typename ElementType, typename ScalarType>
	Vector2<ElementType> operator/(const Vector2<ElementType>& vec, const ScalarType& scalar)
	{
		return { vec.x / scalar, vec.y / scalar };
	}

	// Scalar / Vector2
	template <typename ElementType, typename ScalarType>
	Vector2<ElementType> operator/(const ScalarType& scalar, const Vector2<ElementType>& vec)
	{
		return { scalar / vec.x, scalar / vec.y };
	}

	// Vector2 += Scalar
	template <typename ElementType, typename ScalarType>
	Vector2<ElementType>& operator+=(Vector2<ElementType>& vec, const ScalarType& scalar)
	{
		vec.x += scalar;
		vec.y += scalar;
		return vec;
	}

	// Vector2 -= Scalar
	template <typename ElementType, typename ScalarType>
	Vector2<ElementType>& operator-=(Vector2<ElementType>& vec, const ScalarType& scalar)
	{
		vec.x -= scalar;
		vec.y -= scalar;
		return vec;
	}

	// Vector2 *= Scalar
	template <typename ElementType, typename ScalarType>
	Vector2<ElementType>& operator*=(Vector2<ElementType>& vec, const ScalarType& scalar)
	{
		vec.x *= scalar;
		vec.y *= scalar;
		return vec;
	}

	// Vector2 /= Scalar
	template <typename ElementType, typename ScalarType>
	Vector2<ElementType>& operator/=(Vector2<ElementType>& vec, const ScalarType& scalar)
	{
		vec.x /= scalar;
		vec.y /= scalar;
		return vec;
	}

	// Vector2 + Vector2
	template <typename ElementType>
	Vector2<ElementType> operator+(const Vector2<ElementType>& left, const Vector2<ElementType>& right)
	{
		return { left.x + right.x, left.y + right.y };
	}

	// Vector2 - Vector2
	template <typename ElementType>
	Vector2<ElementType> operator-(const Vector2<ElementType>& left, const Vector2<ElementType>& right)
	{
		return { left.x - right.x, left.y - right.y };
	}

	// Vector2 += Vector2
	template <typename ElementType>
	Vector2<ElementType>& operator+=(Vector2<ElementType>& left, const Vector2<ElementType>& right)
	{
		left.x += right.x;
		left.y += right.y;
		return left;
	}

	// Vector2 -= Vector2
	template <typename ElementType>
	Vector2<ElementType>& operator-=(Vector2<ElementType>& left, const Vector2<ElementType>& right)
	{
		left.x -= right.x;
		left.y -= right.y;
		return left;
	}

	// -Vector2
	template <typename ElementType>
	Vector2<ElementType> operator-(const Vector2<ElementType>& vec)
	{
		return { -vec.x, -vec.y };
	}

	using Vector2i = Vector2<int>;
	using Vector2f = Vector2<float>;
	using Vector2d = Vector2<double>;
	using Vector2u = Vector2<unsigned int>;
	using Vector2l = Vector2<long long>;
	using Vector2ul = Vector2<unsigned long long>;

} // namespace Ailurus