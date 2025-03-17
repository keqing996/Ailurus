#pragma once

#include <unordered_map>
#include "Ailurus/Application/RenderPass/RenderPassType.h"
#include "Ailurus/Application/Shader/ShaderStage.h"
#include "Ailurus/Utility/NonCopyable.h"

namespace Ailurus
{
    class Material: public NonCopyable
    {
        struct RenderPassParameters
        {
            std::unordered_map<ShaderStage, std::string> shaderMap;
            // todo uniform
        };
    public:
        void SetShader(RenderPassType pass, ShaderStage stage, const std::string& shader);
        const std::string& GetShader(RenderPassType pass, ShaderStage stage);

    private:
         std::unordered_map<RenderPassType, RenderPassParameters> _renderPassParaMap;
    };
}