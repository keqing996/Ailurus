#pragma once

#include "CompLight.h"

namespace Ailurus
{
	class CompAreaLight : public TComponent<ComponentType::AreaLight, CompLight>
	{
	public:
		CompAreaLight();
		CompAreaLight(const Vector3f& width, const Vector3f& height);
		~CompAreaLight() override = default;

	public:
		Vector3f GetWidth() const;
		void SetWidth(const Vector3f& width);

		Vector3f GetHeight() const;
		void SetHeight(const Vector3f& height);

		void SetSize(const Vector3f& width, const Vector3f& height);

		bool IsTwoSided() const;
		void SetTwoSided(bool isTwoSided);

	private:
		Vector3f _width = Vector3f(1.0f, 0.0f, 0.0f);
		Vector3f _height = Vector3f(0.0f, 1.0f, 0.0f);
		bool _isTwoSided = false;
	};

} // namespace Ailurus
