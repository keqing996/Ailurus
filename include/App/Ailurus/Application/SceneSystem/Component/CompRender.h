#pragma once

#include <memory>
#include "BaseComponent.h"

namespace Ailurus
{
	class CompRender : public Component
	{
	public:
		ComponentType GetType() const override;
	};
} // namespace Ailurus