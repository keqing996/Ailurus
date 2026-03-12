#pragma once

#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <glm/gtc/quaternion.hpp>
#include "Vector3.hpp"

namespace Ailurus
{
	template <typename ElementType>
	requires std::is_floating_point_v<ElementType>
	class Quaternion
	{
	public:
		using GlmType = glm::qua<ElementType>;

		ElementType x, y, z, w;

		Quaternion()
			: x(0), y(0), z(0), w(1) {}

		Quaternion(ElementType x, ElementType y, ElementType z, ElementType w)
			: x(x), y(y), z(z), w(w) {}

		explicit Quaternion(const GlmType& q)
			: x(q.x), y(q.y), z(q.z), w(q.w) {}

		Quaternion(const Quaternion&) = default;
		Quaternion(Quaternion&&) noexcept = default;
		Quaternion& operator=(const Quaternion&) = default;
		Quaternion& operator=(Quaternion&&) noexcept = default;

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

		template <typename T>
		explicit operator Quaternion<T>() const
		{
			return Quaternion<T>(static_cast<T>(x), static_cast<T>(y), static_cast<T>(z), static_cast<T>(w));
		}

		ElementType Dot(const Quaternion& other) const
		{
			return glm::dot(Glm(), other.Glm());
		}

		ElementType Magnitude() const
		{
			return glm::length(Glm());
		}

		Quaternion& Normalize()
		{
			GlmType n = glm::normalize(Glm());
			x = n.x; y = n.y; z = n.z; w = n.w;
			return *this;
		}

		Quaternion Normalized() const
		{
			return Quaternion(glm::normalize(Glm()));
		}

		Quaternion Conjugate() const
		{
			return Quaternion(glm::conjugate(Glm()));
		}

		Quaternion Inverse() const
		{
			return Quaternion(glm::inverse(Glm()));
		}

		GlmType& Glm() { return reinterpret_cast<GlmType&>(*this); }
		const GlmType& Glm() const { return reinterpret_cast<const GlmType&>(*this); }

		static const Quaternion Zero;
		static const Quaternion Identity;
	};

	static_assert(sizeof(Quaternion<float>) == sizeof(glm::quat), "Quaternion<float> and glm::quat must have matching layout");

	template <typename ElementType>
	requires std::is_floating_point_v<ElementType>
	Quaternion<ElementType> const Quaternion<ElementType>::Zero(ElementType(0), ElementType(0), ElementType(0), ElementType(0));

	template <typename ElementType>
	requires std::is_floating_point_v<ElementType>
	Quaternion<ElementType> const Quaternion<ElementType>::Identity(ElementType(0), ElementType(0), ElementType(0), ElementType(1));

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
		return Quaternion<ElementType>(left.Glm() + right.Glm());
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
		return Quaternion<ElementType>(left.Glm() - right.Glm());
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

	// Quaternion-Quaternion multiplication (delegated to GLM)
	template <typename ElementType>
	Quaternion<ElementType> operator*(const Quaternion<ElementType>& left, const Quaternion<ElementType>& right)
	{
		return Quaternion<ElementType>(left.Glm() * right.Glm());
	}

	template <typename ElementType, typename ScalarType>
	Quaternion<ElementType> operator*(ScalarType scalar, const Quaternion<ElementType>& quat)
	{
		return Quaternion<ElementType>(quat.x * scalar, quat.y * scalar, quat.z * scalar, quat.w * scalar);
	}

	template <typename ElementType, typename ScalarType>
	Quaternion<ElementType> operator*(const Quaternion<ElementType>& quat, ScalarType scalar)
	{
		return Quaternion<ElementType>(quat.x * scalar, quat.y * scalar, quat.z * scalar, quat.w * scalar);
	}

	template <typename ElementType>
	Vector3<ElementType> operator*(const Quaternion<ElementType>& quat, const Vector3<ElementType>& vec)
	{
		auto result = quat.Glm() * vec.Glm();
		return Vector3<ElementType>(result.x, result.y, result.z);
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

	template <typename ElementType>
	Quaternion<ElementType> operator-(Quaternion<ElementType>& quat)
	{
		return Quaternion<ElementType>(-quat.x, -quat.y, -quat.z, -quat.w);
	}

	using Quaternionf = Quaternion<float>;
	using Quaterniond = Quaternion<double>;
} // namespace Ailurus