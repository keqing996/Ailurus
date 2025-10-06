#include "Ailurus/Application/SceneSystem/Component/CompAreaLight.h"

namespace Ailurus
{
	CompAreaLight::CompAreaLight()
		: _width(1.0f, 0.0f, 0.0f)
		, _height(0.0f, 1.0f, 0.0f)
		, _isTwoSided(false)
	{
	}

	CompAreaLight::CompAreaLight(const Vector3f& width, const Vector3f& height)
		: _width(width)
		, _height(height)
		, _isTwoSided(false)
	{
	}

	Vector3f CompAreaLight::GetWidth() const
	{
		return _width;
	}

	void CompAreaLight::SetWidth(const Vector3f& width)
	{
		_width = width;
	}

	Vector3f CompAreaLight::GetHeight() const
	{
		return _height;
	}

	void CompAreaLight::SetHeight(const Vector3f& height)
	{
		_height = height;
	}

	void CompAreaLight::SetSize(const Vector3f& width, const Vector3f& height)
	{
		_width = width;
		_height = height;
	}

	bool CompAreaLight::IsTwoSided() const
	{
		return _isTwoSided;
	}

	void CompAreaLight::SetTwoSided(bool isTwoSided)
	{
		_isTwoSided = isTwoSided;
	}

} // namespace Ailurus
