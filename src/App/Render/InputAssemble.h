#pragma once

#include <memory>
#include <vulkan/vulkan.hpp>
#include "Ailurus/Application/Render/InputAttribute.h"

namespace Ailurus
{
	class VertexBuffer;
	class IndexBuffer;

	class InputAssemble
	{
	public:
		InputAssemble(std::unique_ptr<VertexBuffer>&& pVertexBuffer, const InputAttribute& inputAttr);
		InputAssemble(std::unique_ptr<VertexBuffer>&& pVertexBuffer, const InputAttribute& inputAttr,
			std::unique_ptr<IndexBuffer>&& pIndexBuffer);
		~InputAssemble();

	public:
		const VertexBuffer* GetVertexBuffer() const;
		const InputAttribute& GetInputAttribute() const;
		const IndexBuffer* GetIndexBuffer() const;
		size_t GetVertexCount() const;
		std::vector<vk::VertexInputAttributeDescription> GetAttributeDescription() const;

	private:
		std::unique_ptr<VertexBuffer> _pVertexBuffer;
		InputAttribute _inputAttr;
		std::unique_ptr<IndexBuffer> _pIndexBuffer;
	};
} // namespace Ailurus