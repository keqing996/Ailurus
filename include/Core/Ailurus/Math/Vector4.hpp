#pragma once

#include <cmath>
#include <utility>

namespace Ailurus
{
	template <typename ElementType>
	struct Vector4
	{
		ElementType x, y, z, w;

		Vector4()
			: x(0), y(0), z(0), w(0) {}

		Vector4(ElementType x, ElementType y, ElementType z, ElementType w)
			: x(x), y(y), z(z), w(w) {}

		Vector4(const Vector4& other)
			: x(other.x), y(other.y), z(other.z), w(other.w) {}

		Vector4(Vector4&& other) noexcept
			: x(std::move(other.x)), y(std::move(other.y)), z(std::move(other.z)), w(std::move(other.w)) {}

		~Vector4() = default;

		Vector4& operator=(const Vector4& other)
		{
			if (this != &other)
			{
				x = other.x;
				y = other.y;
				z = other.z;
				w = other.w;
			}
			return *this;
		}

		Vector4& operator=(Vector4&& other) noexcept
		{
			if (this != &other)
			{
				x = std::move(other.x);
				y = std::move(other.y);
				z = std::move(other.z);
				w = std::move(other.w);
			}
			return *this;
		}

		ElementType& operator[](std::size_t index)
		{
			return (index == 0) ? x : ((index == 1) ? y : ((index == 2) ? z : w));
		}

		const ElementType& operator[](std::size_t index) const
		{
			return (index == 0) ? x : ((index == 1) ? y : ((index == 2) ? z : w));
		}

		template <typename T>
		explicit operator Vector4<T>() const
		{
			return { static_cast<T>(x), static_cast<T>(y), static_cast<T>(z), static_cast<T>(w) };
		}

		ElementType SquareMagnitude() const
		{
			return x * x + y * y + z * z + w * w;
		}

		ElementType Magnitude() const
		{
			return std::sqrt(x * x + y * y + z * z + w * w);
		}

		Vector4& Normalize()
		{
			ElementType mag = Magnitude();
			if (mag > 0)
			{
				x /= mag;
				y /= mag;
				z /= mag;
				w /= mag;
			}
			return *this;
		}

		Vector4 Normalized() const
		{
			Vector4 result(*this);
			result.Normalize();
			return result;
		}

		ElementType Dot(const Vector4& other) const
		{
			return x * other.x + y * other.y + z * other.z + w * other.w;
		}

		static const Vector4 Zero;
		static const Vector4 One;
	};

	template <typename ElementType>
	Vector4<ElementType> const Vector4<ElementType>::Zero(ElementType(0), ElementType(0), ElementType(0), ElementType(0));

	template <typename ElementType>
	Vector4<ElementType> const Vector4<ElementType>::One(ElementType(1), ElementType(1), ElementType(1), ElementType(1));

	// == operator
	template <typename ElementType>
	bool operator==(const Vector4<ElementType>& left, const Vector4<ElementType>& right)
	{
		return left.x == right.x && left.y == right.y && left.z == right.z && left.w == right.w;
	}

	// != operator
	template <typename ElementType>
	bool operator!=(const Vector4<ElementType>& left, const Vector4<ElementType>& right)
	{
		return !(left == right);
	}

	// Vector4 + Scalar
	template <typename ElementType, typename ScalarType>
	Vector4<ElementType> operator+(const Vector4<ElementType>& vec, const ScalarType& scalar)
	{
		return { vec.x + scalar, vec.y + scalar, vec.z + scalar, vec.w + scalar };
	}

	// Scalar + Vector4
	template <typename ElementType, typename ScalarType>
	Vector4<ElementType> operator+(const ScalarType& scalar, const Vector4<ElementType>& vec)
	{
		return vec + scalar;
	}

	// Vector4 - Scalar
	template <typename ElementType, typename ScalarType>
	Vector4<ElementType> operator-(const Vector4<ElementType>& vec, const ScalarType& scalar)
	{
		return { vec.x - scalar, vec.y - scalar, vec.z - scalar, vec.w - scalar };
	}

	// Scalar - Vector4
	template <typename ElementType, typename ScalarType>
	Vector4<ElementType> operator-(const ScalarType& scalar, const Vector4<ElementType>& vec)
	{
		return { scalar - vec.x, scalar - vec.y, scalar - vec.z, scalar - vec.w };
	}

	// Vector4 * Scalar
	template <typename ElementType, typename ScalarType>
	Vector4<ElementType> operator*(const Vector4<ElementType>& vec, const ScalarType& scalar)
	{
		return { vec.x * scalar, vec.y * scalar, vec.z * scalar, vec.w * scalar };
	}

	// Scalar * Vector4
	template <typename ElementType, typename ScalarType>
	Vector4<ElementType> operator*(const ScalarType& scalar, const Vector4<ElementType>& vec)
	{
		return vec * scalar;
	}

	// Vector4 / Scalar
	template <typename ElementType, typename ScalarType>
	Vector4<ElementType> operator/(const Vector4<ElementType>& vec, const ScalarType& scalar)
	{
		return { vec.x / scalar, vec.y / scalar, vec.z / scalar, vec.w / scalar };
	}

	// Scalar / Vector4
	template <typename ElementType, typename ScalarType>
	Vector4<ElementType> operator/(const ScalarType& scalar, const Vector4<ElementType>& vec)
	{
		return { scalar / vec.x, scalar / vec.y, scalar / vec.z, scalar / vec.w };
	}

	// Vector4 += Scalar
	template <typename ElementType, typename ScalarType>
	Vector4<ElementType>& operator+=(Vector4<ElementType>& vec, const ScalarType& scalar)
	{
		vec.x += scalar;
		vec.y += scalar;
		vec.z += scalar;
		vec.w += scalar;
		return vec;
	}

	// Vector4 -= Scalar
	template <typename ElementType, typename ScalarType>
	Vector4<ElementType>& operator-=(Vector4<ElementType>& vec, const ScalarType& scalar)
	{
		vec.x -= scalar;
		vec.y -= scalar;
		vec.z -= scalar;
		vec.w -= scalar;
		return vec;
	}

	// Vector4 *= Scalar
	template <typename ElementType, typename ScalarType>
	Vector4<ElementType>& operator*=(Vector4<ElementType>& vec, const ScalarType& scalar)
	{
		vec.x *= scalar;
		vec.y *= scalar;
		vec.z *= scalar;
		vec.w *= scalar;
		return vec;
	}

	// Vector4 /= Scalar
	template <typename ElementType, typename ScalarType>
	Vector4<ElementType>& operator/=(Vector4<ElementType>& vec, const ScalarType& scalar)
	{
		vec.x /= scalar;
		vec.y /= scalar;
		vec.z /= scalar;
		vec.w /= scalar;
		return vec;
	}

	// Vector4 + Vector4
	template <typename ElementType>
	Vector4<ElementType> operator+(const Vector4<ElementType>& left, const Vector4<ElementType>& right)
	{
		return { left.x + right.x, left.y + right.y, left.z + right.z, left.w + right.w };
	}

	// Vector4 - Vector4
	template <typename ElementType>
	Vector4<ElementType> operator-(const Vector4<ElementType>& left, const Vector4<ElementType>& right)
	{
		return { left.x - right.x, left.y - right.y, left.z - right.z, left.w - right.w };
	}

	// Vector4 += Vector4
	template <typename ElementType>
	Vector4<ElementType>& operator+=(Vector4<ElementType>& left, const Vector4<ElementType>& right)
	{
		left.x += right.x;
		left.y += right.y;
		left.z += right.z;
		left.w += right.w;
		return left;
	}

	// Vector4 -= Vector4
	template <typename ElementType>
	Vector4<ElementType>& operator-=(Vector4<ElementType>& left, const Vector4<ElementType>& right)
	{
		left.x -= right.x;
		left.y -= right.y;
		left.z -= right.z;
		left.w -= right.w;
		return left;
	}

	// -Vector4
	template <typename ElementType>
	Vector4<ElementType> operator-(const Vector4<ElementType>& vec)
	{
		return { -vec.x, -vec.y, -vec.z, -vec.w };
	}

	using Vector4i = Vector4<int>;
	using Vector4f = Vector4<float>;
	using Vector4d = Vector4<double>;
	using Vector4u = Vector4<unsigned int>;
	using Vector4l = Vector4<long long>;
	using Vector4ul = Vector4<unsigned long long>;

} // namespace Ailurus