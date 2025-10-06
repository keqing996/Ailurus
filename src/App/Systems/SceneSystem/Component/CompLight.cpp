#include "Ailurus/Application/SceneSystem/Component/CompLight.h"

namespace Ailurus
{
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

	bool CompLight::IsCastingShadow() const
	{
		return _castShadow;
	}

	void CompLight::SetCastingShadow(bool castShadow)
	{
		_castShadow = castShadow;
	}

} // namespace Ailurus
