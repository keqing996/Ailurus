#pragma once

#include <memory>
#include "InputAttribute.h"

namespace Ailurus
{
    class Renderer;
    class VertexBuffer;
    class IndexBuffer;

    class InputAssemble
    {
    public:
        InputAssemble(const Renderer* pRenderer, const char* vertexData, size_t vertexSize, const InputAttribute& inputAttr);
        InputAssemble(const Renderer* pRenderer, const char* vertexData, size_t vertexSize, const InputAttribute& inputAttr,
            const char* indexData, size_t indexSize);
        ~InputAssemble();

    public:
        const VertexBuffer* GetVertexBuffer() const;
        const InputAttribute& GetInputAttribute() const;
        const IndexBuffer* GetIndexBuffer() const;

    private:
        const Renderer* _pRenderer;
        std::unique_ptr<VertexBuffer> _pVertexBuffer;
        InputAttribute _inputAttr;
        std::unique_ptr<IndexBuffer> _pIndexBuffer;
    };
}