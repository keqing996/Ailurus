#pragma once

#include "Ailurus/Graphics/InputAssemble/InputAttribute.h"
#include "Ailurus/Graphics/Shader/ShaderStage.h"

namespace Ailurus
{
    struct PipelineConfig
    {
        InputAttribute inputAttribute;
        PipelineShaderStages shaderStages;
    };
}