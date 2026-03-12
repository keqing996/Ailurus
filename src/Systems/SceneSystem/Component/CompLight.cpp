#include "Ailurus/Systems/SceneSystem/Component/CompLight.h"
#include <nlohmann/json.hpp>

namespace Ailurus
{
	CompLight::CompLight()
	{
	}

	LightType CompLight::GetLightType() const
	{
		return _lightType;
	}

	void CompLight::SetLightType(LightType type)
	{
		_lightType = type;
	}

	Vector3f CompLight::GetColor() const
	{
		return _color;
	}

	void CompLight::SetColor(const Vector3f& color)
	{
		_color = color;
	}

	float CompLight::GetIntensity() const
	{
		return _intensity;
	}

	void CompLight::SetIntensity(float intensity)
	{
		_intensity = intensity;
	}

	Vector3f CompLight::GetDirection() const
	{
		return _direction;
	}

	void CompLight::SetDirection(const Vector3f& direction)
	{
		_direction = direction;
	}

	Vector3f CompLight::GetAttenuation() const
	{
		return _attenuation;
	}

	void CompLight::SetAttenuation(const Vector3f& attenuation)
	{
		_attenuation = attenuation;
	}

	float CompLight::GetInnerCutoff() const
	{
		return _innerCutoff;
	}

	void CompLight::SetInnerCutoff(float degrees)
	{
		_innerCutoff = degrees;
	}

	float CompLight::GetOuterCutoff() const
	{
		return _outerCutoff;
	}

	void CompLight::SetOuterCutoff(float degrees)
	{
		_outerCutoff = degrees;
	}

	nlohmann::json CompLight::Serialize() const
	{
		nlohmann::json j;
		j["type"] = "Light";
		j["lightType"] = EnumReflection<LightType>::ToString(_lightType);
		j["color"] = { _color.x, _color.y, _color.z };
		j["intensity"] = _intensity;
		j["direction"] = { _direction.x, _direction.y, _direction.z };
		j["attenuation"] = { _attenuation.x, _attenuation.y, _attenuation.z };
		j["innerCutoff"] = _innerCutoff;
		j["outerCutoff"] = _outerCutoff;
		return j;
	}
}
