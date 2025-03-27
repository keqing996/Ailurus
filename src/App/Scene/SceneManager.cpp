#include "Ailurus/Application/Scene/SceneManager.h"

namespace Ailurus
{

	SceneManager::~SceneManager()
	{
	}

	std::weak_ptr<Entity> SceneManager::CreateEntity()
	{
		_entityIdCounter++;
		auto pEntity = std::make_shared<Entity>(_entityIdCounter);
		_entityMap[_entityIdCounter] = pEntity;
		return pEntity;
	}

	std::weak_ptr<Entity> SceneManager::GetEntity(uint32_t guid) const
	{
		if (const auto itr = _entityMap.find(guid); itr != _entityMap.end())
			return itr->second;

		return {};
	}

	bool SceneManager::DestroyEntity(uint32_t guid)
	{
		if (_entityMap.contains(guid))
		{
			_entityMap.erase(guid);
			return true;
		}

		return false;
	}

	bool SceneManager::DestroyEntity(const std::weak_ptr<Entity>& pEntity)
	{
		if (const auto spEntity = pEntity.lock())
			return DestroyEntity(spEntity->GetGuid());

		return false;
	}

	std::vector<Entity*> SceneManager::GetAllRawEntities() const
	{
		std::vector<Entity*> result;
		result.reserve(_entityMap.size());
		for (const auto kvp : _entityMap)
			result.push_back(kvp.second.get());
	}

	SceneManager::SceneManager()
	{
	}
} // namespace Ailurus