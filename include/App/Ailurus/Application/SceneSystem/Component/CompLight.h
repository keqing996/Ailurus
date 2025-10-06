#pragma once

#include "Base/Component.h"
#include <Ailurus/Math/Math.hpp>

namespace Ailurus
{
	class CompLight : public TComponent<ComponentType::Light, Component>
	{
	public:
		CompLight() = default;
		~CompLight() override = default;

	public:
		Vector3f GetColor() const;
		void SetColor(const Vector3f& color);

		float GetIntensity() const;
		void SetIntensity(float intensity);

		bool IsCastingShadow() const;
		void SetCastingShadow(bool castShadow);

	protected:
		Vector3f _color = Vector3f(1.0f, 1.0f, 1.0f);
		float _intensity = 1.0f;
		bool _castShadow = true;
	};

} // namespace Ailurus
