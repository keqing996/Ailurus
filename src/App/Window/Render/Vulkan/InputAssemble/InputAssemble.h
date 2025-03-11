#pragma once

#include <memory>
#include "InputAttribute.h"

namespace Ailurus
{
    class Render;
    class VertexBuffer;
    class IndexBuffer;

    class InputAssemble
    {
    public:
        InputAssemble(const Render* pRenderer, std::unique_ptr<VertexBuffer>&& pVertexBuffer,
            const InputAttribute& inputAttr);
        InputAssemble(const Render* pRenderer, std::unique_ptr<VertexBuffer>&& pVertexBuffer,
            const InputAttribute& inputAttr, std::unique_ptr<IndexBuffer>&& pIndexBuffer);
        ~InputAssemble();

    public:
        const VertexBuffer* GetVertexBuffer() const;
        const InputAttribute& GetInputAttribute() const;
        const IndexBuffer* GetIndexBuffer() const;
        size_t GetVertexCount() const;

    private:
        const Render* _pRenderer;
        std::unique_ptr<VertexBuffer> _pVertexBuffer;
        InputAttribute _inputAttr;
        std::unique_ptr<IndexBuffer> _pIndexBuffer;
    };
}