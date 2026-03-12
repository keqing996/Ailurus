#pragma once

#include <numbers>

namespace Ailurus::Math
{
	template <typename T>
	requires std::is_floating_point_v<T>
	T DegreeToRadian(T degrees)
	{
		return glm::radians(degrees);
	}

	template <typename T>
	requires std::is_floating_point_v<T>
	T RadianToDegree(T radians)
	{
		return glm::degrees(radians);
	}

	template <typename T>
	Matrix4x4<T> TranslateMatrix(const Vector3<T>& translation)
	{
		return Matrix4x4<T>(glm::translate(typename Matrix4x4<T>::GlmType(T(1)), translation.Glm()));
	}

	template <typename T>
	Matrix4x4<T> ScaleMatrix(const Vector3<T>& scale)
	{
		return Matrix4x4<T>(glm::scale(typename Matrix4x4<T>::GlmType(T(1)), scale.Glm()));
	}

	template <typename T, typename F>
	Quaternion<T> RotateAxis(const Vector3<T>& axis, F angleInDegree)
	{
		T angle = glm::radians(static_cast<T>(angleInDegree));
		return Quaternion<T>(glm::angleAxis(angle, axis.Normalized().Glm()));
	}

	template <typename T>
	Matrix4x4<T> LookAtMatrix(const Vector3<T>& forward, const Vector3<T>& up)
	{
		Vector3<T> f = forward.Normalized();	// Normalize the forward vector
		Vector3<T> u = up.Normalized();			// Normalize the up vector
		Vector3<T> r = f.Cross(u).Normalized(); // Right vector (cross product of forward and up)

		// Ensure up vector is orthogonal to forward and right
		u = r.Cross(f);

		// Create rotation matrix from axes (row vectors → column-major via constructor)
		Matrix4x4<T> rotation = {
			{ f.x, u.x, r.x, 0 },
			{ f.y, u.y, r.y, 0 },
			{ f.z, u.z, r.z, 0 },
			{ 0, 0, 0, 1 }
		};

		return rotation;
	}

	template <typename T>
	Quaternion<T> LookAtQuaternion(const Vector3<T>& forward, const Vector3<T>& up)
	{
		return RotateMatrixToQuaternion(LookAtMatrix(forward, up));
	}

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
	Matrix4x4<T> QuaternionToRotateMatrix(const Quaternion<T>& quaternion)
	{
		return Matrix4x4<T>(glm::mat4_cast(quaternion.Glm()));
	}

	template <typename T>
	Quaternion<T> RotateMatrixToQuaternion(const Matrix4x4<T>& matrix)
	{
		return Quaternion<T>(glm::quat_cast(matrix.Glm()));
	}

	template <typename T>
	Matrix4x4<T> EulerAngleToRotateMatrix(const EulerAngles<T>& euler)
	{
		Quaternion<T> quaternion = EulerAngleToQuaternion(euler);
		return QuaternionToRotateMatrix(quaternion);
	}

	template <typename T>
	EulerAngles<T> RotateMatrixToEulerAngle(const Matrix4x4<T>& matrix)
	{
		Quaternion<T> quaternion = RotateMatrixToQuaternion(matrix);
		return QuaternionToEulerAngles(quaternion);
	}

	template <typename T, typename U> requires std::is_floating_point_v<U>
	T Lerp(const T& a, const T& b, U t)
	{
		return a + static_cast<T>((b - a) * t);
	}

	template <typename T> requires std::is_floating_point_v<T>
	Quaternion<T> SLerp(const Quaternion<T>& q1, const Quaternion<T>& q2, T t)
	{
		return Quaternion<T>(glm::slerp(q1.Glm(), q2.Glm(), t));
	}
} // namespace Ailurus::Math