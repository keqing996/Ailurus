#pragma once

#include <memory>
#include <unordered_map>
#include "Ailurus/Application/RenderSystem/RenderPass/RenderPassType.h"
#include "Ailurus/Application/RenderSystem/Shader/ShaderStage.h"
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"

namespace Ailurus
{
	class Shader;
	class VulkanDescriptorSet;

	class Material : public NonCopyable, public NonMovable
	{
		struct RenderPassParameters
		{
			StageShaderArray stageShaders;
			// todo uniform
		};
	public:
		Material();
		~Material();

	public:
		void SetShader(RenderPassType pass, ShaderStage stage, const std::string& shader);
		std::optional<StageShaderArray> GetStageShaderArray(RenderPassType pass) const;
		const VulkanDescriptorSet* GetDescriptorSet() const;

	private:
		std::unique_ptr<VulkanDescriptorSet> _pVkDescriptorSet;
		std::unordered_map<RenderPassType, RenderPassParameters> _renderPassParaMap;
	};
} // namespace Ailurus