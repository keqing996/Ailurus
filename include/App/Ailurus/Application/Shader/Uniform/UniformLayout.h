#pragma once

#include "Ailurus/Application/Shader/ShaderStage.h"
#include "UniformBindingType.h"

namespace Ailurus
{
	struct UniformLayout
	{
		size_t bindingPoint;
		UniformBindingType bindingType;
		size_t arraySize;
		std::vector<ShaderStage> usedStage;
	};
}