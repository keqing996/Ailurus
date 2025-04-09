#pragma once

#include <cmath>
#include <stdexcept>
#include "Vector3.hpp"
#include "Internal/MathInternal.hpp"

namespace Ailurus
{
	template <typename ElementType>
		requires std::is_floating_point_v<ElementType>
	class Quaternion
	{
	public:
		ElementType x, y, z, w;

		Quaternion()
			: x(0), y(0), z(0), w(1) {}

		Quaternion(ElementType x, ElementType y, ElementType z, ElementType w)
			: x(x), y(y), z(z), w(w) {}

		Quaternion(const Quaternion& other)
			: x(other.x), y(other.y), z(other.z), w(other.w) {}

		Quaternion(Quaternion&& other) noexcept
			: x(std::move(other.x)), y(std::move(other.y)), z(std::move(other.z)), w(std::move(other.w)) {}

		ElementType& operator[](std::size_t index)
		{
			switch (index)
			{
				case 0:
					return x;
				case 1:
					return y;
				case 2:
					return z;
				case 3:
					return w;
				default:
					throw std::out_of_range("Index out of range");
			}
		}

		const ElementType& operator[](std::size_t index) const
		{
			switch (index)
			{
				case 0:
					return x;
				case 1:
					return y;
				case 2:
					return z;
				case 3:
					return w;
				default:
					throw std::out_of_range("Index out of range");
			}
		}

		Quaternion& operator=(const Quaternion& other)
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

		Quaternion& operator=(Quaternion&& other) noexcept
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

		template <typename T>
		explicit operator Quaternion<T>() const
		{
			Quaternion<T> result;

			result.x = static_cast<T>(x);
			result.y = static_cast<T>(y);
			result.z = static_cast<T>(z);
			result.w = static_cast<T>(w);

			return result;
		}

		ElementType Dot(const Quaternion& other) const
		{
			return x * other.x + y * other.y + z * other.z + w * other.w;
		}

		ElementType Magnitude() const
		{
			return std::sqrt(x * x + y * y + z * z + w * w);
		}

		template <typename T = ElementType>
			requires _internal::CanNormalize<T>
		Quaternion& Normalize()
		{
			ElementType mag = Magnitude();
			if (mag > 0)
			{
				x = x / mag;
				y = y / mag;
				z = z / mag;
				w = w / mag;
			}
			return *this;
		}

		template <typename T = ElementType>
			requires _internal::CanNormalize<T>
		Quaternion Normalized() const
		{
			Quaternion result = *this;
			result.Normalize();
			return result;
		}

		Quaternion Conjugate() const
		{
			return Quaternion(-x, -y, -z, w);
		}

		Quaternion Inverse() const
		{
			ElementType norm = Magnitude();
			return Conjugate() * (1 / (norm * norm));
		}

		static const Quaternion Zero;
		static const Quaternion Identity;

		static Quaternion Lerp(const Quaternion& q1, const Quaternion& q2, ElementType t)
		{
			return (q1 * (1 - t) + q2 * t).Normalize();
		}

		static Quaternion SLerp(const Quaternion& q1, const Quaternion& q2, ElementType t)
		{
			ElementType dot = q1.Dot(q2);
			const ElementType threshold = 0.9995;

			if (dot > threshold)
			{
				return Lerp(q1, q2, t);
			}

			dot = std::clamp(dot, static_cast<ElementType>(-1), static_cast<ElementType>(1));
			ElementType theta_0 = std::acos(dot);
			ElementType theta = theta_0 * t;

			Quaternion q3 = (q2 - q1 * dot).Normalize();
			return q1 * std::cos(theta) + q3 * std::sin(theta);
		}
	};

	template <typename ElementType>
		requires std::is_floating_point_v<ElementType>
	Quaternion<ElementType> const Quaternion<ElementType>::Zero(ElementType(0), ElementType(0), ElementType(0), ElementType(0));

	template <typename ElementType>
		requires std::is_floating_point_v<ElementType>
	Quaternion<ElementType> const Quaternion<ElementType>::Identity(ElementType(1), ElementType(1), ElementType(1), ElementType(1));

	template <typename ElementType>
	bool operator==(const Quaternion<ElementType>& left, const Quaternion<ElementType>& right)
	{
		return left.x == right.x && left.y == right.y && left.z == right.z && left.w == right.w;
	}

	template <typename ElementType>
	bool operator!=(const Quaternion<ElementType>& left, const Quaternion<ElementType>& right)
	{
		return !(left == right);
	}

	template <typename ElementType>
	Quaternion<ElementType> operator+(const Quaternion<ElementType>& left, const Quaternion<ElementType>& right)
	{
		return Quaternion(left.x + right.x, left.y + right.y, left.z + right.z, left.w + right.w);
	}

	template <typename ElementType>
	Quaternion<ElementType>& operator+=(Quaternion<ElementType>& left, const Quaternion<ElementType>& right)
	{
		left.x += right.x;
		left.y += right.y;
		left.z += right.z;
		left.w += right.w;
		return left;
	}

	template <typename ElementType>
	Quaternion<ElementType> operator-(const Quaternion<ElementType>& left, const Quaternion<ElementType>& right)
	{
		return Quaternion(left.x - right.x, left.y - right.y, left.z - right.z, left.w - right.w);
	}

	template <typename ElementType>
	Quaternion<ElementType>& operator-=(Quaternion<ElementType>& left, const Quaternion<ElementType>& right)
	{
		left.x -= right.x;
		left.y -= right.y;
		left.z -= right.z;
		left.w -= right.w;
		return left;
	}

	template <typename ElementType>
	Quaternion<ElementType> operator*(const Quaternion<ElementType>& left, const Quaternion<ElementType>& right)
	{
		ElementType w = left.w * right.w - left.x * right.x - left.y * right.y - left.z * right.z;
		ElementType x = left.w * right.x + left.x * right.w + left.y * right.z - left.z * right.y;
		ElementType y = left.w * right.y + left.y * right.w + left.z * right.x - left.x * right.z;
		ElementType z = left.w * right.z + left.z * right.w + left.x * right.y - left.y * right.x;
		return Quaternion<ElementType>(x, y, z, w);
	}

	template <typename ElementType, typename ScalarType>
	Quaternion<ElementType> operator*(ScalarType scalar, const Quaternion<ElementType>& quat)
	{
		return Quaternion(quat.x * scalar, quat.y * scalar, quat.z * scalar, quat.w * scalar);
	}

	template <typename ElementType, typename ScalarType>
	Quaternion<ElementType> operator*(const Quaternion<ElementType>& quat, ScalarType scalar)
	{
		return Quaternion(quat.x * scalar, quat.y * scalar, quat.z * scalar, quat.w * scalar);
	}

	template <typename ElementType>
	Vector3<ElementType> operator*(const Quaternion<ElementType>& quat, const Vector3<ElementType>& vec)
	{
		Quaternion<ElementType> vecQuat(vec.x(), vec.y(), vec.z(), 0);
		Quaternion<ElementType> result = quat * vecQuat * quat.Conjugate();
		return Vector<ElementType, 3>(result.x, result.y, result.z);
	}

	template <typename ElementType, typename ScalarType>
	Quaternion<ElementType>& operator*=(Quaternion<ElementType>& quat, ScalarType scalar)
	{
		quat.x *= scalar;
		quat.y *= scalar;
		quat.z *= scalar;
		quat.w *= scalar;
		return quat;
	}

} // namespace Ailurus