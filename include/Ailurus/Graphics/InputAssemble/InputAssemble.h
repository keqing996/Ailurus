#pragma once

#include <memory>
#include <vector>
#include <optional>
#include "InputAttribute.h"

namespace Ailurus
{
    class Renderer;
    class VertexBuffer;
    class IndexBuffer;

    class InputAssemble
    {
    public:
        InputAssemble(const Renderer* pRenderer, const std::vector<char>& vertexData, const InputAttribute& inputAttr);
        InputAssemble(const Renderer* pRenderer, const std::vector<char>& vertexData, const InputAttribute& inputAttr, const std::vector<char>& indexData);
        ~InputAssemble();

    public:
        const VertexBuffer* GetVertexBuffer() const;
        const std::optional<InputAttribute>& GetInputAttribute() const;
        const IndexBuffer* GetIndexBuffer() const;

    private:
        const Renderer* _pRenderer;
        std::unique_ptr<VertexBuffer> _pVertexBuffer;
        InputAttribute _inputAttr;
        std::unique_ptr<IndexBuffer> _pIndexBuffer;
    };
}