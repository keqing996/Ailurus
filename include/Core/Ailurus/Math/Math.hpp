#pragma once

#include <numbers>
#include "Vector2.hpp"
#include "Vector3.hpp"
#include "Matrix4x4.hpp"
#include "Quaternion.hpp"
#include "EulerAngle.hpp"

namespace Ailurus::Math
{
#pragma region[Basic Math Functions]

	/**
	 * @brief Converts an angle from degrees to radians.
	 *
	 * @param degrees The angle in degrees to convert.
	 * @return The equivalent angle in radians.
	 */
	template <typename T>
	requires std::is_floating_point_v<T>
	T DegreeToRadian(T degrees)
	{
		return degrees * std::numbers::pi_v<T> / T(180);
	}

	/**
	 * @brief Converts an angle from radians to degrees.
	 *
	 * @param radians The angle in radians to convert.
	 * @return The equivalent angle in degrees.
	 */
	template <typename T>
	requires std::is_floating_point_v<T>
	T RadianToDegree(T radians)
	{
		return radians * T(180) / std::numbers::pi_v<T>;
	}

	/**
	 * @brief Creates a translation matrix from a vector.
	 *
	 * @param translation The translation vector.
	 * @return Matrix4x4 representing the translation.
	 */
	template <typename T>
	Matrix4x4<T> TranslateMatrix(const Vector3<T>& translation)
	{
		Matrix4x4<T> matrix = Matrix4x4<T>::Identity;
		matrix.SetCol(3, Vector4<T>(translation.x, translation.y, translation.z, 1));
		return matrix;
	}

	/**
	 * @brief Creates a scale matrix from a vector.
	 *
	 * @param scale The scale vector.
	 * @return Matrix4x4 representing the scaling.
	 */
	template <typename T>
	Matrix4x4<T> ScaleMatrix(const Vector3<T>& scale)
	{
		Matrix4x4<T> result = Matrix4x4<T>::Identity;
		result[0][0] = scale.x;
		result[1][1] = scale.y;
		result[2][2] = scale.z;
		return result;
	}

	/**
	 * @brief Creates a quaternion that represents a rotation around a specific axis.
	 *
	 * @param axis The axis to rotate around (will be normalized).
	 * @param angleInDegree The rotation angle in degrees.
	 * @return Quaternion representing the rotation.
	 */
	template <typename T, typename F>
	Quaternion<T> RotateAxis(const Vector3<T>& axis, F angleInDegree)
	{
		Vector3<T> normalizedAxis = axis.Normalized();
		
		T angle = Math::DegreeToRadian(static_cast<T>(angleInDegree));
		T halfAngle = angle / 2;
		T s = std::sin(halfAngle);
		return Quaternion<T>(normalizedAxis.x * static_cast<T>(s),
			normalizedAxis.y * static_cast<T>(s),
			normalizedAxis.z * static_cast<T>(s),
			static_cast<T>(std::cos(halfAngle)));
	}

	/**
	 * @brief Creates a rotation matrix that orients an object to look in the specified direction.
	 *
	 * @param forward The direction to look at (will be normalized).
	 * @param up The up direction used to determine the orientation (will be normalized).
	 * @return Matrix4x4 representing the "look at" rotation.
	 */
	template <typename T>
	Matrix4x4<T> LookAtMatrix(const Vector3<T>& forward, const Vector3<T>& up)
	{
		Vector3<T> f = forward.Normalized();	// Normalize the forward vector
		Vector3<T> u = up.Normalized();			// Normalize the up vector
		Vector3<T> r = f.Cross(u).Normalized(); // Right vector (cross product of forward and up)

		// Ensure up vector is orthogonal to forward and right
		u = r.Cross(f);

		// Create rotation matrix from axes (column-major order for column vectors)
		Matrix4x4<T> rotation = {
			{ f.x, u.x, r.x, 0 },
			{ f.y, u.y, r.y, 0 },
			{ f.z, u.z, r.z, 0 },
			{ 0, 0, 0, 1 }
		};

		return rotation;
	}

#pragma endregion

#pragma region[Rotation Conversion]

	// Forward declaration for LookAtQuaternion
	template <typename T>
	Quaternion<T> RotateMatrixToQuaternion(const Matrix4x4<T>& matrix);

	/**
	 * @brief Creates a quaternion that represents a rotation looking in the specified direction.
	 *
	 * @param forward The direction to look at (will be normalized).
	 * @param up The up direction used to determine the orientation (will be normalized).
	 * @return Quaternion representing the "look at" rotation.
	 */
	template <typename T>
	Quaternion<T> LookAtQuaternion(const Vector3<T>& forward, const Vector3<T>& up)
	{
		return RotateMatrixToQuaternion(LookAtMatrix(forward, up));
	}

	/**
	 * @brief Converts Euler angles to a quaternion.
	 *
	 * @param euler The Euler angles to convert.
	 * @return Quaternion representing the same rotation.
	 */
	template <typename T>
	Quaternion<T> EulerAngleToQuaternion(const EulerAngles<T>& euler)
	{
		T cr = std::cos(euler.roll * 0.5);
		T sr = std::sin(euler.roll * 0.5);
		T cp = std::cos(euler.pitch * 0.5);
		T sp = std::sin(euler.pitch * 0.5);
		T cy = std::cos(euler.yaw * 0.5);
		T sy = std::sin(euler.yaw * 0.5);

		return Quaternion<T>(
			sr * cp * cy - cr * sp * sy,
			cr * sp * cy + sr * cp * sy,
			cr * cp * sy - sr * sp * cy,
			cr * cp * cy + sr * sp * sy);
	}

	/**
	 * @brief Converts a quaternion to Euler angles.
	 *
	 * @param quaternion The quaternion to convert.
	 * @return Euler angles representing the same rotation.
	 */
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

	/**
	 * @brief Converts a quaternion to a rotation matrix.
	 *
	 * @param quaternion The quaternion to convert.
	 * @return Matrix4x4 representing the same rotation.
	 */
	template <typename T>
	Matrix4x4<T> QuaternionToRotateMatrix(const Quaternion<T>& quaternion)
	{
		Matrix4x4<T> matrix;

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

	/**
	 * @brief Converts a rotation matrix to a quaternion.
	 *
	 * @param matrix The rotation matrix to convert.
	 * @return Quaternion representing the same rotation.
	 */
	template <typename T>
	Quaternion<T> RotateMatrixToQuaternion(const Matrix4x4<T>& matrix)
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

	/**
	 * @brief Converts Euler angles to a rotation matrix.
	 *
	 * @param euler The Euler angles to convert.
	 * @return Matrix4x4 representing the same rotation.
	 */
	template <typename T>
	Matrix4x4<T> EulerAngleToRotateMatrix(const EulerAngles<T>& euler)
	{
		Quaternion<T> quaternion = EulerAngleToQuaternion(euler);
		return QuaternionToRotateMatrix(quaternion);
	}

	/**
	 * @brief Converts a rotation matrix to Euler angles.
	 *
	 * @param matrix The rotation matrix to convert.
	 * @return Euler angles representing the same rotation.
	 */
	template <typename T>
	EulerAngles<T> RotateMatrixToEulerAngle(const Matrix4x4<T>& matrix)
	{
		Quaternion<T> quaternion = RotateMatrixToQuaternion(matrix);
		return QuaternionToEulerAngles(quaternion);
	}

#pragma endregion

#pragma region[Interpolation]

	/**
	 * @brief Linearly interpolates between two values.
	 *
	 * @param a The start value when t = 0.
	 * @param b The end value when t = 1.
	 * @param t The interpolation parameter (ideally between 0 and 1).
	 * @return The interpolated value.
	 */
	template <typename T, typename U>
	requires std::is_floating_point_v<U>
	T Lerp(const T& a, const T& b, U t)
	{
		return a + static_cast<T>((b - a) * t);
	}

	/**
	 * @brief Spherically interpolates between two quaternions.
	 *
	 * @param q1 The start quaternion when t = 0.
	 * @param q2 The end quaternion when t = 1.
	 * @param t The interpolation parameter (ideally between 0 and 1).
	 * @return The spherically interpolated quaternion.
	 */
	template <typename T>
	requires std::is_floating_point_v<T>
	Quaternion<T> SLerp(const Quaternion<T>& q1, const Quaternion<T>& q2, T t)
	{
		T dot = q1.Dot(q2);
		const T threshold = 0.9995;

		if (dot > threshold)
		{
			return Lerp(q1, q2, t);
		}

		dot = std::clamp(dot, static_cast<T>(-1), static_cast<T>(1));
		T theta_0 = std::acos(dot);
		T theta = theta_0 * t;

		Quaternion q3 = (q2 - q1 * dot).Normalize();
		return q1 * std::cos(theta) + q3 * std::sin(theta);
	}

#pragma endregion

} // namespace Ailurus::Math