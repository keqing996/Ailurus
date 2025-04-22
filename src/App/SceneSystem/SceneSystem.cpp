#include "Ailurus/Application/SceneSystem/SceneSystem.h"

namespace Ailurus
{

	SceneSystem::~SceneSystem()
	{
	}

	std::weak_ptr<Entity> SceneSystem::CreateEntity()
	{
		_entityIdCounter++;
		auto pEntity = std::make_shared<Entity>(_entityIdCounter);
		_entityMap[_entityIdCounter] = pEntity;
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
		if (_entityMap.contains(guid))
		{
			_entityMap.erase(guid);
			return true;
		}

		return false;
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
} // namespace Ailurus