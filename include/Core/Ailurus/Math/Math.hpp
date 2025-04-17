#pragma once

#include "Vector2.hpp"
#include "Vector3.hpp"
#include "Matrix4x4.hpp"
#include "Quaternion.hpp"
#include "EulerAngle.hpp"

namespace Ailurus::Math
{
#pragma region[Basic Math Functions]

	template <typename T> requires std::is_floating_point_v<T>
	T DegreeToRadian(T degrees);

	template <typename T> requires std::is_floating_point_v<T>
	T RadianToDegree(T radians);

	template <typename T>
	Matrix4x4<T> TranslateMatrix(const Vector3<T>& translation);

	template <typename T>
	Matrix4x4<T> ScaleMatrix(const Vector3<T>& scale);

	template <typename T, typename F>
	Quaternion<T> RotateAxis(const Vector3<T>& axis, F angleInDegree);

	template <typename T>
	Matrix4x4<T> LookAtMatrix(const Vector3<T>& forward, const Vector3<T>& up);

	/**
	 * @brief Creates a quaternion that represents a rotation looking in the specified direction.
	 *
	 * @param forward The direction to look at (will be normalized).
	 * @param up The up direction used to determine the orientation (will be normalized).
	 * @return Quaternion representing the "look at" rotation.
	 */
	template <typename T>
	Quaternion<T> LookAtQuaternion(const Vector3<T>& forward, const Vector3<T>& up);

#pragma endregion

#pragma region[Rotation Conversion]

	template <typename T>
	Quaternion<T> EulerAngleToQuaternion(const EulerAngles<T>& euler);

	template <typename T>
	EulerAngles<T> QuaternionToEulerAngles(const Quaternion<T>& quaternion);

	template <typename T>
	Matrix4x4<T> QuaternionToRotateMatrix(const Quaternion<T>& quaternion);

	template <typename T>
	Quaternion<T> RotateMatrixToQuaternion(const Matrix4x4<T>& matrix);

	template <typename T>
	Matrix4x4<T> EulerAngleToRotateMatrix(const EulerAngles<T>& euler);

	template <typename T>
	EulerAngles<T> RotateMatrixToEulerAngle(const Matrix4x4<T>& matrix);

#pragma endregion

#pragma region[MVP Matrix]

	template <typename T>
	Matrix4x4<T> MakeModelMatrix(const Vector3<T>& pos, const Quaternion<T>& rot, const Vector3<T>& scale);

	template <typename T>
	Matrix4x4<T> MakeViewMatrix(const Vector3<T>& pos, const Quaternion<T>& rot);

	/**
	 * @brief Projection matrix of ortho camera, camera is looking at +x, frustum is symmetry by y-axis
	 * and z-axis.
	 *
	 * @remark The matrix will eventually restrict all points within the range to be between -1 and 1
	 * on all three coordinate axes.
	 *
	 * @param nearPlaneHalfY Near plane half width.
	 * @param nearPlaneHalfZ Near plane half height.
	 * @param nearPlaneX Near plane x value.
	 * @param farPlaneX Far plane x value.
	 */
	template <typename T>
	Matrix4x4<T> MakeOrthoProjectionMatrix(float nearPlaneHalfY, float nearPlaneHalfZ, float nearPlaneX, float farPlaneX);

	/**
	 * @brief Projection matrix of perspective camera, camera is looking at +x, frustum is symmetry by y-axis
	 * and z-axis.
	 *
	 * @remark The matrix will eventually restrict all points within the range to be between -1 and 1
	 * on all three coordinate axes.
	 *
	 * @param nearPlaneHalfY Near plane half width.
	 * @param nearPlaneHalfZ Near plane half height.
	 * @param nearPlaneX Near plane x value.
	 * @param farPlaneX Far plane x value.
	 */
	template <typename T>
	Matrix4x4<T> MakePerspectiveProjectionMatrix(float nearPlaneHalfY, float nearPlaneHalfZ, float nearPlaneX, float farPlaneX);

	/**
	 * @brief Get matrix that turns clips space to NDC space.
	 *
	 * @remark In OpenGL, the camera is looking at +z, up is +y, right is +x, and
	 * the depth range is [-1, 1].
	 */
	template <typename T>
	constexpr Matrix4x4<T> MakeNdcMatrixOpenGL();

	/**
	 * @brief Get matrix that turns clips space to NDC space.
	 *
	 * @remark In Vulkan, the camera is looking at +z, up is -y, right is +x, and
	 * the depth range is [0, 1].
	 */
	template <typename T>
	constexpr Matrix4x4<T> MakeNdcMatrixVulkan();

	/**
	 * @brief Get matrix that turns clips space to NDC space.
	 *
	 * @remark In DirectX, the camera is looking at -z, up is -y, right is +x, and
	 * the depth range is [0, 1].
	 */
	template <typename T>
	constexpr Matrix4x4<T> MakeNdcMatrixD3D();

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
	template <typename T, typename U> requires std::is_floating_point_v<U>
	T Lerp(const T& a, const T& b, U t);

	template <typename T> requires std::is_floating_point_v<T>
	Quaternion<T> SLerp(const Quaternion<T>& q1, const Quaternion<T>& q2, T t);

#pragma endregion

} // namespace Ailurus::Math

#include "Math.inl"