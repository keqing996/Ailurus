
#include "Ailurus/Application/AssetsSystem/AssetsSystem.h"
#include "Ailurus/Application/AssetsSystem/Model/Model.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{

	AssetsSystem::~AssetsSystem()
	{
	}

	AssetsSystem::AssetsSystem()
	{
	}

	template <>
	AssetReference<Model> AssetsSystem::LoadAsset<Model>(const std::string& path)
	{
		auto itr = _assetsMap.find(path);
		if (itr != _assetsMap.end())
			return AssetReference(reinterpret_cast<Model*>(itr->second.get()));

		std::unique_ptr<Model> pModel = std::make_unique<Model>();
		if (pModel->LoadFromFile(path))
		{
			AssetReference ret(pModel.get());
			_assetsMap[path] = std::move(pModel);
			return ret;
		}

		Logger::LogError("Fail to load model: {}", path);
		return AssetReference<Model>(nullptr);
	}
} // namespace Ailurus