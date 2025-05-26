#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include "Ailurus/Application/RenderSystem/RenderPass/RenderPassType.h"
#include "Ailurus/Application/RenderSystem/Shader/ShaderStage.h"
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"
#include "UniformVariable.h"

namespace Ailurus
{
	class Shader;

	class Material : public NonCopyable, public NonMovable
	{
		using UniformBindingPointMap = std::unordered_map<uint32_t, std::unique_ptr<MaterialUniformVariable>>;

		struct RenderPassParameters
		{
			StageShaderArray stageShaders;
			std::unordered_map<uint32_t, UniformBindingPointMap> uniformSetsMap;
		};

	public:
		bool LoadFromFile(const std::string& path);
		const RenderPassParameters* GetRenderPassParameters(RenderPassType pass) const;

	private:
		std::unordered_map<RenderPassType, RenderPassParameters> _renderPassParaMap;
	};
} // namespace Ailurus