#pragma once

#include <memory>
#include "VertexAttributeDescription.h"

namespace Ailurus
{
	class VertexBuffer;
	class IndexBuffer;

	class Mesh
	{
	public:
		Mesh(const void* vertexData, size_t vertexDataSizeInBytes, const VertexAttributeDescription& vertexDataAttribute);
		Mesh(const void* vertexData, size_t vertexDataSizeInBytes, const VertexAttributeDescription& vertexDataAttribute,
			IndexBufferFormat format, const void* indexData, size_t indexDtaSizeInBytes);
		~Mesh();

	public:
		const VertexBuffer* GetVertexBuffer() const;
		const VertexAttributeDescription& GetInputAttribute() const;
		const IndexBuffer* GetIndexBuffer() const;
		size_t GetVertexCount() const;

	private:
		std::unique_ptr<VertexBuffer> _pVertexBuffer;
		VertexAttributeDescription _vertexAttrDesc;
		std::unique_ptr<IndexBuffer> _pIndexBuffer;
	};
} // namespace Ailurus