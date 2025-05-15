#include "Ailurus/Application/SceneSystem/Component/CompRender.h"

namespace Ailurus
{

	ComponentType CompRender::GetType() const
	{
		return ComponentType::Render;
	}
} // namespace Ailurus