#pragma once

#include <cmath>

namespace Ailurus
{
	template <typename ElementType>
	struct Vector3
	{
		ElementType x, y, z;

		Vector3()
			: x(0), y(0), z(0) {}

		Vector3(ElementType x, ElementType y, ElementType z)
			: x(x), y(y), z(z) {}

		Vector3(const Vector3& other)
			: x(other.x), y(other.y), z(other.z) {}

		Vector3(Vector3&& other) noexcept
			: x(std::move(other.x)), y(std::move(other.y)), z(std::move(other.z)) {}

		Vector3& operator=(const Vector3& other)
		{
			if (this != &other)
			{
				x = other.x;
				y = other.y;
				z = other.z;
			}
			return *this;
		}

		Vector3& operator=(Vector3&& other) noexcept
		{
			if (this != &other)
			{
				x = std::move(other.x);
				y = std::move(other.y);
				z = std::move(other.z);
			}
			return *this;
		}

		ElementType& operator[](std::size_t index)
		{
			return (index == 0) ? x : ((index == 1) ? y : z);
		}

		const ElementType& operator[](std::size_t index) const
		{
			return (index == 0) ? x : ((index == 1) ? y : z);
		}

		template <typename T>
		explicit operator Vector3<T>() const
		{
			return { static_cast<T>(x), static_cast<T>(y), static_cast<T>(z) };
		}

		ElementType SquareMagnitude() const
		{
			return x * x + y * y + z * z;
		}

		ElementType Magnitude() const
		{
			return std::sqrt(x * x + y * y + z * z);
		}

		Vector3& Normalize()
		{
			ElementType mag = Magnitude();
			if (mag > 0)
			{
				x /= mag;
				y /= mag;
				z /= mag;
			}
			return *this;
		}

		Vector3 Normalized() const
		{
			Vector3 result(*this);
			result.Normalize();
			return result;
		}

		ElementType Dot(const Vector3& other) const
		{
			return x * other.x + y * other.y + z * other.z;
		}

		Vector3 Cross(const Vector3& other) const
		{
			return {
				y * other.z - z * other.y,
				z * other.x - x * other.z,
				x * other.y - y * other.x
			};
		}

		static const Vector3 Zero;
		static const Vector3 One;
		static const Vector3 Up;
		static const Vector3 Right;
		static const Vector3 Forward;
	};

	template <typename ElementType>
	Vector3<ElementType> const Vector3<ElementType>::Zero(ElementType(0), ElementType(0), ElementType(0));

	template <typename ElementType>
	Vector3<ElementType> const Vector3<ElementType>::One(ElementType(1), ElementType(1), ElementType(1));

	template <typename ElementType>
	Vector3<ElementType> const Vector3<ElementType>::Up(ElementType(0), ElementType(0), ElementType(1));

	template <typename ElementType>
	Vector3<ElementType> const Vector3<ElementType>::Right(ElementType(0), ElementType(1), ElementType(0));

	template <typename ElementType>
	Vector3<ElementType> const Vector3<ElementType>::Forward(ElementType(1), ElementType(0), ElementType(0));

	// == operator
	template <typename ElementType>
	bool operator==(const Vector3<ElementType>& left, const Vector3<ElementType>& right)
	{
		return left.x == right.x && left.y == right.y && left.z == right.z;
	}

	// != operator
	template <typename ElementType>
	bool operator!=(const Vector3<ElementType>& left, const Vector3<ElementType>& right)
	{
		return !(left == right);
	}

	// Vector3 + Scalar
	template <typename ElementType, typename ScalarType>
	Vector3<ElementType> operator+(const Vector3<ElementType>& vec, const ScalarType& scalar)
	{
		return { vec.x + scalar, vec.y + scalar, vec.z + scalar };
	}

	// Scalar + Vector3
	template <typename ElementType, typename ScalarType>
	Vector3<ElementType> operator+(const ScalarType& scalar, const Vector3<ElementType>& vec)
	{
		return vec + scalar;
	}

	// Vector3 - Scalar
	template <typename ElementType, typename ScalarType>
	Vector3<ElementType> operator-(const Vector3<ElementType>& vec, const ScalarType& scalar)
	{
		return { vec.x - scalar, vec.y - scalar, vec.z - scalar };
	}

	// Scalar - Vector3
	template <typename ElementType, typename ScalarType>
	Vector3<ElementType> operator-(const ScalarType& scalar, const Vector3<ElementType>& vec)
	{
		return { scalar - vec.x, scalar - vec.y, scalar - vec.z };
	}

	// Vector3 * Scalar
	template <typename ElementType, typename ScalarType>
	Vector3<ElementType> operator*(const Vector3<ElementType>& vec, const ScalarType& scalar)
	{
		return { vec.x * scalar, vec.y * scalar, vec.z * scalar };
	}

	// Scalar * Vector3
	template <typename ElementType, typename ScalarType>
	Vector3<ElementType> operator*(const ScalarType& scalar, const Vector3<ElementType>& vec)
	{
		return vec * scalar;
	}

	// Vector3 / Scalar
	template <typename ElementType, typename ScalarType>
	Vector3<ElementType> operator/(const Vector3<ElementType>& vec, const ScalarType& scalar)
	{
		return { vec.x / scalar, vec.y / scalar, vec.z / scalar };
	}

	// Scalar / Vector3
	template <typename ElementType, typename ScalarType>
	Vector3<ElementType> operator/(const ScalarType& scalar, const Vector3<ElementType>& vec)
	{
		return { scalar / vec.x, scalar / vec.y, scalar / vec.z };
	}

	// Vector3 += Scalar
	template <typename ElementType, typename ScalarType>
	Vector3<ElementType>& operator+=(Vector3<ElementType>& vec, const ScalarType& scalar)
	{
		vec.x += scalar;
		vec.y += scalar;
		vec.z += scalar;
		return vec;
	}

	// Vector3 -= Scalar
	template <typename ElementType, typename ScalarType>
	Vector3<ElementType>& operator-=(Vector3<ElementType>& vec, const ScalarType& scalar)
	{
		vec.x -= scalar;
		vec.y -= scalar;
		vec.z -= scalar;
		return vec;
	}

	// Vector3 *= Scalar
	template <typename ElementType, typename ScalarType>
	Vector3<ElementType>& operator*=(Vector3<ElementType>& vec, const ScalarType& scalar)
	{
		vec.x *= scalar;
		vec.y *= scalar;
		vec.z *= scalar;
		return vec;
	}

	// Vector3 /= Scalar
	template <typename ElementType, typename ScalarType>
	Vector3<ElementType>& operator/=(Vector3<ElementType>& vec, const ScalarType& scalar)
	{
		vec.x /= scalar;
		vec.y /= scalar;
		vec.z /= scalar;
		return vec;
	}

	// Vector3 + Vector3
	template <typename ElementType>
	Vector3<ElementType> operator+(const Vector3<ElementType>& left, const Vector3<ElementType>& right)
	{
		return { left.x + right.x, left.y + right.y, left.z + right.z };
	}

	// Vector3 - Vector3
	template <typename ElementType>
	Vector3<ElementType> operator-(const Vector3<ElementType>& left, const Vector3<ElementType>& right)
	{
		return { left.x - right.x, left.y - right.y, left.z - right.z };
	}

	// Vector3 += Vector3
	template <typename ElementType>
	Vector3<ElementType>& operator+=(Vector3<ElementType>& left, const Vector3<ElementType>& right)
	{
		left.x += right.x;
		left.y += right.y;
		left.z += right.z;
		return left;
	}

	// Vector3 -= Vector3
	template <typename ElementType>
	Vector3<ElementType>& operator-=(Vector3<ElementType>& left, const Vector3<ElementType>& right)
	{
		left.x -= right.x;
		left.y -= right.y;
		left.z -= right.z;
		return left;
	}

	using Vector3i = Vector3<int>;
	using Vector3f = Vector3<float>;
	using Vector3d = Vector3<double>;
	using Vector3u = Vector3<unsigned int>;
	using Vector3l = Vector3<long long>;
	using Vector3ul = Vector3<unsigned long long>;

} // namespace Ailurus