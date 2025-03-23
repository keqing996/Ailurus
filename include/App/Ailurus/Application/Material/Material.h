#pragma once

#include <unordered_map>
#include "Ailurus/Application/RenderPass/RenderPassType.h"
#include "Ailurus/Application/Shader/ShaderStage.h"
#include "Ailurus/Utility/NonCopyable.h"

namespace Ailurus
{
    class Shader;

    class Material: public NonCopyable
    {
        struct RenderPassParameters
        {
            StageShaderArray stageShaders;
            // todo uniform
        };
    public:
        void SetShader(RenderPassType pass, ShaderStage stage, const std::string& shader);
        std::optional<StageShaderArray> GetStageShaderArray(RenderPassType pass) const;

    private:
         std::unordered_map<RenderPassType, RenderPassParameters> _renderPassParaMap;
    };
}