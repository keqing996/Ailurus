#include "Ailurus/Application/SceneSystem/Component/CompPointLight.h"

namespace Ailurus
{
	CompPointLight::CompPointLight()
		: _range(10.0f)
		, _constantAttenuation(1.0f)
		, _linearAttenuation(0.09f)
		, _quadraticAttenuation(0.032f)
	{
	}

	CompPointLight::CompPointLight(float range)
		: _range(range)
		, _constantAttenuation(1.0f)
		, _linearAttenuation(0.09f)
		, _quadraticAttenuation(0.032f)
	{
	}

	float CompPointLight::GetRange() const
	{
		return _range;
	}

	void CompPointLight::SetRange(float range)
	{
		_range = range;
	}

	float CompPointLight::GetConstantAttenuation() const
	{
		return _constantAttenuation;
	}

	void CompPointLight::SetConstantAttenuation(float value)
	{
		_constantAttenuation = value;
	}

	float CompPointLight::GetLinearAttenuation() const
	{
		return _linearAttenuation;
	}

	void CompPointLight::SetLinearAttenuation(float value)
	{
		_linearAttenuation = value;
	}

	float CompPointLight::GetQuadraticAttenuation() const
	{
		return _quadraticAttenuation;
	}

	void CompPointLight::SetQuadraticAttenuation(float value)
	{
		_quadraticAttenuation = value;
	}

	void CompPointLight::SetAttenuation(float constant, float linear, float quadratic)
	{
		_constantAttenuation = constant;
		_linearAttenuation = linear;
		_quadraticAttenuation = quadratic;
	}

} // namespace Ailurus
