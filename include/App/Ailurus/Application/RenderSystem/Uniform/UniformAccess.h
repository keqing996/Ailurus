#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include "Ailurus/Application/RenderSystem/Uniform/UniformValue.h"

namespace Ailurus
{
    struct UniformAccess
	{
		uint32_t bindingId;
		std::string access;
	};

	struct UniformAccessHash
	{
		size_t operator()(const UniformAccess& entry) const;
	};

	struct UniformAccessEqual
	{
		bool operator()(const UniformAccess& lhs, const UniformAccess& rhs) const;
	};

    using UniformValueMap = std::unordered_map<UniformAccess, UniformValue, 
        UniformAccessHash, UniformAccessEqual>;
}