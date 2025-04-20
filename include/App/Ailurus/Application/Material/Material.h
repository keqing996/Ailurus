#pragma once

#include <memory>
#include <unordered_map>
#include "Ailurus/Application/RenderPass/RenderPassType.h"
#include "Ailurus/Application/Shader/ShaderStage.h"
#include "Ailurus/Utility/NonCopyable.h"

namespace Ailurus
{
	class Shader;
	class DescriptorSet;

	class Material : public NonCopyable
	{
		struct RenderPassParameters
		{
			StageShaderArray stageShaders;
			// todo uniform
		};

	public:
		void SetShader(RenderPassType pass, ShaderStage stage, const std::string& shader);
		std::optional<StageShaderArray> GetStageShaderArray(RenderPassType pass) const;
		const DescriptorSet* GetDescriptorSet() const;

	private:
		std::unique_ptr<DescriptorSet> _pVkDescriptorSet;
		std::unordered_map<RenderPassType, RenderPassParameters> _renderPassParaMap;
	};
} // namespace Ailurus