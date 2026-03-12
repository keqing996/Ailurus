#include "Ailurus/Systems/SceneSystem/SceneSerializer.h"
#include "Ailurus/Systems/SceneSystem/SceneSystem.h"
#include "Ailurus/Systems/SceneSystem/Entity/Entity.h"
#include "Ailurus/Systems/SceneSystem/Component/CompCamera.h"
#include "Ailurus/Systems/SceneSystem/Component/CompLight.h"
#include "Ailurus/Systems/SceneSystem/Component/CompStaticMeshRender.h"
#include "Ailurus/Systems/AssetsSystem/AssetsSystem.h"
#include "Ailurus/Utility/Logger.h"
#include <nlohmann/json.hpp>
#include <fstream>

namespace Ailurus
{
	nlohmann::json SceneSerializer::SerializeEntity(const Entity& entity)
	{
		nlohmann::json j;
		j["guid"] = entity.GetGuid();
		j["name"] = entity.GetName();

		const Entity* parent = entity.GetParent();
		j["parent"] = parent ? nlohmann::json(parent->GetGuid()) : nlohmann::json(nullptr);

		// Transform
		const auto pos = entity.GetPosition();
		const auto rot = entity.GetRotation();
		const auto scl = entity.GetScale();
		j["transform"]["position"] = { pos.x, pos.y, pos.z };
		j["transform"]["rotation"] = { rot.x, rot.y, rot.z, rot.w };
		j["transform"]["scale"] = { scl.x, scl.y, scl.z };

		// Components
		nlohmann::json components = nlohmann::json::array();

		for (int i = 0; i < EnumReflection<ComponentType>::Size(); i++)
		{
			auto compType = static_cast<ComponentType>(i);
			if (compType == ComponentType::Component || compType == ComponentType::Render)
				continue;

			const auto* pComp = entity.GetComponent(compType);
			if (pComp != nullptr)
			{
				auto compJson = pComp->Serialize();
				if (!compJson.empty())
					components.push_back(compJson);
			}
		}

		j["components"] = components;
		return j;
	}

	nlohmann::json SceneSerializer::SerializeScene(const SceneSystem& scene)
	{
		nlohmann::json j;
		j["version"] = 1;

		nlohmann::json entities = nlohmann::json::array();
		for (const auto* pEntity : scene.GetAllRawEntities())
			entities.push_back(SerializeEntity(*pEntity));

		j["entities"] = entities;
		return j;
	}

	static void DeserializeCamera(Entity* pEntity, const nlohmann::json& compJson)
	{
		float fov = compJson.value("fovHorizontal", 90.0f);
		float aspect = compJson.value("aspect", 16.0f / 9.0f);
		float nearPlane = compJson.value("near", 0.1f);
		float farPlane = compJson.value("far", 1000.0f);
		bool isPerspective = compJson.value("isPerspective", true);

		auto* pCamera = pEntity->AddComponent<CompCamera>(1.0f, 1.0f, nearPlane, farPlane);
		pCamera->SetPerspective(isPerspective);
		if (isPerspective)
			pCamera->Set(fov, aspect, nearPlane, farPlane);
	}

	static void DeserializeLight(Entity* pEntity, const nlohmann::json& compJson)
	{
		auto* pLight = pEntity->AddComponent<CompLight>();

		if (compJson.contains("lightType"))
		{
			LightType lt;
			if (EnumReflection<LightType>::TryFromString(compJson["lightType"].get<std::string>(), &lt))
				pLight->SetLightType(lt);
		}

		if (compJson.contains("color"))
		{
			const auto& c = compJson["color"];
			pLight->SetColor({ c[0].get<float>(), c[1].get<float>(), c[2].get<float>() });
		}

		if (compJson.contains("intensity"))
			pLight->SetIntensity(compJson["intensity"].get<float>());

		if (compJson.contains("direction"))
		{
			const auto& d = compJson["direction"];
			pLight->SetDirection({ d[0].get<float>(), d[1].get<float>(), d[2].get<float>() });
		}

		if (compJson.contains("attenuation"))
		{
			const auto& a = compJson["attenuation"];
			pLight->SetAttenuation({ a[0].get<float>(), a[1].get<float>(), a[2].get<float>() });
		}

		if (compJson.contains("innerCutoff"))
			pLight->SetInnerCutoff(compJson["innerCutoff"].get<float>());

		if (compJson.contains("outerCutoff"))
			pLight->SetOuterCutoff(compJson["outerCutoff"].get<float>());
	}

	static void DeserializeStaticMeshRender(Entity* pEntity, AssetsSystem& assets, const nlohmann::json& compJson)
	{
		std::string modelPath = compJson.value("modelPath", "");
		std::string materialPath = compJson.value("materialPath", "");

		if (modelPath.empty() || materialPath.empty())
		{
			Logger::LogError("SceneSerializer: StaticMeshRender missing modelPath or materialPath");
			return;
		}

		auto modelRef = assets.LoadModel(modelPath);
		auto materialRef = assets.LoadMaterial(materialPath);

		if (modelRef && materialRef)
			pEntity->AddComponent<CompStaticMeshRender>(modelRef, materialRef);
	}

	void SceneSerializer::DeserializeScene(SceneSystem& scene, AssetsSystem& assets, const nlohmann::json& json)
	{
		if (!json.contains("entities") || !json["entities"].is_array())
		{
			Logger::LogError("SceneSerializer: invalid scene JSON");
			return;
		}

		// First pass: create all entities
		std::unordered_map<uint32_t, std::weak_ptr<Entity>> guidToEntity;
		std::unordered_map<uint32_t, uint32_t> childToParentGuid; // child guid -> parent guid

		for (const auto& entityJson : json["entities"])
		{
			auto wpEntity = scene.CreateEntity();
			auto spEntity = wpEntity.lock();
			if (!spEntity)
				continue;

			uint32_t originalGuid = entityJson.value("guid", spEntity->GetGuid());
			guidToEntity[originalGuid] = wpEntity;

			if (entityJson.contains("name"))
				spEntity->SetName(entityJson["name"].get<std::string>());

			// Transform
			if (entityJson.contains("transform"))
			{
				const auto& t = entityJson["transform"];
				if (t.contains("position"))
				{
					const auto& p = t["position"];
					spEntity->SetPosition({ p[0].get<float>(), p[1].get<float>(), p[2].get<float>() });
				}
				if (t.contains("rotation"))
				{
					const auto& r = t["rotation"];
					spEntity->SetRotation(Quaternionf{ r[0].get<float>(), r[1].get<float>(), r[2].get<float>(), r[3].get<float>() });
				}
				if (t.contains("scale"))
				{
					const auto& s = t["scale"];
					spEntity->SetScale({ s[0].get<float>(), s[1].get<float>(), s[2].get<float>() });
				}
			}

			// Parent reference (deferred to second pass)
			if (entityJson.contains("parent") && !entityJson["parent"].is_null())
				childToParentGuid[originalGuid] = entityJson["parent"].get<uint32_t>();

			// Components
			if (entityJson.contains("components") && entityJson["components"].is_array())
			{
				for (const auto& compJson : entityJson["components"])
				{
					if (!compJson.contains("type"))
						continue;

					const std::string& compType = compJson["type"].get<std::string>();

					if (compType == "Camera")
						DeserializeCamera(spEntity.get(), compJson);
					else if (compType == "Light")
						DeserializeLight(spEntity.get(), compJson);
					else if (compType == "StaticMeshRender")
						DeserializeStaticMeshRender(spEntity.get(), assets, compJson);
				}
			}
		}

		// Second pass: restore parent-child relationships
		for (const auto& [childGuid, parentGuid] : childToParentGuid)
		{
			auto childIt = guidToEntity.find(childGuid);
			auto parentIt = guidToEntity.find(parentGuid);

			if (childIt != guidToEntity.end() && parentIt != guidToEntity.end())
			{
				auto spChild = childIt->second.lock();
				auto spParent = parentIt->second.lock();
				if (spChild && spParent)
					spChild->SetParent(spParent.get());
			}
		}
	}

	void SceneSerializer::SaveToFile(const SceneSystem& scene, const std::string& filePath)
	{
		nlohmann::json j = SerializeScene(scene);
		std::ofstream ofs(filePath);
		if (!ofs.is_open())
		{
			Logger::LogError("SceneSerializer: failed to open file for writing: {}", filePath);
			return;
		}
		ofs << j.dump(2);
	}

	void SceneSerializer::LoadFromFile(SceneSystem& scene, AssetsSystem& assets, const std::string& filePath)
	{
		std::ifstream ifs(filePath);
		if (!ifs.is_open())
		{
			Logger::LogError("SceneSerializer: failed to open file for reading: {}", filePath);
			return;
		}

		nlohmann::json j;
		ifs >> j;
		DeserializeScene(scene, assets, j);
	}

} // namespace Ailurus
