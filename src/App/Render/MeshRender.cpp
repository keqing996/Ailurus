#include "Ailurus/Application/Render/MeshRender.h"
#include "InputAssemble.h"
#include "Vulkan/DataBuffer/VertexBuffer.h"
#include "Vulkan/DataBuffer/IndexBuffer.h"

namespace Ailurus
{
    MeshRender::MeshRender(const char* vertexData, size_t vertexDataSizeInBytes,
        const InputAttribute& vertexDataAttribute)
    {
        _pInputAssemble = std::make_unique<InputAssemble>(
            std::make_unique<VertexBuffer>(vertexData, vertexDataSizeInBytes),
            vertexDataAttribute);
    }

    MeshRender::MeshRender(const char* vertexData, size_t vertexDataSizeInBytes,
        const InputAttribute& vertexDataAttribute, IndexBufferFormat format, const char* indexData,
        size_t indexDtaSizeInBytes)
    {
        _pInputAssemble = std::make_unique<InputAssemble>(
            std::make_unique<VertexBuffer>(vertexData, vertexDataSizeInBytes),
            vertexDataAttribute,
            std::make_unique<IndexBuffer>(format, indexData, indexDtaSizeInBytes));
    }

    void MeshRender::SetMaterial(const Material* pMat)
    {
        _pMaterial = pMat;
    }
}
