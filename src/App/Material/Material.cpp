#include "Ailurus/Application/Material/Material.h"

namespace Ailurus
{
    void Material::SetShader(RenderPassType pass, ShaderStage stage, const std::string& shader)
    {
        _renderPassParaMap[pass].shaderMap[stage] = shader;
    }

    const std::string& Material::GetShader(RenderPassType pass, ShaderStage stage)
    {
        auto const passItr = _renderPassParaMap.find(pass);
        if (passItr == _renderPassParaMap.end())
            return "";

        auto& shaderMap = passItr->second.shaderMap;
        if (const auto shaderItr = shaderMap.find(stage); shaderItr != shaderMap.end())
            return shaderItr->second;

        return "";
    }
}
