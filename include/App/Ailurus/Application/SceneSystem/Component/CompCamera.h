#pragma once

#include "Base/Component.h"
#include <Ailurus/Math/Math.hpp>

namespace Ailurus
{
	struct CameraFieldOfView
	{
		float horizontalFieldOfView;

		// width / height
		float rationWidthByHeight;
	};

	class CompCamera : public TComponent<ComponentType::Camera, Component>
	{
	public:
		CompCamera(float widthInWorldCoord, float heightInWorldCoord, float nearPlaneDist, float farPlaneDist);
		CompCamera(const CameraFieldOfView& fieldOfViewSetting, float nearPlaneDist, float farPlaneDist);

	public:
		bool IsPerspective() const;
		void SetPerspective(bool isPerspective);
		float GetHorizontalFOV() const;
		float GetAspectRatio() const;
		Matrix4x4f GetProjectionMatrix() const;
		Matrix4x4f GetViewMatrix() const;

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
		float _nearPlaneDist = 0;
		float _farPlaneDist = 0;

		float _fovHorizontal = 0;
		float _aspect = 0;
	};
} // namespace Ailurus