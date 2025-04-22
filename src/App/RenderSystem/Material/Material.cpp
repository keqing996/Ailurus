#include "Ailurus/Application/RenderSystem/Material/Material.h"
#include "Ailurus/Application/Application.h"
#include "RenderSystem/Descriptor/DescriptorSet.h"

namespace Ailurus
{
	Material::Material() = default;

	Material::~Material() = default;

    void Material::SetShader(RenderPassType pass, ShaderStage stage, const std::string& shader)
	{
    	const Shader* pShader = Application::Get<RenderSystem>()->GetShaderLibrary()->GetShader(stage, shader);
		_renderPassParaMap[pass].stageShaders[stage] = pShader;
	}

	std::optional<StageShaderArray> Material::GetStageShaderArray(RenderPassType pass) const
	{
		auto const passItr = _renderPassParaMap.find(pass);
		if (passItr == _renderPassParaMap.end())
			return std::nullopt;

		return passItr->second.stageShaders;
	}

	const DescriptorSet* Material::GetDescriptorSet() const
	{
    	return _pVkDescriptorSet.get();
	}
} // namespace Ailurus
