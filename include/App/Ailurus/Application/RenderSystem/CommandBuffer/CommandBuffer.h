#pragma once

#include <memory>
#include "Ailurus/Utility/NonCopyable.h"

namespace Ailurus
{
	class RhiCommandBuffer;

	class CommandBuffer: public NonCopyable
	{
	public:
		enum class Timing
		{
			BeforeEveryThing,
		};

	public:
		CommandBuffer();
		~CommandBuffer() override;

	public:
		const RhiCommandBuffer* GetImpl() const;

	private:
		std::unique_ptr<RhiCommandBuffer> _pImpl;
	};
}