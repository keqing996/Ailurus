#pragma once

#include <memory>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"
#include "Ailurus/Application/RenderSystem/Vertex/IndexBufferFormat.h"

namespace Ailurus
{
	class VulkanVertexBuffer;
	class VulkanIndexBuffer;

	class Mesh: public NonCopyable, public NonMovable
	{
	public:
		Mesh(const void* vertexData, size_t vertexDataSizeInBytes, uint64_t vertexLayoutId);
		Mesh(const void* vertexData, size_t vertexDataSizeInBytes, uint64_t vertexLayoutId,
			IndexBufferFormat format, const void* indexData, size_t indexDtaSizeInBytes);
		~Mesh();

	public:
		const VulkanVertexBuffer* GetVertexBuffer() const;
		uint64_t GetVertexLayoutId() const;
		const VulkanIndexBuffer* GetIndexBuffer() const;

	private:
		std::unique_ptr<VulkanVertexBuffer> _pVertexBuffer;
		uint64_t _layoutId;
		std::unique_ptr<VulkanIndexBuffer> _pIndexBuffer;
	};
} // namespace Ailurus