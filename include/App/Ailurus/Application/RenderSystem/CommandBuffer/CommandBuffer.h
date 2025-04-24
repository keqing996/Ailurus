#pragma once

#include <memory>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"

namespace Ailurus
{
	class VulkanCommandBuffer;

	class CommandBuffer: public NonCopyable, public NonMovable
	{
	public:
		enum class Timing
		{
			BeforeEveryThing,
		};

	public:
		CommandBuffer();
		~CommandBuffer();

	public:
		const VulkanCommandBuffer* GetImpl() const;

	private:
		std::unique_ptr<VulkanCommandBuffer> _pImpl;
	};
}