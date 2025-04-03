#pragma once

#include <cmath>
#include <array>
#include "Quaternion.hpp"

namespace Ailurus
{
    template <typename ElementType>
    class EulerAngles
    {
    public:
        ElementType pitch, yaw, roll;

        EulerAngles()
            : pitch(0), yaw(0), roll(0) {}

        EulerAngles(ElementType pitch, ElementType yaw, ElementType roll)
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

        bool operator==(const EulerAngles& other) const
        {
            return pitch == other.pitch && yaw == other.yaw && roll == other.roll;
        }

        bool operator!=(const EulerAngles& other) const
        {
            return !(*this == other);
        }

        Quaternion<ElementType> ToQuaternion() const
        {
            ElementType cy = std::cos(yaw * 0.5);
            ElementType sy = std::sin(yaw * 0.5);
            ElementType cp = std::cos(pitch * 0.5);
            ElementType sp = std::sin(pitch * 0.5);
            ElementType cr = std::cos(roll * 0.5);
            ElementType sr = std::sin(roll * 0.5);

            return Quaternion<ElementType>(
                sr * cp * cy - cr * sp * sy,
                cr * sp * cy + sr * cp * sy,
                cr * cp * sy - sr * sp * cy,
                cr * cp * cy + sr * sp * sy
            );
        }

        static EulerAngles FromQuaternion(const Quaternion<ElementType>& q)
        {
            ElementType sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
            ElementType cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
            ElementType roll = std::atan2(sinr_cosp, cosr_cosp);

            ElementType sinp = 2 * (q.w * q.y - q.z * q.x);
            ElementType pitch;
            if (std::abs(sinp) >= 1)
                pitch = std::copysign(M_PI / 2, sinp);
            else
                pitch = std::asin(sinp);

            ElementType siny_cosp = 2 * (q.w * q.z + q.x * q.y);
            ElementType cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
            ElementType yaw = std::atan2(siny_cosp, cosy_cosp);

            return EulerAngles(pitch, yaw, roll);
        }
    };
}