#include "Ailurus/Graphics/InputAssemble/InputAssemble.h"
#include "Ailurus/Graphics/DataBuffer/VertexBuffer.h"
#include "Ailurus/Graphics/DataBuffer/IndexBuffer.h"

namespace Ailurus
{
    InputAssemble::InputAssemble(const Renderer* pRenderer, const std::vector<char>& vertexData,
        const InputAttribute& inputAttr)
        : _pRenderer(pRenderer)
        , _pVertexBuffer(std::make_unique<VertexBuffer>(pRenderer, vertexData))
        , _inputAttr(inputAttr)
        , _pIndexBuffer(nullptr)
    {
    }

    InputAssemble::InputAssemble(const Renderer* pRenderer, const std::vector<char>& vertexData,
        const InputAttribute& inputAttr, const std::vector<char>& indexData)
        : _pRenderer(pRenderer)
        , _pVertexBuffer(std::make_unique<VertexBuffer>(pRenderer, vertexData))
        , _inputAttr(inputAttr)
        , _pIndexBuffer(std::make_unique<IndexBuffer>(pRenderer, indexData))
    {
    }

    InputAssemble::~InputAssemble()
    {
    }

    const VertexBuffer* InputAssemble::GetVertexBuffer() const
    {
        return _pVertexBuffer.get();
    }

    const InputAttribute& InputAssemble::GetInputAttribute() const
    {
        return _inputAttr;
    }

    const IndexBuffer* InputAssemble::GetIndexBuffer() const
    {
        return _pIndexBuffer.get();
    }
}
