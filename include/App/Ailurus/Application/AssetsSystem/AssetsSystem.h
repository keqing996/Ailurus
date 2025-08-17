#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <atomic>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"
#include "AssetRef.h"
#include "Model/Model.h"
#include "Material/MaterialInstance.h"

namespace Ailurus
{
	class Mesh;

	class AssetsSystem: public NonCopyable, public NonMovable
	{
	public:
		~AssetsSystem();

	public:
		AssetRef<Model> LoadModel(const std::string& path);
		AssetRef<MaterialInstance> LoadMaterial(const std::string& path);
		AssetRef<MaterialInstance> CopyMaterialInstance(const AssetRef<MaterialInstance>& materialInstance);

		template <typename AssetType>
		AssetRef<AssetType> GetAsset(uint64_t assetId) const;

	private:
		friend class Application;
		AssetsSystem();

	private:
		uint64_t NextAssetId();
		Asset* GetAssetRaw(AssetType type, uint64_t assetId) const;

	private:
		std::atomic<uint64_t> _globalAssetIdCounter { 0 };
		std::unordered_map<std::string, uint64_t> _fileAssetToIdMap;
		std::unordered_map<uint64_t, std::unique_ptr<Asset>> _assetsMap;
	};

	template <typename AssetType>
	AssetRef<AssetType> AssetsSystem::GetAsset(uint64_t assetId) const
	{
		Asset* pAsset = GetAssetRaw(AssetType::StaticAssetType(), assetId);
		if (pAsset == nullptr)
			return AssetRef<AssetType>(nullptr);

		return AssetRef<AssetType>(static_cast<AssetType*>(pAsset));
	}
} // namespace Ailurus