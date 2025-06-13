#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <atomic>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"
#include "AssetReference.h"
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
		AssetReference<Model> LoadModel(const std::string& path);
		AssetReference<MaterialInstance> LoadMaterial(const std::string& path);
		AssetReference<MaterialInstance> CopyMaterialInstance(const AssetReference<MaterialInstance>& materialInstance);

	private:
		friend class Application;
		AssetsSystem();

	private:
		uint64_t NextAssetId();

	private:
		std::atomic<uint64_t> _globalAssetIdCounter { 0 };
		std::unordered_map<std::string, uint64_t> _fileAssetToIdMap;
		std::unordered_map<uint64_t, std::unique_ptr<Asset>> _assetsMap;
	};
} // namespace Ailurus