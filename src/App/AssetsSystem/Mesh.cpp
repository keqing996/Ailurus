#include "Ailurus/Application/AssetsSystem/Mesh.h"
#include "VulkanSystem/Buffer/VulkanIndexBuffer.h"
#include "VulkanSystem/Buffer/VulkanVertexBuffer.h"

namespace Ailurus
{

	Mesh::Mesh(const void* vertexData, size_t vertexDataSizeInBytes, const VertexAttributeDescription& vertexDataAttribute)
		: _pVertexBuffer(std::make_unique<VulkanVertexBuffer>(vertexData, vertexDataSizeInBytes))
		, _vertexAttrDesc(vertexDataAttribute)
		, _pIndexBuffer(nullptr)
	{
	}

	Mesh::Mesh(const void* vertexData, size_t vertexDataSizeInBytes, const VertexAttributeDescription& vertexDataAttribute,
		IndexBufferFormat format, const void* indexData, size_t indexDtaSizeInBytes)
		: _pVertexBuffer(std::make_unique<VulkanVertexBuffer>(vertexData, vertexDataSizeInBytes))
		, _vertexAttrDesc(vertexDataAttribute)
		, _pIndexBuffer(std::make_unique<VulkanIndexBuffer>(format, indexData, indexDtaSizeInBytes))
	{
	}

	Mesh::~Mesh() = default;

	const VulkanVertexBuffer* Mesh::GetVertexBuffer() const
	{
		return _pVertexBuffer.get();
	}

	const VertexAttributeDescription& Mesh::GetInputAttribute() const
	{
		return _vertexAttrDesc;
	}

	const VulkanIndexBuffer* Mesh::GetIndexBuffer() const
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
