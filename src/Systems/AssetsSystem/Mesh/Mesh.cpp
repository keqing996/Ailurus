#include "Ailurus/Systems/AssetsSystem/Mesh/Mesh.h"
#include "VulkanContext/DataBuffer/VulkanIndexBuffer.h"
#include "VulkanContext/DataBuffer/VulkanVertexBuffer.h"

namespace Ailurus
{
	Mesh::Mesh(const void* vertexData, size_t vertexDataSizeInBytes, uint64_t vertexLayoutId, const AABBf& localAABB)
		: _pVertexBuffer(std::make_unique<VulkanVertexBuffer>(vertexData, vertexDataSizeInBytes))
		, _layoutId(vertexLayoutId)
		, _pIndexBuffer(nullptr)
		, _localAABB(localAABB)
	{
	}

	Mesh::Mesh(const void* vertexData, size_t vertexDataSizeInBytes, uint64_t vertexLayoutId,
		IndexBufferFormat format, const void* indexData, size_t indexDtaSizeInBytes, const AABBf& localAABB)
		: _pVertexBuffer(std::make_unique<VulkanVertexBuffer>(vertexData, vertexDataSizeInBytes))
		, _layoutId(vertexLayoutId)
		, _pIndexBuffer(std::make_unique<VulkanIndexBuffer>(format, indexData, indexDtaSizeInBytes))
		, _localAABB(localAABB)
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

	const AABBf& Mesh::GetLocalAABB() const
	{
		return _localAABB;
	}
} // namespace Ailurus
