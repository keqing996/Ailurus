#pragma once

#include <memory>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"
#include "Ailurus/Systems/RenderSystem/Vertex/IndexBufferFormat.h"
#include "Ailurus/Math/AABB.hpp"

namespace Ailurus
{
	class VulkanVertexBuffer;
	class VulkanIndexBuffer;

	class Mesh: public NonCopyable, public NonMovable
	{
	public:
		Mesh(const void* vertexData, size_t vertexDataSizeInBytes, uint64_t vertexLayoutId, const AABBf& localAABB);
		Mesh(const void* vertexData, size_t vertexDataSizeInBytes, uint64_t vertexLayoutId,
			IndexBufferFormat format, const void* indexData, size_t indexDtaSizeInBytes, const AABBf& localAABB);
		~Mesh();

	public:
		uint32_t GetVertexCount() const;
		const VulkanVertexBuffer* GetVertexBuffer() const;
		uint64_t GetVertexLayoutId() const;
		const VulkanIndexBuffer* GetIndexBuffer() const;
		const AABBf& GetLocalAABB() const;

	private:
		uint32_t _vertexCount;
		std::unique_ptr<VulkanVertexBuffer> _pVertexBuffer;
		uint64_t _layoutId;
		std::unique_ptr<VulkanIndexBuffer> _pIndexBuffer;
		AABBf _localAABB;
	};
} // namespace Ailurus