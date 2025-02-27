#pragma once

#include "Ailurus/Graphics/Shader/ShaderStage.h"

namespace Ailurus
{
    class InputAssemble;

    struct PipelineConfig
    {
        const InputAssemble* pInputAssemble;
        PipelineShaderStages shaderStages;

        bool operator==(const PipelineConfig& rhs) const
        {
            return pInputAssemble == rhs.pInputAssemble && shaderStages == rhs.shaderStages;
        }

        struct Hash
        {
            size_t operator()(const PipelineConfig& p) const
            {
                const unsigned char* ptr = reinterpret_cast<const unsigned char*>(&p);
                size_t size = sizeof(PipelineConfig);
                size_t hash_value = 0;
                for (size_t i = 0; i < size; ++i)
                    hash_value = hash_value * 31 + ptr[i];  // 31 is a small prime number

                return hash_value;
            }
        };
    };
}