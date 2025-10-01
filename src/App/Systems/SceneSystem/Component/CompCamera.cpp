#include "Ailurus/Application/SceneSystem/Component/CompCamera.h"
#include "Ailurus/Application/SceneSystem/Entity/Entity.h"
#include "Ailurus/Assert.h"

/*
 * Before projective transformation, world coordinate is right-hand, so the eye locates
 * at (0, 0, 0), looks towards -z, which means 0 > near > far. 
 * 
 * After projection, coordinate turns to the Vulkan NDC coordinate:
 * - X and Y range: [-1, 1] (left-hand coordinate)
 * - Z depth range: [0, 1] (near plane → 0, far plane → 1)
 * 
 * Note: Vulkan uses [0, 1] depth range, different from OpenGL's [-1, 1].
 */

namespace Ailurus
{
	template <typename T>
	Matrix4x4<T> MakeViewMatrix(const Vector3<T>& pos, const Quaternion<T>& rot)
	{
		Matrix4x4<T> translationMatrix = Math::TranslateMatrix(-pos);
		Matrix4x4<T> rotationMatrix = Math::QuaternionToRotateMatrix(rot.Conjugate());

		return rotationMatrix * translationMatrix;
	}

	template <typename T>
	Matrix4x4<T> MakeOrthoProjectionMatrix(float l, float r, float t, float b, float n, float f)
	{
		float a11 = 2 / (r - l);
		float a22 = 2 / (t - b);
		float a33 = 2 / (f - n);
		float a34 = -(f + n) / (f - n);
		float a44 = 1;

		return -1 * Matrix4x4<T>{
				{ a11,	0,		0,		0	},
				{ 0,	a22,	0,		0	},
				{ 0,	0,		a33,	a34 },
				{ 0,	0,		0,		a44	}
		};
	}

	template <typename T>
	Matrix4x4<T> MakePerspectiveProjectionMatrix(float leftCoord, float rightCoord, float topCoord, float bottomCoord, float nearAbs, float farAbs)
	{
		// For right-handed view space (looking towards -Z):
		// near plane at z = -nearAbs, far plane at z = -farAbs
		// Maps to Vulkan NDC depth range [0, 1]: near→0, far→1
		float a11 = 2 * nearAbs / (rightCoord - leftCoord);
		float a22 = 2 * nearAbs / (topCoord - bottomCoord);
		float a33 = -farAbs / (farAbs - nearAbs);          // Note the negative sign
		float a34 = -(farAbs * nearAbs) / (farAbs - nearAbs);
		float a43 = -1;

		Matrix4x4f projMat = Matrix4x4<T>{
			{ a11, 0, 0, 0 },
			{ 0, a22, 0, 0 },
			{ 0, 0, a33, a34 },
			{ 0, 0, a43, 0 }
		};

		return projMat;
	}

	CompCamera::CompCamera(float widthInWorldCoord, float heightInWorldCoord, float nearPlaneDist, float farPlaneDist)
		: _isPerspective(true)
	{
		ASSERT_MSG(widthInWorldCoord > 0, "Camera width should be greater than 0");
		ASSERT_MSG(heightInWorldCoord > 0, "Camera height should be greater than 0");
		ASSERT_MSG(nearPlaneDist > 0, "Near plane distance should be greater than 0");
		ASSERT_MSG(farPlaneDist > 0, "Far plane distance should be greater than 0");

		const float left = -widthInWorldCoord / 2.0f;
		const float right = widthInWorldCoord / 2.0f;
		const float top = heightInWorldCoord / 2.0f;
		const float bottom = -heightInWorldCoord / 2.0f;

		Set(left, right, top, bottom, nearPlaneDist, farPlaneDist);
	}

	CompCamera::CompCamera(const CameraFieldOfView& fieldOfViewSetting, float nearPlaneDist, float farPlaneDist)
		: _isPerspective(true)
	{
		// todo
	}

	bool CompCamera::IsPerspective() const
	{
		return _isPerspective;
	}

	void CompCamera::SetPerspective(bool isPerspective)
	{
		_isPerspective = isPerspective;
	}

	float CompCamera::GetHorizontalFOV() const
	{
		return Math::RadianToDegree(_fovHorizontal);
	}

	float CompCamera::GetAspectRatio() const
	{
		return _aspect;
	}

	Matrix4x4f CompCamera::GetProjectionMatrix() const
	{
		if (_isPerspective)
			return MakePerspectiveProjectionMatrix<float>(_left, _right, _top, _bottom, _nearPlaneDist, _farPlaneDist);
		else
			return MakeOrthoProjectionMatrix<float>(_left, _right, _top, _bottom, _nearPlaneDist, _farPlaneDist);
	}

	Matrix4x4f CompCamera::GetViewMatrix() const
	{
		return MakeViewMatrix(_parentEntity->GetPosition(), _parentEntity->GetRotation());
	}

	void CompCamera::Set(float l, float r, float t, float b, float n, float f)
	{
		_left = l;
		_right = r;
		_top = t;
		_bottom = b;
		_nearPlaneDist = n;
		_farPlaneDist = f;

		_aspect = (r - l) / (t - b);
		_fovHorizontal = 2 * std::atan((r - l) / (2 * n));
	}

	void CompCamera::Set(float fovHorizontal, float aspect, float n, float f)
	{
		_fovHorizontal = Math::DegreeToRadian(fovHorizontal) / 2;
		_aspect = aspect;
		_nearPlaneDist = n;
		_farPlaneDist = f;

		_right = n * std::tan(_fovHorizontal);
		_left = -_right;
		_top = _aspect * _right;
		_bottom = -_top;
	}
}