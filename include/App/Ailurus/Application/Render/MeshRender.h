#pragma once

#include <memory>
#include "Render.h"
#include "InputAttribute.h"

namespace Ailurus
{
    class InputAssemble;
    class Material;

    class MeshRender: public Render
    {
    public:
        MeshRender(const char* vertexData, size_t vertexDataSizeInBytes, const InputAttribute& vertexDataAttribute);
        MeshRender(const char* vertexData, size_t vertexDataSizeInBytes, const InputAttribute& vertexDataAttribute,
            IndexBufferFormat format, const char* indexData, size_t indexDtaSizeInBytes);

        ~MeshRender() override = default;

    public:
        void SetMaterial(const Material* pMat);

    private:
        std::unique_ptr<InputAssemble> _pInputAssemble = nullptr;
        const Material* _pMaterial = nullptr;
    };
}