#pragma once

#include <cmath>
#include <array>

namespace Ailurus
{
	/**
	 * @brief Degrees Euler angles.
	 *
	 * Euler angle describe a ZYX intrinsic rotation, uses +X as forward, so 'row' rotates around
	 * X axis, 'pitch' rotates around Y axis, 'yaw' rotates around Z axis.
	 */
	template <typename ElementType>
		requires std::is_floating_point_v<ElementType>
	class EulerAngles
	{
	public:
		ElementType pitch, yaw, roll;

		EulerAngles()
			: pitch(0), yaw(0), roll(0) {}

		EulerAngles(ElementType roll, ElementType pitch, ElementType yaw)
			: pitch(pitch), yaw(yaw), roll(roll) {}

		EulerAngles(const EulerAngles& other)
			: pitch(other.pitch), yaw(other.yaw), roll(other.roll) {}

		EulerAngles(EulerAngles&& other) noexcept
			: pitch(std::move(other.pitch)), yaw(std::move(other.yaw)), roll(std::move(other.roll)) {}

		EulerAngles& operator=(const EulerAngles& other)
		{
			if (this != &other)
			{
				pitch = other.pitch;
				yaw = other.yaw;
				roll = other.roll;
			}
			return *this;
		}

		EulerAngles& operator=(EulerAngles&& other) noexcept
		{
			if (this != &other)
			{
				pitch = std::move(other.pitch);
				yaw = std::move(other.yaw);
				roll = std::move(other.roll);
			}
			return *this;
		}

		template <typename T>
		explicit operator EulerAngles<T>() const
		{
			EulerAngles<T> result;

			result.yaw = static_cast<T>(yaw);
			result.pitch = static_cast<T>(pitch);
			result.roll = static_cast<T>(roll);

			return result;
		}
	};

	template <typename ElementType>
	bool operator==(const EulerAngles<ElementType>& left, const EulerAngles<ElementType>& right)
	{
		return left.pitch == right.pitch && left.yaw == right.yaw && left.roll == right.roll;
	}

	template <typename ElementType>
	bool operator!=(const EulerAngles<ElementType>& left, const EulerAngles<ElementType>& right)
	{
		return !(left == right);
	}

	using EulerAnglesf = EulerAngles<float>;
	using EulerAnglesd = EulerAngles<double>;
} // namespace Ailurus