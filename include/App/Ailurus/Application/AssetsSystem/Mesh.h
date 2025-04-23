#pragma once

#include <memory>
#include "VertexAttributeDescription.h"

namespace Ailurus
{
	class VulkanVertexBuffer;
	class VulkanIndexBuffer;

	class Mesh
	{
	public:
		Mesh(const void* vertexData, size_t vertexDataSizeInBytes, const VertexAttributeDescription& vertexDataAttribute);
		Mesh(const void* vertexData, size_t vertexDataSizeInBytes, const VertexAttributeDescription& vertexDataAttribute,
			IndexBufferFormat format, const void* indexData, size_t indexDtaSizeInBytes);
		~Mesh();

	public:
		const VulkanVertexBuffer* GetVertexBuffer() const;
		const VertexAttributeDescription& GetInputAttribute() const;
		const VulkanIndexBuffer* GetIndexBuffer() const;
		size_t GetVertexCount() const;

	private:
		std::unique_ptr<VulkanVertexBuffer> _pVertexBuffer;
		VertexAttributeDescription _vertexAttrDesc;
		std::unique_ptr<VulkanIndexBuffer> _pIndexBuffer;
	};
} // namespace Ailurus