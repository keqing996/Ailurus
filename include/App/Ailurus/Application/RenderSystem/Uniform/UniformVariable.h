#pragma once

#include <variant>
#include <cstdint>
#include "Ailurus/Utility/EnumReflection.h"
#include "Ailurus/Math/Vector2.hpp"
#include "Ailurus/Math/Vector3.hpp"
#include "Ailurus/Math/Vector4.hpp"
#include "Ailurus/Math/Matrix4x4.hpp"

namespace Ailurus
{
	REFLECTION_ENUM(UniformVariableType,
		Int,
		Uint,
		Float,
		Float2,
		Float3,
		Float4,
		Mat3,
		Mat4)

	using UniformValue = std::variant<int32_t, float, Vector2f, Vector3f, Vector4f, Matrix4x4f>;

	struct UniformVariableNumeric;
	struct UniformVariableStructure;

	using UniformVariable = std::variant<UniformVariableNumeric, UniformVariableStructure>;

	struct UniformVariableNumeric
	{
		std::string name;
		UniformValue value;
	};

	struct UniformVariableStructure
	{
		std::string name;
		std::vector<UniformVariable> members;
	};

} // namespace Ailurus