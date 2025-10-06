#pragma once

#include "CompLight.h"

namespace Ailurus
{
	class CompSpotLight : public TComponent<ComponentType::SpotLight, CompLight>
	{
	public:
		CompSpotLight();
		CompSpotLight(const Vector3f& direction, float innerConeAngle, float outerConeAngle);
		~CompSpotLight() override = default;

	public:
		Vector3f GetDirection() const;
		void SetDirection(const Vector3f& direction);

		float GetInnerConeAngle() const;
		void SetInnerConeAngle(float angle);

		float GetOuterConeAngle() const;
		void SetOuterConeAngle(float angle);

		float GetRange() const;
		void SetRange(float range);

		float GetConstantAttenuation() const;
		void SetConstantAttenuation(float value);

		float GetLinearAttenuation() const;
		void SetLinearAttenuation(float value);

		float GetQuadraticAttenuation() const;
		void SetQuadraticAttenuation(float value);

		void SetAttenuation(float constant, float linear, float quadratic);

	private:
		Vector3f _direction = Vector3f(0.0f, -1.0f, 0.0f);
		float _innerConeAngle = 12.5f;
		float _outerConeAngle = 17.5f;
		float _range = 10.0f;
		float _constantAttenuation = 1.0f;
		float _linearAttenuation = 0.09f;
		float _quadraticAttenuation = 0.032f;
	};

} // namespace Ailurus
