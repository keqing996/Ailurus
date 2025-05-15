#pragma once

#include "Ailurus/Utility/EnumReflection.h"

namespace Ailurus
{
	REFLECTION_ENUM(UniformDataType,
		None,
		Bool,
		Float,
		Float2,
		Float3,
		Float4,
		Matrix3x3f,
		Matrix4x4f,
		Int,
		Int2,
		Int3,
		Int4)
}