#pragma once

#include <cmath>
#include <array>
#include "Internal/MathInternal.hpp"

namespace Ailurus
{
	template <typename ElementType>
		requires std::is_floating_point_v<ElementType>
	class Quaternion
	{
	public:
		ElementType w, x, y, z;

		Quaternion()
			: w(1), x(0), y(0), z(0) {}

		Quaternion(ElementType w, ElementType x, ElementType y, ElementType z)
			: w(w), x(x), y(y), z(z) {}

		Quaternion operator+(const Quaternion& other) const
		{
			return Quaternion(w + other.w, x + other.x, y + other.y, z + other.z);
		}

		Quaternion& operator+=(const Quaternion& other)
		{
			w += other.w;
			x += other.x;
			y += other.y;
			z += other.z;
			return *this;
		}

		Quaternion operator-(const Quaternion& other) const
		{
			return Quaternion(w - other.w, x - other.x, y - other.y, z - other.z);
		}

		Quaternion& operator-=(const Quaternion& other)
		{
			w -= other.w;
			x -= other.x;
			y -= other.y;
			z -= other.z;
			return *this;
		}

		Quaternion operator*(const Quaternion& other) const
		{
			return Quaternion(
				w * other.w - x * other.x - y * other.y - z * other.z,
				w * other.x + x * other.w + y * other.z - z * other.y,
				w * other.y - x * other.z + y * other.w + z * other.x,
				w * other.z + x * other.y - y * other.x + z * other.w);
		}

		Quaternion& operator*=(const Quaternion& other)
		{
			*this = *this * other;
			return *this;
		}

		Quaternion operator*(ElementType scalar) const
		{
			return Quaternion(w * scalar, x * scalar, y * scalar, z * scalar);
		}

		Quaternion& operator*=(ElementType scalar)
		{
			w *= scalar;
			x *= scalar;
			y *= scalar;
			z *= scalar;
			return *this;
		}

		ElementType Dot(const Quaternion& other) const
		{
			return w * other.w + x * other.x + y * other.y + z * other.z;
		}

		ElementType Magnitude() const
		{
			return std::sqrt(w * w + x * x + y * y + z * z);
		}

		template <typename T = ElementType>
			requires _internal::CanNormalize<T>
		void Normalize()
		{
			ElementType mag = Magnitude();
			w = w / mag;
			x = x / mag;
			y = y / mag;
			z = z / mag;
		}

		template <typename T = ElementType>
			requires _internal::CanNormalize<T>
		Quaternion Normalized() const
		{
			Quaternion result = *this;
			result.Normalize<>();
			return result;
		}

		Quaternion Conjugate() const
		{
			return Quaternion(w, -x, -y, -z);
		}

		Quaternion Inverse() const
		{
			ElementType norm = Magnitude();
			return Conjugate() * (1 / (norm * norm));
		}

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
} // namespace Ailurus