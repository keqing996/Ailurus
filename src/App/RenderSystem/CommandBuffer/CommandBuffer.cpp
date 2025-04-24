#include "Ailurus/Application/RenderSystem/CommandBuffer/CommandBuffer.h"
#include "VulkanSystem/CommandBuffer/VulkanCommandBuffer.h"

namespace Ailurus
{
	CommandBuffer::CommandBuffer()
	{
		_pImpl = std::make_unique<VulkanCommandBuffer>();
	}

	CommandBuffer::~CommandBuffer()
	{
	}

	const VulkanCommandBuffer* CommandBuffer::GetImpl() const
	{
		return _pImpl.get();
	}
} // namespace Ailurus