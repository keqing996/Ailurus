#pragma once

#include "Ailurus/Application/RenderSystem/Shader/ShaderStage.h"

namespace Ailurus
{
	struct RhiPipelineConfig
	{
		const class Mesh* pMesh;
		StageShaderArray shaderStages;

		bool operator==(const RhiPipelineConfig& rhs) const
		{
			return pMesh == rhs.pMesh && shaderStages == rhs.shaderStages;
		}

		struct Hash
		{
			size_t operator()(const RhiPipelineConfig& p) const
			{
				const unsigned char* ptr = reinterpret_cast<const unsigned char*>(&p);
				size_t size = sizeof(RhiPipelineConfig);
				size_t hash_value = 0;
				for (size_t i = 0; i < size; ++i)
					hash_value = hash_value * 31 + ptr[i]; // 31 is a small prime number

				return hash_value;
			}
		};
	};
} // namespace Ailurus