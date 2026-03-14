#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"
#include "Entity/Entity.h"

namespace Ailurus
{
	class SceneObserver
	{
	public:
		virtual ~SceneObserver() = default;

		virtual void OnEntityCreated(const Entity& entity) {}
		virtual void OnEntityDestroyed(const Entity& entity) {}
		virtual void OnEntityNameChanged(const Entity& entity) {}
		virtual void OnEntityParentChanged(const Entity& entity) {}
		virtual void OnEntityTransformChanged(const Entity& entity) {}
	};

    class SceneSystem : public NonCopyable, public NonMovable
	{
	public:
		~SceneSystem();

	public:
		void AddObserver(void* key, SceneObserver* observer);
		void RemoveObserver(void* key);
		std::weak_ptr<Entity> CreateEntity();
		std::weak_ptr<Entity> GetEntity(uint32_t guid) const;
		bool DestroyEntity(uint32_t guid);
		bool DestroyEntity(const std::weak_ptr<Entity>& pEntity);
		std::vector<Entity*> GetAllRawEntities() const;
		void UpdateAllComponents(float deltaTime);
		void SaveToFile(const std::string& filePath) const;
		void LoadFromFile(const std::string& filePath);

	private:
		friend class Application;
		friend class Entity;
		SceneSystem();
		void NotifyEntityCreated(const Entity& entity);
		void NotifyEntityDestroyed(const Entity& entity);
		void NotifyEntityNameChanged(const Entity& entity);
		void NotifyEntityParentChanged(const Entity& entity);
		void NotifyEntityTransformChanged(const Entity& entity);

	private:
		uint32_t _entityIdCounter = 0;
		std::unordered_map<uint32_t, std::shared_ptr<Entity>> _entityMap;
		std::unordered_map<void*, SceneObserver*> _observers;
	};

} // namespace Ailurus