#include "Ailurus/Application/SceneSystem/Component/CompSpotLight.h"

namespace Ailurus
{
	CompSpotLight::CompSpotLight()
		: _direction(0.0f, -1.0f, 0.0f)
		, _innerConeAngle(12.5f)
		, _outerConeAngle(17.5f)
		, _range(10.0f)
		, _constantAttenuation(1.0f)
		, _linearAttenuation(0.09f)
		, _quadraticAttenuation(0.032f)
	{
	}

	CompSpotLight::CompSpotLight(const Vector3f& direction, float innerConeAngle, float outerConeAngle)
		: _direction(direction)
		, _innerConeAngle(innerConeAngle)
		, _outerConeAngle(outerConeAngle)
		, _range(10.0f)
		, _constantAttenuation(1.0f)
		, _linearAttenuation(0.09f)
		, _quadraticAttenuation(0.032f)
	{
	}

	Vector3f CompSpotLight::GetDirection() const
	{
		return _direction;
	}

	void CompSpotLight::SetDirection(const Vector3f& direction)
	{
		_direction = direction;
	}

	float CompSpotLight::GetInnerConeAngle() const
	{
		return _innerConeAngle;
	}

	void CompSpotLight::SetInnerConeAngle(float angle)
	{
		_innerConeAngle = angle;
	}

	float CompSpotLight::GetOuterConeAngle() const
	{
		return _outerConeAngle;
	}

	void CompSpotLight::SetOuterConeAngle(float angle)
	{
		_outerConeAngle = angle;
	}

	float CompSpotLight::GetRange() const
	{
		return _range;
	}

	void CompSpotLight::SetRange(float range)
	{
		_range = range;
	}

	float CompSpotLight::GetConstantAttenuation() const
	{
		return _constantAttenuation;
	}

	void CompSpotLight::SetConstantAttenuation(float value)
	{
		_constantAttenuation = value;
	}

	float CompSpotLight::GetLinearAttenuation() const
	{
		return _linearAttenuation;
	}

	void CompSpotLight::SetLinearAttenuation(float value)
	{
		_linearAttenuation = value;
	}

	float CompSpotLight::GetQuadraticAttenuation() const
	{
		return _quadraticAttenuation;
	}

	void CompSpotLight::SetQuadraticAttenuation(float value)
	{
		_quadraticAttenuation = value;
	}

	void CompSpotLight::SetAttenuation(float constant, float linear, float quadratic)
	{
		_constantAttenuation = constant;
		_linearAttenuation = linear;
		_quadraticAttenuation = quadratic;
	}

} // namespace Ailurus
