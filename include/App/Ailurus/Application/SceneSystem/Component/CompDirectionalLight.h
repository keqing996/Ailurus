#pragma once

#include "CompLight.h"

namespace Ailurus
{
	class CompDirectionalLight : public TComponent<ComponentType::DirectionalLight, CompLight>
	{
	public:
		CompDirectionalLight();
		explicit CompDirectionalLight(const Vector3f& direction);
		~CompDirectionalLight() override = default;

	public:
		Vector3f GetDirection() const;
		void SetDirection(const Vector3f& direction);

	private:
		Vector3f _direction = Vector3f(0.0f, -1.0f, 0.0f);
	};

} // namespace Ailurus
