#pragma once

#include <string>
#include <nlohmann/json_fwd.hpp>

namespace Ailurus
{
	class Entity;
	class SceneSystem;
	class AssetsSystem;

	class SceneSerializer
	{
	public:
		static nlohmann::json SerializeScene(const SceneSystem& scene);
		static void DeserializeScene(SceneSystem& scene, AssetsSystem& assets, const nlohmann::json& json);

		static nlohmann::json SerializeEntity(const Entity& entity);

		static void SaveToFile(const SceneSystem& scene, const std::string& filePath);
		static void LoadFromFile(SceneSystem& scene, AssetsSystem& assets, const std::string& filePath);
	};

} // namespace Ailurus
