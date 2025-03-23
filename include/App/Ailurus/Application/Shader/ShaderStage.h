#pragma once

#include <vulkan/vulkan.hpp>
#include "Ailurus/Utility/EnumReflection.h"

namespace Ailurus
{
	REFLECTION_ENUM(ShaderStage,
		Vertex,
		Fragment)

	class Shader;

	struct StageShaderArray
	{
		const Shader* shaders[EnumReflection<ShaderStage>::Size()];

		const Shader* operator[](ShaderStage stage) const
		{
			return shaders[static_cast<int>(stage)];
		}

		const Shader*& operator[](ShaderStage stage)
		{
			return shaders[static_cast<int>(stage)];
		}

		bool operator==(const StageShaderArray& rhs) const
		{
			for (auto i = 0; i < EnumReflection<ShaderStage>::Size(); i++)
			{
				if (shaders[i] != rhs.shaders[i])
					return false;
			}

			return true;
		}

		bool operator!=(const StageShaderArray& rhs) const
		{
			return !(*this == rhs);
		}
	};
} // namespace Ailurus