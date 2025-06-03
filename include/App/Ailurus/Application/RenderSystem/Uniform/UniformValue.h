#pragma once

#include <variant>
#include "Ailurus/Utility/EnumReflection.h"
#include "Ailurus/Math/Vector2.hpp"
#include "Ailurus/Math/Vector3.hpp"
#include "Ailurus/Math/Vector4.hpp"
#include "Ailurus/Math/Matrix4x4.hpp"

namespace Ailurus
{
    REFLECTION_ENUM(UniformValueType,
		Int,
		Float,
		Vector2f,
		Vector3f,
		Vector4f,
		Mat3,
		Mat4)

	using UniformValue = std::variant<int32_t, float, Vector2f, Vector3f, Vector4f, Matrix4x4f>;
}