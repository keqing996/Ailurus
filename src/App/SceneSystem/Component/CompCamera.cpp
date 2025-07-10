#include "Ailurus/Application/SceneSystem/Component/CompCamera.h"
#include "Ailurus/Application/SceneSystem/Entity/Entity.h"

namespace Ailurus
{
	template <typename T>
	Matrix4x4<T> MakeViewMatrix(const Vector3<T>& pos, const Quaternion<T>& rot)
	{
		Matrix4x4<T> translationMatrix = TranslateMatrix(-pos);
		Matrix4x4<T> rotationMatrix = QuaternionToRotateMatrix(rot.Conjugate());

		return rotationMatrix * translationMatrix;
	}

	template <typename T>
	Matrix4x4<T> MakeOrthoProjectionMatrix(float l, float r, float t, float b, float n, float f)
	{
		Matrix4x4<T> translationMatrix = Math::TranslateMatrix(Vector3<T>{
			-(r + l) / 2,
			-(t + b) / 2,
			-(n + f) / 2 });

		Matrix4x4<T> scaleMatrix = Math::ScaleMatrix(Vector3<T>{
			2 / (r - l),
			2 / (t - b),
			2 / (f - n) });

		Matrix4x4<T> standardOrthoProj = scaleMatrix * translationMatrix;

		return standardOrthoProj;
	}

	template <typename T>
	Matrix4x4<T> MakePerspectiveProjectionMatrix(float l, float r, float t, float b, float n, float f)
	{
		Matrix4x4<T> standardOrthoProj = MakeOrthoProjectionMatrix<T>(l, r, t, b, n, f);

		Matrix4x4<T> compressMatrix = {
			{ n, 0, 0, 0 },
			{ 0, n, 0, 0 },
			{ 0, 0, n + f, -f * n },
			{ 0, 0, 1, 0 }
		};

		return standardOrthoProj * compressMatrix;
	}

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
			return MakePerspectiveProjectionMatrix<float>(_left, _right, _bottom, _top, _near, _far);
		else
			return MakeOrthoProjectionMatrix<float>(_left, _right, _bottom, _top, _near, _far);
	}

	Matrix4x4f CompCamera::GetViewMatrix()
	{
		const Matrix4x4f translation = Math::TranslateMatrix(_parentEntity->GetPosition() * -1);
		const Matrix4x4f rotation = Math::QuaternionToRotateMatrix(_parentEntity->GetRotation()).Transpose();
		return translation * rotation;
	}
}