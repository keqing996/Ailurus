#include "Ailurus/Application/AssetsSystem/Mesh/Mesh.h"
#include "VulkanSystem/Buffer/VulkanIndexBuffer.h"
#include "VulkanSystem/Buffer/VulkanVertexBuffer.h"

namespace Ailurus
{
	Mesh::Mesh(const void* vertexData, size_t vertexDataSizeInBytes, uint64_t vertexLayoutId)
		: _pVertexBuffer(std::make_unique<VulkanVertexBuffer>(vertexData, vertexDataSizeInBytes))
		, _layoutId(vertexLayoutId)
		, _pIndexBuffer(nullptr)
	{
	}

	Mesh::Mesh(const void* vertexData, size_t vertexDataSizeInBytes, uint64_t vertexLayoutId,
		IndexBufferFormat format, const void* indexData, size_t indexDtaSizeInBytes)
		: _pVertexBuffer(std::make_unique<VulkanVertexBuffer>(vertexData, vertexDataSizeInBytes))
		, _layoutId(vertexLayoutId)
		, _pIndexBuffer(std::make_unique<VulkanIndexBuffer>(format, indexData, indexDtaSizeInBytes))
	{
	}

	Mesh::~Mesh() = default;

	const VulkanVertexBuffer* Mesh::GetVertexBuffer() const
	{
		return _pVertexBuffer.get();
	}

	uint64_t Mesh::GetVertexLayoutId() const
	{
		return _layoutId;
	}

	const VulkanIndexBuffer* Mesh::GetIndexBuffer() const
	{
		return _pIndexBuffer.get();
	}

	uint32_t Mesh::GetVertexCount() const
	{
		return _vertexCount;
	}
} // namespace Ailurus
