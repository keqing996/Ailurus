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
        InputAssemble(const Renderer* pRenderer, std::unique_ptr<VertexBuffer>&& pVertexBuffer,
            const InputAttribute& inputAttr);
        InputAssemble(const Renderer* pRenderer, std::unique_ptr<VertexBuffer>&& pVertexBuffer,
            const InputAttribute& inputAttr, std::unique_ptr<IndexBuffer>&& pIndexBuffer);
        ~InputAssemble();

    public:
        const VertexBuffer* GetVertexBuffer() const;
        const InputAttribute& GetInputAttribute() const;
        const IndexBuffer* GetIndexBuffer() const;
        size_t GetVertexCount() const;

    private:
        const Renderer* _pRenderer;
        std::unique_ptr<VertexBuffer> _pVertexBuffer;
        InputAttribute _inputAttr;
        std::unique_ptr<IndexBuffer> _pIndexBuffer;
    };
}