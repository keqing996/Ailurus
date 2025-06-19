#pragma once

#include <cstdint>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"
#include "AssetType.h"

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

		virtual AssetType GetAssetType() const = 0;

	protected:
		int32_t _refCount = 0;
		uint64_t _assetId;
	};

	template <AssetType Type>
	class TypedAsset : public Asset
	{
	public:
		AssetType GetAssetType() const override
		{
			return Type;
		}

		static AssetType StaticAssetType()
		{
			return Type;
		}
	};

} // namespace Ailurus