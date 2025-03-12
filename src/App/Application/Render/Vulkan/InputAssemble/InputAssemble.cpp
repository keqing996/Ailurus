#include "Ailurus/Graphics/InputAssemble/InputAssemble.h"
#include "Ailurus/Graphics/DataBuffer/VertexBuffer.h"
#include "Ailurus/Graphics/DataBuffer/IndexBuffer.h"

namespace Ailurus
{
    InputAssemble::InputAssemble(const Renderer* pRenderer, std::unique_ptr<VertexBuffer>&& pVertexBuffer,
        const InputAttribute& inputAttr)
        : _pRenderer(pRenderer)
        , _pVertexBuffer(std::move(pVertexBuffer))
        , _inputAttr(inputAttr)
        , _pIndexBuffer(nullptr)
    {
    }

    InputAssemble::InputAssemble(const Renderer* pRenderer, std::unique_ptr<VertexBuffer>&& pVertexBuffer,
        const InputAttribute& inputAttr, std::unique_ptr<IndexBuffer>&& pIndexBuffer)
        : _pRenderer(pRenderer)
        , _pVertexBuffer(std::move(pVertexBuffer))
        , _inputAttr(inputAttr)
        , _pIndexBuffer(std::move(pIndexBuffer))
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

    size_t InputAssemble::GetVertexCount() const
    {
        auto vertexSizeBytes = _pVertexBuffer->GetSize();
        auto stride = _inputAttr.GetStride();
        return vertexSizeBytes /stride;
    }
}
