#include "Ailurus/Systems/SceneSystem/SceneSystem.h"
#include "Ailurus/Systems/SceneSystem/SceneSerializer.h"
#include "Ailurus/Application.h"
#include "Ailurus/Systems/AssetsSystem/AssetsSystem.h"

namespace Ailurus
{

	SceneSystem::~SceneSystem()
	{
	}

	void SceneSystem::AddObserver(void* key, SceneObserver* observer)
	{
		_observers[key] = observer;
	}

	void SceneSystem::RemoveObserver(void* key)
	{
		_observers.erase(key);
	}

	std::weak_ptr<Entity> SceneSystem::CreateEntity()
	{
		_entityIdCounter++;
		auto pEntity = std::make_shared<Entity>(_entityIdCounter);
		pEntity->_sceneSystem = this;
		_entityMap[_entityIdCounter] = pEntity;
		NotifyEntityCreated(*pEntity);
		return pEntity;
	}

	std::weak_ptr<Entity> SceneSystem::GetEntity(uint32_t guid) const
	{
		if (const auto itr = _entityMap.find(guid); itr != _entityMap.end())
			return itr->second;

		return {};
	}

	bool SceneSystem::DestroyEntity(uint32_t guid)
	{
		auto it = _entityMap.find(guid);
		if (it == _entityMap.end())
			return false;

		Entity* pEntity = it->second.get();

		// Detach from parent
		if (pEntity->GetParent() != nullptr)
			pEntity->SetParent(nullptr);

		// Recursively destroy children (collect guids first to avoid iterator invalidation)
		std::vector<uint32_t> childGuids;
		for (Entity* child : pEntity->GetChildren())
			childGuids.push_back(child->GetGuid());

		for (uint32_t childGuid : childGuids)
			DestroyEntity(childGuid);

		NotifyEntityDestroyed(*pEntity);
		pEntity->_sceneSystem = nullptr;

		_entityMap.erase(it);
		return true;
	}

	bool SceneSystem::DestroyEntity(const std::weak_ptr<Entity>& pEntity)
	{
		if (const auto spEntity = pEntity.lock())
			return DestroyEntity(spEntity->GetGuid());

		return false;
	}

	std::vector<Entity*> SceneSystem::GetAllRawEntities() const
	{
		std::vector<Entity*> result;
		result.reserve(_entityMap.size());
		for (const auto kvp : _entityMap)
			result.push_back(kvp.second.get());

		return result;
	}

	SceneSystem::SceneSystem()
	{
	}

	void SceneSystem::NotifyEntityCreated(const Entity& entity)
	{
		for (const auto& [key, observer] : _observers)
		{
			(void)key;
			if (observer != nullptr)
				observer->OnEntityCreated(entity);
		}
	}

	void SceneSystem::NotifyEntityDestroyed(const Entity& entity)
	{
		for (const auto& [key, observer] : _observers)
		{
			(void)key;
			if (observer != nullptr)
				observer->OnEntityDestroyed(entity);
		}
	}

	void SceneSystem::NotifyEntityNameChanged(const Entity& entity)
	{
		for (const auto& [key, observer] : _observers)
		{
			(void)key;
			if (observer != nullptr)
				observer->OnEntityNameChanged(entity);
		}
	}

	void SceneSystem::NotifyEntityParentChanged(const Entity& entity)
	{
		for (const auto& [key, observer] : _observers)
		{
			(void)key;
			if (observer != nullptr)
				observer->OnEntityParentChanged(entity);
		}
	}

	void SceneSystem::NotifyEntityTransformChanged(const Entity& entity)
	{
		for (const auto& [key, observer] : _observers)
		{
			(void)key;
			if (observer != nullptr)
				observer->OnEntityTransformChanged(entity);
		}
	}

	void SceneSystem::UpdateAllComponents(float deltaTime)
	{
		for (const auto& [guid, pEntity] : _entityMap)
		{
			for (const auto& [compType, compVec] : pEntity->_components)
			{
				for (const auto& pComp : compVec)
					pComp->OnUpdate(deltaTime);
			}
		}
	}

	void SceneSystem::SaveToFile(const std::string& filePath) const
	{
		SceneSerializer::SaveToFile(*this, filePath);
	}

	void SceneSystem::LoadFromFile(const std::string& filePath)
	{
		auto* pAssets = Application::Get<AssetsSystem>();
		SceneSerializer::LoadFromFile(*this, *pAssets, filePath);
	}
} // namespace Ailurus