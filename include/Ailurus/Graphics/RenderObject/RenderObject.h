#pragma once

#include <optional>
#include <memory>
#include <unordered_map>
#include "Ailurus/Graphics/RenderPass/RenderPassType.h"
#include "Ailurus/Graphics/Shader/ShaderStage.h"

namespace Ailurus
{
    class Renderer;
    class Shader;
    class InputAssemble;

    class RenderObject
    {
    public:
        RenderObject(const Renderer* pRenderer);
        ~RenderObject();

    public:
        void SetInputAssemble(std::unique_ptr<InputAssemble>&& pInputAssemble);
        void AddShaderToRenderPass(RenderPassType passType, const Shader* pShader);
        std::optional<const PipelineShaderStages&> GetRenderPassShaders(RenderPassType passType) const;

    private:
        const Renderer* _pRenderer;
        std::unique_ptr<InputAssemble> _pInputAssemble;
        std::unordered_map<RenderPassType, PipelineShaderStages> _renderPassShaderMap;
    };
}