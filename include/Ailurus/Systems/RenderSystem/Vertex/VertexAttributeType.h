#pragma once

#include "Ailurus/Utility/EnumReflection.h"

namespace Ailurus
{
    REFLECTION_ENUM(AttributeType,
		Position,
		Normal,
		TexCoord,
		Tangent,
		Bitangent,
		Color);
}