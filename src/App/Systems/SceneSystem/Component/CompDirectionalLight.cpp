#include "Ailurus/Application/SceneSystem/Component/CompDirectionalLight.h"

namespace Ailurus
{
	CompDirectionalLight::CompDirectionalLight()
		: _direction(0.0f, -1.0f, 0.0f)
	{
	}

	CompDirectionalLight::CompDirectionalLight(const Vector3f& direction)
		: _direction(direction)
	{
	}

	Vector3f CompDirectionalLight::GetDirection() const
	{
		return _direction;
	}

	void CompDirectionalLight::SetDirection(const Vector3f& direction)
	{
		_direction = direction;
	}

} // namespace Ailurus
