#include "Ailurus/Application/Material/Material.h"
#include "Ailurus/Application/Application.h"

namespace Ailurus
{
    void Material::SetShader(RenderPassType pass, ShaderStage stage, const std::string& shader)
	{
    	const Shader* pShader = Application::GetShaderManager().GetShader(stage, shader);
		_renderPassParaMap[pass].stageShaders[stage] = pShader;
	}

	std::optional<StageShaderArray> Material::GetStageShaderArray(RenderPassType pass) const
	{
    	auto const passItr = _renderPassParaMap.find(pass);
    	if (passItr == _renderPassParaMap.end())
    		return std::nullopt;

    	return passItr->second.stageShaders;
	}
}
