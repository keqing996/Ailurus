#pragma once

#include <cstdint>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"

namespace Ailurus
{
	class Asset: public NonCopyable, public NonMovable
	{
	public:
		Asset(uint64_t assetId)
			: _assetId(assetId)
		{
		}

		virtual ~Asset() = default;

	public:
		uint64_t GetAssetId() const
		{
			return _assetId;
		}

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
		uint64_t _assetId;
	};

} // namespace Ailurus