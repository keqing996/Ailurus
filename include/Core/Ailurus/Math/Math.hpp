#pragma once

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
	template <typename T> requires std::is_floating_point_v<T>
	T DegreeToRadian(T degrees);

	/**
	 * @brief Converts an angle from radians to degrees.
	 *
	 * @param radians The angle in radians to convert.
	 * @return The equivalent angle in degrees.
	 */
	template <typename T> requires std::is_floating_point_v<T>
	T RadianToDegree(T radians);

	/**
	 * @brief Creates a translation matrix from a vector.
	 *
	 * @param translation The translation vector.
	 * @return Matrix4x4 representing the translation.
	 */
	template <typename T>
	Matrix4x4<T> TranslateMatrix(const Vector3<T>& translation);

	/**
	 * @brief Creates a scale matrix from a vector.
	 *
	 * @param scale The scale vector.
	 * @return Matrix4x4 representing the scaling.
	 */
	template <typename T>
	Matrix4x4<T> ScaleMatrix(const Vector3<T>& scale);

	/**
	 * @brief Creates a quaternion that represents a rotation around a specific axis.
	 *
	 * @param axis The axis to rotate around (will be normalized).
	 * @param angleInDegree The rotation angle in degrees.
	 * @return Quaternion representing the rotation.
	 */
	template <typename T, typename F>
	Quaternion<T> RotateAxis(const Vector3<T>& axis, F angleInDegree);

	/**
	 * @brief Creates a rotation matrix that orients an object to look in the specified direction.
	 *
	 * @param forward The direction to look at (will be normalized).
	 * @param up The up direction used to determine the orientation (will be normalized).
	 * @return Matrix4x4 representing the "look at" rotation.
	 */
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

	/**
	 * @brief Converts Euler angles to a quaternion.
	 *
	 * @param euler The Euler angles to convert.
	 * @return Quaternion representing the same rotation.
	 */
	template <typename T>
	Quaternion<T> EulerAngleToQuaternion(const EulerAngles<T>& euler);

	/**
	 * @brief Converts a quaternion to Euler angles.
	 *
	 * @param quaternion The quaternion to convert.
	 * @return Euler angles representing the same rotation.
	 */
	template <typename T>
	EulerAngles<T> QuaternionToEulerAngles(const Quaternion<T>& quaternion);

	/**
	 * @brief Converts a quaternion to a rotation matrix.
	 *
	 * @param quaternion The quaternion to convert.
	 * @return Matrix4x4 representing the same rotation.
	 */
	template <typename T>
	Matrix4x4<T> QuaternionToRotateMatrix(const Quaternion<T>& quaternion);

	/**
	 * @brief Converts a rotation matrix to a quaternion.
	 *
	 * @param matrix The rotation matrix to convert.
	 * @return Quaternion representing the same rotation.
	 */
	template <typename T>
	Quaternion<T> RotateMatrixToQuaternion(const Matrix4x4<T>& matrix);

	/**
	 * @brief Converts Euler angles to a rotation matrix.
	 *
	 * @param euler The Euler angles to convert.
	 * @return Matrix4x4 representing the same rotation.
	 */
	template <typename T>
	Matrix4x4<T> EulerAngleToRotateMatrix(const EulerAngles<T>& euler);

	/**
	 * @brief Converts a rotation matrix to Euler angles.
	 *
	 * @param matrix The rotation matrix to convert.
	 * @return Euler angles representing the same rotation.
	 */
	template <typename T>
	EulerAngles<T> RotateMatrixToEulerAngle(const Matrix4x4<T>& matrix);

#pragma endregion

#pragma region[MVP Matrix]

	/**
	 * @brief Creates a model matrix from position, rotation, and scale.
	 *
	 * @param pos The position vector.
	 * @param rot The rotation quaternion.
	 * @param scale The scale vector.
	 * @return Matrix4x4 representing the model transformation.
	 */
	template <typename T>
	Matrix4x4<T> MakeModelMatrix(const Vector3<T>& pos, const Quaternion<T>& rot, const Vector3<T>& scale);

	/**
	 * @brief Creates a view matrix from position and rotation.
	 *
	 * @param pos The camera position.
	 * @param rot The camera rotation quaternion.
	 * @return Matrix4x4 representing the view transformation.
	 */
	template <typename T>
	Matrix4x4<T> MakeViewMatrix(const Vector3<T>& pos, const Quaternion<T>& rot);

	/**
	 * @brief Projection matrix of ortho camera, camera is looking at +x, frustum is symmetry by y-axis
	 * and z-axis.
	 *
	 * @remark The matrix will eventually restrict all points within the range to be between -1 and 1
	 * on all three coordinate axes.
	 *
	 * @param nearPlaneHalfY Near plane half-width.
	 * @param nearPlaneHalfZ Near plane half-height.
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
	 * @param nearPlaneHalfY Near plane half-width.
	 * @param nearPlaneHalfZ Near plane half-height.
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

	/**
	 * @brief Spherically interpolates between two quaternions.
	 *
	 * @param q1 The start quaternion when t = 0.
	 * @param q2 The end quaternion when t = 1.
	 * @param t The interpolation parameter (ideally between 0 and 1).
	 * @return The spherically interpolated quaternion.
	 */
	template <typename T> requires std::is_floating_point_v<T>
	Quaternion<T> SLerp(const Quaternion<T>& q1, const Quaternion<T>& q2, T t);

#pragma endregion

} // namespace Ailurus::Math

#include "Math.inl"