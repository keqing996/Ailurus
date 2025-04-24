#pragma once

#include <cstdint>
#include "Ailurus/Utility/EnumReflection.h"

namespace Ailurus
{
	REFLECTION_ENUM(VkResourceType,
		Buffer,
		Shader)

	struct IVulkanResource
	{
		virtual VkResourceType GetResourceType() const = 0;
		virtual uint64_t GetHash() const = 0;
		virtual void DestroyResource() = 0;
	};
} // namespace Ailurus