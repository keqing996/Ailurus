#pragma once

#include <cstdint>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"

namespace Ailurus
{
	class Asset: public NonCopyable, public NonMovable
	{
	public:
		virtual ~Asset() = default;

	public:
		int32_t GetRefCount() const
		{
			return _refCount;
		}

		void AddRef()
		{
			++_refCount;
		}

		void RemoveRef()
		{
			--_refCount;
		}

	protected:
		int32_t _refCount = 0;
	};

} // namespace Ailurus