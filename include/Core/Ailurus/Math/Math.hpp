#pragma once

#include <numbers>
#include "Vector3.hpp"
#include "Vector4.hpp"
#include "Matrix4x4.hpp"
#include "Quaternion.hpp"
#include "EulerAngle.hpp"

namespace Ailurus::Math
{
	template <typename T>
		requires std::is_floating_point_v<T>
	T DegreeToRadian(T degrees)
	{
		return degrees * std::numbers::pi_v<T> / T(180);
	}

	template <typename T>
		requires std::is_floating_point_v<T>
	T RadianToDegree(T radians)
	{
		return radians * T(180) / std::numbers::pi_v<T>;
	}

	template <typename T>
	Matrix4x4<T> TranslateMatrix(const Vector3<T>& translation)
	{
		Matrix4x4<T> matrix = Matrix4x4<T>::Identity;
		matrix.SetCol(3, Vector4<T>(translation.x, translation.y, translation.z, 1));
		return matrix;
	}

	template <typename T>
	Matrix4x4<T> ScaleMatrix(const Vector3<T>& scale)
	{
		Matrix4x4<T> result = Matrix4x4<T>::Identity;
		result[0][0] = scale.x;
		result[1][1] = scale.y;
		result[2][2] = scale.z;
		return result;
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

	template <typename T>
	Quaternion<T> RotateAxis(const Vector3<T>& axis, T angle)
	{
		T halfAngle = angle / 2;
		T s = std::sin(halfAngle);
		return Quaternion<T>(axis.x * s, axis.y * s, axis.z * s, std::cos(halfAngle));
	}

	template <typename T>
	Matrix4x4<T> MakeModelMatrix(const Vector3<T>& pos, const Quaternion<T>& rot, const Vector3<T>& scale)
	{
		return TranslateMatrix(pos) * QuaternionToRotateMatrix(rot) * ScaleMatrix(scale);
	}

	template <typename T>
	Matrix4x4<T> MakeViewMatrix(const Vector3<T>& pos, const Quaternion<T>& rot)
	{
		Matrix4x4<T> translationMatrix = TranslateMatrix(-pos);
		Matrix4x4<T> rotationMatrix = QuaternionToRotateMatrix(rot.Conjugate());

		return rotationMatrix * translationMatrix;
	}

	/**
     * @brief Projection matrix of ortho camera, frustum is symmetry by y-axis & z-axis. \n\n
     *
     * For standard ortho projection, before projection matrix, camera looks at +x, near value
     * and far value are greater than zero, near > far > 0. The standard ortho projection maps
     * near value to -1 and maps far value to +1. \n\n
     *
     * @param nearPlaneHalfY Near plane half width.
     * @param nearPlaneHalfZ Near plane half height.
     * @param nearPlaneX Near plane x value.
     * @param farPlaneX Far plane x value.
     */
	template <typename T>
	Matrix4x4<T> MakeOrthoProjectionMatrix(float nearPlaneHalfY, float nearPlaneHalfZ, float nearPlaneX, float farPlaneX)
	{
		const float vLeft = -nearPlaneHalfY;
		const float vRight = nearPlaneHalfY;
		const float vTop = nearPlaneHalfZ;
		const float vBottom = -nearPlaneHalfZ;
		const float vNear = nearPlaneX;
		const float vFar = farPlaneX;

		Matrix4x4<T> translationMatrix = TranslateMatrix(Vector3<T>{
			-(vRight + vLeft) / 2,
			-(vTop + vBottom) / 2,
			-(vNear + vFar) / 2 });

		Matrix4x4<T> scaleMatrix = ScaleMatrix(Vector3<T>{
			2 / (vRight - vLeft),
			2 / (vTop - vBottom),
			2 / (vNear - vFar) });

		Matrix4x4<T> standardOrthoProj = scaleMatrix * translationMatrix;

		return standardOrthoProj;
	}

	/**
     * @brief Projection matrix of perspective camera, frustum is symmetry by y-axis & z-axis. \n\n
     *
	 * @remark In OpenGL, we should apply two modifications to standard perspective projection result. The first
     * is multiply -1 to result. Because gl_Position's w value can not be less than zero. In OpenGL,
     * a clip space is defined to -w <= x,y,z < w, so if w is less than zero, clip space flipped,
     * which leads nothing drew. The second modification is to reverse z. The NDC coordinate in OpenGL
     * is left-handed system, but our coordinates in every calculation step is right-handed system,
     * so we need to flip z coordinate to fit OpenGL NDC space.
     *
     * @param nearPlaneHalfY Near plane half width.
     * @param nearPlaneHalfZ Near plane half height.
     * @param nearPlaneX Near plane x value.
     * @param farPlaneX Far plane x value.
     */
	template <typename T>
	Matrix4x4<T> MakePerspectiveProjectionMatrix(float nearPlaneHalfY, float nearPlaneHalfZ, float nearPlaneX, float farPlaneX)
	{
		const float vLeft = -nearPlaneHalfY;
		const float vRight = nearPlaneHalfY;
		const float vTop = nearPlaneHalfZ;
		const float vBottom = -nearPlaneHalfZ;
		const float vNear = nearPlaneX;
		const float vFar = farPlaneX;

		Matrix4x4<T> translationMatrix = TranslateMatrix(Vector3<T>{
			-(vRight + vLeft) / 2,
			-(vTop + vBottom) / 2,
			-(vNear + vFar) / 2 });

		Matrix4x4<T> scaleMatrix = ScaleMatrix(Vector3<T>{
			2 / (vRight - vLeft),
			2 / (vTop - vBottom),
			2 / (vNear - vFar) });

		Matrix4x4<T> standardOrthoProj = scaleMatrix * translationMatrix;

		Matrix4x4<T> compressMatrix = {
			{ vNear, 0, 0, 0 },
			{ 0, vNear, 0, 0 },
			{ 0, 0, vNear + vFar, -vFar * vNear },
			{ 0, 0, 1, 0 }
		};

		return standardOrthoProj * compressMatrix;
	}

} // namespace Ailurus::Math