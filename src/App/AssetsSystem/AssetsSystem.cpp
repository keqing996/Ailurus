
#include "Ailurus/Application/AssetsSystem/AssetsSystem.h"
#include "Ailurus/Application/AssetsSystem/Mesh/Mesh.h"
#include "Ailurus/Utility/Logger.h"


namespace Ailurus
{

	AssetsManager::~AssetsManager()
	{
	}

	AssetsManager::AssetsManager()
	{
	}



	template <>
	AssetReference<Mesh> AssetsManager::LoadAsset<Mesh>(const std::string& path)
	{
		auto itr = _meshMap.find(path);
		if (itr != _meshMap.end())
			return AssetReference(reinterpret_cast<Mesh*>(itr->second.get()));




	}
} // namespace Ailurus