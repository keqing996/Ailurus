#pragma once

#include <numbers>

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

	template <typename T, typename F>
	Quaternion<T> RotateAxis(const Vector3<T>& axis, F angleInDegree)
	{
		T angle = Math::DegreeToRadian(static_cast<T>(angleInDegree));
		T halfAngle = angle / 2;
		T s = std::sin(halfAngle);
		return Quaternion<T>(axis.x * static_cast<T>(s),
			axis.y * static_cast<T>(s),
			axis.z * static_cast<T>(s),
			static_cast<T>(std::cos(halfAngle)));
	}

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
			2 / (vFar - vNear) });

		Matrix4x4<T> standardOrthoProj = scaleMatrix * translationMatrix;

		return standardOrthoProj;
	}

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
			2 / (vFar - vNear) });

		Matrix4x4<T> standardOrthoProj = scaleMatrix * translationMatrix;

		Matrix4x4<T> compressMatrix = {
			{ vNear, 0, 0, 0 },
			{ 0, vNear, 0, 0 },
			{ 0, 0, vNear + vFar, -vFar * vNear },
			{ 0, 0, 1, 0 }
		};

		return standardOrthoProj * compressMatrix;
	}

	template <typename T, typename U> requires std::is_floating_point_v<U>
	T Lerp(const T& a, const T& b, U t)
	{
		return a + static_cast<T>((b - a) * t);
	}

	template <typename T> requires std::is_floating_point_v<T>
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

	template <typename T>
	constexpr Matrix4x4<T> MakeNdcMatrixOpenGL()
	{
		auto matSwitchAxis = Matrix4x4<T>{
			{ 0, 0, 1, 0 }, // +x -> +z
			{ 1, 0, 0, 0 }, // +y -> +x
			{ 0, 1, 0, 0 }, // +z -> +y
			{ 0, 0, 0, 1 }
		};

		auto matCompressZ = Matrix4x4<T>::Identity;

		return matCompressZ * matSwitchAxis;
	}

	template <typename T>
	constexpr Matrix4x4<T> MakeNdcMatrixVulkan()
	{
		auto matSwitchAxis = Matrix4x4<T>{
			{ 0, 0, 1, 0 }, // +x -> +z
			{ 1, 0, 0, 0 }, // +y -> +x
			{ 0, 1, 0, 0 }, // +z -> -y
			{ 0, 0, 0, 1 }
		};

		auto matCompressZ = Matrix4x4<T>{
			{ 1, 0, 0, 0 },
			{ 0, 1, 0, 0 },
			{ 0, 0, 0.5, 0.5 }, // 0.5 * z + 0.6
			{ 0, 0, 0, 1 }
		};

		return matCompressZ * matSwitchAxis;
	}

	template <typename T>
	constexpr Matrix4x4<T> MakeNdcMatrixD3D()
	{
		auto matSwitchAxis = Matrix4x4<T>{
			{ 0, 0, 1, 0 }, // +x -> +z
			{ 1, 0, 0, 0 }, // +y -> +x
			{ 0, 1, 0, 0 }, // +z -> +y
			{ 0, 0, 0, 1 }
		};

		auto matCompressZ = Matrix4x4<T>{
			{ 1, 0, 0, 0 },
			{ 0, 1, 0, 0 },
			{ 0, 0, 0.5, 0.5 }, // 0.5 * z + 0.6
			{ 0, 0, 0, 1 }
		};

		return matCompressZ * matSwitchAxis;
	}
} // namespace Ailurus::Math