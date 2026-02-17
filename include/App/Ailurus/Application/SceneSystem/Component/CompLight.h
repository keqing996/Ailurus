#pragma once

#include "Base/Component.h"
#include "Ailurus/Math/Vector3.hpp"
#include "Ailurus/Math/Vector4.hpp"
#include "Ailurus/Utility/EnumReflection.h"

namespace Ailurus
{
	REFLECTION_ENUM(LightType,
		Directional,
		Point,
		Spot);

	class CompLight : public TComponent<ComponentType::Light, Component>
	{
	public:
		CompLight();
		~CompLight() override = default;

		// Light type
		LightType GetLightType() const;
		void SetLightType(LightType type);

		// Color (RGB, HDR allowed)
		Vector3f GetColor() const;
		void SetColor(const Vector3f& color);

		// Intensity multiplier
		float GetIntensity() const;
		void SetIntensity(float intensity);

		// Direction (for Directional and Spot lights)
		// For directional lights this is the light direction
		// For spot lights this is the cone direction
		Vector3f GetDirection() const;
		void SetDirection(const Vector3f& direction);

		// Attenuation (for Point and Spot lights)
		// x = constant, y = linear, z = quadratic
		Vector3f GetAttenuation() const;
		void SetAttenuation(const Vector3f& attenuation);

		// Spot light cone angles (in degrees)
		float GetInnerCutoff() const;  // inner cone angle
		void SetInnerCutoff(float degrees);
		float GetOuterCutoff() const;  // outer cone angle
		void SetOuterCutoff(float degrees);

	private:
		LightType _lightType = LightType::Directional;
		Vector3f _color = { 1.0f, 1.0f, 1.0f };
		float _intensity = 1.0f;
		Vector3f _direction = { 0.0f, -1.0f, 0.0f };
		Vector3f _attenuation = { 1.0f, 0.09f, 0.032f }; // constant, linear, quadratic (typical values for ~50 unit range)
		float _innerCutoff = 12.5f;  // degrees
		float _outerCutoff = 17.5f;  // degrees
	};
}
