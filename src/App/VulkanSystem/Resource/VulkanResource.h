#pragma once

#include <cstdint>
#include <unordered_set>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"

namespace Ailurus
{
	class VulkanResource: public NonCopyable, public NonMovable
	{
		friend class VulkanResourceManager;
	public:
		virtual ~VulkanResource();

	public:
		void AddRef();
		void MarkDelete();
		bool IsValid() const;
		virtual uint32_t GetHash() = 0;

	protected:
		VulkanResource();

	private:
		bool _markDeleted = false;
		std::unordered_set<uint64_t> _referencedFrames;
	};
} // namespace Ailurus