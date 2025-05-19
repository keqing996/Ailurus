#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"
#include "AssetReference.h"

namespace Ailurus
{
	class Mesh;

	class AssetsManager: public NonCopyable, public NonMovable
	{
	public:
		~AssetsManager();

	public:
		template <typename T> requires std::derived_from<T, Asset>
		AssetReference<T> LoadAsset(const std::string& path);

	private:
		friend class Application;
		AssetsManager();

	private:
		std::unordered_map<std::string, std::unique_ptr<Asset>> _meshMap;
	};
} // namespace Ailurus