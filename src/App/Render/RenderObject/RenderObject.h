#pragma once

#include <optional>
#include <memory>
#include <unordered_map>
#include "Ailurus/Graphics/RenderPass/RenderPassType.h"
#include "Ailurus/Graphics/Shader/ShaderStage.h"
#include "Ailurus/Graphics/InputAssemble/InputAssemble.h"

namespace Ailurus
{
    class Render;
    class Shader;

    class RenderObject
    {
    public:
        RenderObject(const Render* pRenderer);
        ~RenderObject();

    public:
        void SetInputAssemble(std::unique_ptr<InputAssemble>&& pInputAssemble);
        void SetRenderPassShader(RenderPassType passType, const Shader* pShader);

        template <RenderPassType passType>
        void SetRenderPassShader(const Shader* pShader)
        {
            SetRenderPassShader(passType, pShader);
        }

        const InputAssemble* GetInputAssemble() const;
        std::optional<const PipelineShaderStages*> GetRenderPassShaders(RenderPassType passType) const;

        template<RenderPassType passType>
        std::optional<const PipelineShaderStages*> GetRenderPassShaders() const
        {
            return GetRenderPassShaders(passType);
        }

    private:
        const Render* _pRenderer;
        std::unique_ptr<InputAssemble> _pInputAssemble;
        std::unordered_map<RenderPassType, PipelineShaderStages> _renderPassShaderMap;
    };
}