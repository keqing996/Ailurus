#include "Ailurus/Utility/EnumReflection.h"
#include "Ailurus/Utility/Logger.h"
#include "Ailurus/Application/AssetsSystem/AssetsSystem.h"

namespace Ailurus
{

	AssetsSystem::~AssetsSystem()
	{
	}

	AssetsSystem::AssetsSystem()
	{
	}

	uint64_t AssetsSystem::NextAssetId()
	{
		return _globalAssetIdCounter.fetch_add(1, std::memory_order_seq_cst);
	}

	Asset* AssetsSystem::GetAssetRaw(AssetType type, uint64_t assetId) const
	{
		auto it = _assetsMap.find(assetId);
		if (it == _assetsMap.end())
			return nullptr;

		auto* pAsset = it->second.get();
		if (pAsset->GetAssetType() != type)
		{
			Logger::LogError("AssetsSystem::GetAssetRaw: Asset type mismatch for asset ID {}. Expected {}, got {}.",
				assetId, EnumReflection<AssetType>::ToString(type), 
				EnumReflection<AssetType>::ToString(pAsset->GetAssetType()));
			return nullptr;
		}

		return pAsset;
	}

	void AssetsSystem::RegisterAsset(uint64_t assetId, std::unique_ptr<Asset>&& pAsset)
	{
		_assetsMap[assetId] = std::move(pAsset);
	}
} // namespace Ailurus