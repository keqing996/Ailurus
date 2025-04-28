#pragma once

#include <cstdint>
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
		void Release();
		void MarkDelete();
		bool IsValid() const;
		bool GetRefCount() const;
		virtual uint32_t GetHash() = 0;

	protected:
		VulkanResource();

	private:
		bool _markDeleted = false;
		int _refCount = 0;
	};
} // namespace Ailurus