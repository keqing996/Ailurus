#pragma once

#include "Ailurus/Graphics/Shader/ShaderStage.h"

namespace Ailurus
{
    class InputAssemble;

    struct PipelineConfig
    {
        const InputAssemble* pInputAssemble;
        PipelineShaderStages shaderStages;
    };
}