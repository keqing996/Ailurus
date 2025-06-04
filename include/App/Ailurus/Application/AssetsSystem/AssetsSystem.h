#pragma once

#include <string>
#include <memory>
#include <unordered_map>
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

	private:
		friend class Application;
		AssetsSystem();

	private:
		std::unordered_map<std::string, std::unique_ptr<Asset>> _assetsMap;
	};
} // namespace Ailurus