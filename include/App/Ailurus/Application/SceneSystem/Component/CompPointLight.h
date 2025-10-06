#pragma once

#include "CompLight.h"

namespace Ailurus
{
	class CompPointLight : public TComponent<ComponentType::PointLight, CompLight>
	{
	public:
		CompPointLight();
		explicit CompPointLight(float range);
		~CompPointLight() override = default;

	public:
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
		float _range = 10.0f;
		float _constantAttenuation = 1.0f;
		float _linearAttenuation = 0.09f;
		float _quadraticAttenuation = 0.032f;
	};

} // namespace Ailurus
