#include "Ailurus/Graphics/InputAssemble/InputAssemble.h"
#include "Ailurus/Graphics/DataBuffer/VertexBuffer.h"
#include "Ailurus/Graphics/DataBuffer/IndexBuffer.h"

namespace Ailurus
{
    InputAssemble::InputAssemble(const Renderer* pRenderer, const char* vertexData, size_t vertexSize,
        const InputAttribute& inputAttr)
        : _pRenderer(pRenderer)
        , _pVertexBuffer(std::make_unique<VertexBuffer>(pRenderer, vertexData, vertexSize))
        , _inputAttr(inputAttr)
        , _pIndexBuffer(nullptr)
    {
    }

    InputAssemble::InputAssemble(const Renderer* pRenderer, const char* vertexData, size_t vertexSize, const InputAttribute& inputAttr,
            const char* indexData, size_t indexSize)
        : _pRenderer(pRenderer)
        , _pVertexBuffer(std::make_unique<VertexBuffer>(pRenderer, vertexData, vertexSize))
        , _inputAttr(inputAttr)
        , _pIndexBuffer(std::make_unique<IndexBuffer>(pRenderer, indexData, indexSize))
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
