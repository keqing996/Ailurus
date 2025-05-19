#pragma once

#include "Ailurus/Utility/EnumReflection.h"

namespace Ailurus
{
	REFLECTION_ENUM(ComponentType,
		Component,
		Render,
		MeshRender,
		Light,
		Camera)
}