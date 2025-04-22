#pragma once

#include <unordered_map>
#include <memory>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"
#include "Entity/Entity.h"

namespace Ailurus
{
    class SceneSystem : public NonCopyable, public NonMovable
	{
	public:
		~SceneSystem();

	public:
		std::weak_ptr<Entity> CreateEntity();
		std::weak_ptr<Entity> GetEntity(uint32_t guid) const;
		bool DestroyEntity(uint32_t guid);
		bool DestroyEntity(const std::weak_ptr<Entity>& pEntity);
		std::vector<Entity*> GetAllRawEntities() const;

	private:
		friend class Application;
		SceneSystem();

	private:
		uint32_t _entityIdCounter = 0;
		std::unordered_map<uint32_t, std::shared_ptr<Entity>> _entityMap;
	};

} // namespace Ailurus