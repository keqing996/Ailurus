#pragma once

#include "Vector.hpp"
#include "Matrix.hpp"
#include "Quaternion.hpp"
#include "EulerAngle.hpp"

namespace Ailurus::Math
{
	namespace Rotation
	{
		template <typename T>
		Quaternion<T> EulerAngleToQuaternion(const EulerAngles<T>& euler)
		{
			T cy = std::cos(euler.yaw * 0.5);
			T sy = std::sin(euler.yaw * 0.5);
			T cp = std::cos(euler.pitch * 0.5);
			T sp = std::sin(euler.pitch * 0.5);
			T cr = std::cos(euler.roll * 0.5);
			T sr = std::sin(euler.roll * 0.5);

			return Quaternion<T>(cr * sp * cy - sr * cp * sy,
				sr * cp * cy + cr * sp * sy,
				cr * cp * sy - sr * sp * cy,
				cr * cp * cy + sr * sp * sy);
		}

		template <typename T>
		EulerAngles<T> QuaternionToEulerAngles(const Quaternion<T>& quaternion)
		{
			EulerAngles<T> angles;

			// roll (x-axis rotation)
			T sinr_cosp = 2 * (quaternion.w * quaternion.x + quaternion.y * quaternion.z);
			T cosr_cosp = 1 - 2 * (quaternion.x * quaternion.x + quaternion.y * quaternion.y);
			angles.roll = std::atan2(sinr_cosp, cosr_cosp);

			// pitch (y-axis rotation)
			T sinp = 2 * (quaternion.w * quaternion.y - quaternion.z * quaternion.x);
			if (std::abs(sinp) >= 1)
				angles.pitch = std::copysign(std::numbers::pi_v<T> / 2, sinp); // use 90 degrees if out of range
			else
				angles.pitch = std::asin(sinp);

			// yaw (z-axis rotation)
			T siny_cosp = 2 * (quaternion.w * quaternion.z + quaternion.x * quaternion.y);
			T cosy_cosp = 1 - 2 * (quaternion.y * quaternion.y + quaternion.z * quaternion.z);
			angles.yaw = std::atan2(siny_cosp, cosy_cosp);

			return angles;
		}

		template <typename T>
		Matrix<T, 4, 4> QuaternionToMatrix(const Quaternion<T>& quaternion)
		{
			Matrix<T, 4, 4> matrix;

			T x2 = quaternion.x * quaternion.x;
			T y2 = quaternion.y * quaternion.y;
			T z2 = quaternion.z * quaternion.z;
			T xy = quaternion.x * quaternion.y;
			T xz = quaternion.x * quaternion.z;
			T yz = quaternion.y * quaternion.z;
			T wx = quaternion.w * quaternion.x;
			T wy = quaternion.w * quaternion.y;
			T wz = quaternion.w * quaternion.z;

			matrix[0][0] = 1 - 2 * (y2 + z2);
			matrix[0][1] = 2 * (xy - wz);
			matrix[0][2] = 2 * (xz + wy);
			matrix[0][3] = 0;

			matrix[1][0] = 2 * (xy + wz);
			matrix[1][1] = 1 - 2 * (x2 + z2);
			matrix[1][2] = 2 * (yz - wx);
			matrix[1][3] = 0;

			matrix[2][0] = 2 * (xz - wy);
			matrix[2][1] = 2 * (yz + wx);
			matrix[2][2] = 1 - 2 * (x2 + y2);
			matrix[2][3] = 0;

			matrix[3][0] = 0;
			matrix[3][1] = 0;
			matrix[3][2] = 0;
			matrix[3][3] = 1;

			return matrix;
		}

		template <typename T>
		Quaternion<T> MatrixToQuaternion(const Matrix<T, 4, 4>& matrix)
		{
			Quaternion<T> quaternion;

			T trace = matrix[0][0] + matrix[1][1] + matrix[2][2];
			if (trace > 0)
			{
				T s = std::sqrt(trace + 1);
				quaternion.w = s / 2;
				s = 0.5 / s;
				quaternion.x = (matrix[2][1] - matrix[1][2]) * s;
				quaternion.y = (matrix[0][2] - matrix[2][0]) * s;
				quaternion.z = (matrix[1][0] - matrix[0][1]) * s;
			}
			else
			{
				int i = 0;
				if (matrix[1][1] > matrix[0][0])
					i = 1;
				if (matrix[2][2] > matrix[i][i])
					i = 2;

				int j = (i + 1) % 3;
				int k = (i + 2) % 3;

				T s = std::sqrt(matrix[i][i] - matrix[j][j] - matrix[k][k] + 1);
				quaternion[i] = s / 2;
				s = 0.5 / s;

				quaternion[j] = (matrix[i][j] + matrix[j][i]) * s;
				quaternion[k] = (matrix[i][k] + matrix[k][i]) * s;
				quaternion.w = (matrix[j][k] - matrix[k][j]) * s;
			}

			return quaternion;
		}

		template <typename T>
		Matrix<T, 4, 4> EulerAngleToMatrix(const EulerAngles<T>& euler)
		{
			Quaternion<T> quaternion = EulerAngleToQuaternion(euler);
			return QuaternionToMatrix(quaternion);
		}

		template <typename T>
		EulerAngles<T> MatrixToEulerAngle(const Matrix<T, 4, 4>& matrix)
		{
			Quaternion<T> quaternion = MatrixToQuaternion(matrix);
			return QuaternionToEulerAngles(quaternion);
		}

		template <typename T>
		Quaternion<T> RotateAxis(const Vector<T, 3>& axis, T angle)
		{
			T halfAngle = angle / 2;
			T s = std::sin(halfAngle);
			return Quaternion<T>(axis.x() * s, axis.y() * s, axis.z() * s, std::cos(halfAngle));
		}
	}
} // namespace Ailurus