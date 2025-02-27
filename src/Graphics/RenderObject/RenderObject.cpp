#include "Ailurus/Graphics/RenderObject/RenderObject.h"
#include "Ailurus/Graphics/DataBuffer/VertexBuffer.h"
#include "Ailurus/Graphics/DataBuffer/IndexBuffer.h"

namespace Ailurus
{
    RenderObject::RenderObject(const Renderer* pRenderer)
        : _pRenderer(pRenderer)
    {
    }

    RenderObject::~RenderObject()
    {
    }

    void RenderObject::SetInputAssemble(std::unique_ptr<InputAssemble>&& pInputAssemble)
    {
        _pInputAssemble = std::move(pInputAssemble);
    }

    std::optional<const PipelineShaderStages&> RenderObject::GetRenderPassShaders(RenderPassType passType) const
    {
        const auto itr = _renderPassShaderMap.find(passType);
        if (itr == _renderPassShaderMap.end())
            return std::nullopt;

        return itr->second;
    }

    void RenderObject::SetRenderPassShader(RenderPassType passType, const Shader* pShader)
    {
        _renderPassShaderMap[passType][static_cast<size_t>(pShader->GetStage())] = pShader;
    }
}
