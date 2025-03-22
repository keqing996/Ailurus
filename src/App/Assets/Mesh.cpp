#include "Ailurus/Application/Assets/Mesh.h"
#include "Vulkan/DataBuffer/IndexBuffer.h"
#include "Vulkan/DataBuffer/VertexBuffer.h"

namespace Ailurus
{

	Mesh::Mesh(const char* vertexData, size_t vertexDataSizeInBytes, const VertexAttributeDescription& vertexDataAttribute)
		: _pVertexBuffer(std::make_unique<VertexBuffer>(vertexData, vertexDataSizeInBytes))
		, _vertexAttrDesc(vertexDataAttribute)
		, _pIndexBuffer(nullptr)
	{
	}

	Mesh::Mesh(const char* vertexData, size_t vertexDataSizeInBytes, const VertexAttributeDescription& vertexDataAttribute, IndexBufferFormat format, const char* indexData, size_t indexDtaSizeInBytes)
		: _pVertexBuffer(std::make_unique<VertexBuffer>(vertexData, vertexDataSizeInBytes))
		, _vertexAttrDesc(vertexDataAttribute)
		, _pIndexBuffer(std::make_unique<IndexBuffer>(format, indexData, indexDtaSizeInBytes))
	{
	}

	Mesh::~Mesh() = default;

	const VertexBuffer* Mesh::GetVertexBuffer() const
	{
		return _pVertexBuffer.get();
	}

	const VertexAttributeDescription& Mesh::GetInputAttribute() const
	{
		return _vertexAttrDesc;
	}

	const IndexBuffer* Mesh::GetIndexBuffer() const
	{
		return _pIndexBuffer.get();
	}

	size_t Mesh::GetVertexCount() const
	{
		auto vertexSizeBytes = _pVertexBuffer->GetSize();
		auto stride = _vertexAttrDesc.GetStride();
		return vertexSizeBytes / stride;
	}
} // namespace Ailurus
