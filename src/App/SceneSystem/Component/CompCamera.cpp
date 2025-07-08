#include "Ailurus/Application/SceneSystem/Component/CompCamera.h"
#include "Ailurus/Application/SceneSystem/Entity/Entity.h"

namespace Ailurus
{
	CompCamera::CompCamera(float l, float r, float t, float b, float n, float f, bool isPerspective)
		: _isPerspective(isPerspective)
	{
		Set(l, r, t, b, n, f);
	}

	CompCamera::CompCamera(float fovHorizontal, float aspect, float n, float f, bool isPerspective)
		: _isPerspective(isPerspective)
	{
		Set(fovHorizontal, aspect, n, f);
	}

	float CompCamera::GetLeft() const
	{
		return _left;
	}

	float CompCamera::GetRight() const
	{
		return _right;
	}

	float CompCamera::GetTop() const
	{
		return _top;
	}

	float CompCamera::GetBottom() const
	{
		return _bottom;
	}

	float CompCamera::GetNear() const
	{
		return _near;
	}

	float CompCamera::GetFar() const
	{
		return _far;
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

	void CompCamera::Set(float l, float r, float t, float b, float n, float f)
	{
		_left = l;
		_right = r;
		_top = t;
		_bottom = b;
		_near = n;
		_far = f;

		_aspect = (r - l) / (t - b);
		_fovHorizontal = 2 * std::atan((r - l) / (2 * n));
	}

	void CompCamera::Set(float fovHorizontal, float aspect, float n, float f)
	{
		_fovHorizontal = Math::DegreeToRadian(fovHorizontal) / 2;
		_aspect = aspect;
		_near = n;
		_far = f;

		_right = n * std::tan(_fovHorizontal);
		_left = -_right;
		_top = _aspect * _right;
		_bottom = -_top;
	}

	Matrix4x4f CompCamera::GetProjectionMatrix()
	{
		if (_isPerspective)
			return Math::MakePerspectiveProjectionMatrix<float>(_left, _right, _bottom, _top, _near, _far);
		else
			return Math::MakeOrthoProjectionMatrix<float>(_left, _right, _bottom, _top, _near, _far);
	}

	Matrix4x4f CompCamera::GetViewMatrix()
	{
		const Matrix4x4f translation = Math::TranslateMatrix(_parentEntity->GetPosition() * -1);
		const Matrix4x4f rotation = Math::QuaternionToRotateMatrix(_parentEntity->GetRotation()).Transpose();
		return translation * rotation;
	}
}