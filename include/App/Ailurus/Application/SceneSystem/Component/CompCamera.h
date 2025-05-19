#pragma once

#include "Base/Component.h"
#include <Ailurus/Math/Math.hpp>

namespace Ailurus
{
	class CompCamera : public TComponent<ComponentType::Camera, Component>
	{
	public:
		CompCamera(float l, float r, float t, float b, float n, float f, bool isPerspective = true);
		CompCamera(float fovHorizontal, float aspect, float n, float f, bool isPerspective = true);

	public:
		float GetLeft() const;
		float GetRight() const;
		float GetTop() const;
		float GetBottom() const;
		float GetNear() const;
		float GetFar() const;
		bool IsPerspective() const;
		void SetPerspective(bool isPerspective);
		float GetHorizontalFOV() const;
		float GetAspectRatio() const;
		const Matrix4x4f& GetProjectionMatrix();

		void Set(float l, float r, float t, float b, float n, float f);
		void Set(float fovHorizontal, float aspect, float n, float f);

	private:
		// bool
		bool _isPerspective;

		// camera frustum
		float _left = 0;
		float _right = 0;
		float _top = 0;
		float _bottom = 0;
		float _near = 0;
		float _far = 0;

		float _fovHorizontal = 0;
		float _aspect = 0;

		// projection matrix
		bool _needUpdateProjectionMatrix = true;
		Matrix4x4f _projectionMatrix;
	};
} // namespace Ailurus