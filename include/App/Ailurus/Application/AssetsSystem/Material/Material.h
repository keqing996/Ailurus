#pragma once

#include <memory>
#include <unordered_map>
#include "Ailurus/Application/RenderSystem/RenderPass/RenderPassType.h"
#include "Ailurus/Application/RenderSystem/Shader/ShaderStage.h"
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"
#include "UniformVariable/MaterialUniformVariable.h"

namespace Ailurus
{
	class Shader;

	class Material : public NonCopyable, public NonMovable
	{
		struct RenderPassParameters
		{
			StageShaderArray stageShaders;
			std::vector<std::unique_ptr<MaterialUniformVariable>> uniformVariables;
		};

	public:
		bool LoadFromFile(const std::string& path);
		const RenderPassParameters* GetRenderPassParameters(RenderPassType pass) const;

	private:
		std::unordered_map<RenderPassType, RenderPassParameters> _renderPassParaMap;
	};
} // namespace Ailurus