#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include "Ailurus/Application/RenderSystem/RenderPass/RenderPassType.h"
#include "Ailurus/Application/RenderSystem/Uniform/UniformValue.h"

namespace Ailurus
{
	struct MaterialUniformAccess
	{
		RenderPassType pass;
		uint32_t bindingId;
		std::string access;
	};

	struct MaterialUniformAccessHash
	{
		size_t operator()(const MaterialUniformAccess& entry) const;
	};

	struct MaterialUniformAccessEqual
	{
		bool operator()(const MaterialUniformAccess& lhs, const MaterialUniformAccess& rhs) const;
	};

    using MaterialUniformValueMap = std::unordered_map<MaterialUniformAccess, UniformValue, 
        MaterialUniformAccessHash, MaterialUniformAccessEqual>;
} // namespace Ailurus