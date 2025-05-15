#pragma once

#include <vector>
#include <cstdint>
#include <memory>
#include "Ailurus/Application/SceneSystem/Component/BaseComponent.h"

namespace Ailurus
{
	class Entity
	{
	public:
		explicit Entity(uint32_t guid);
		Entity(const Entity& rhs) = delete;
		Entity& operator=(const Entity& rhs) = delete;

	public:
		uint32_t GetGuid() const;
		Component* GetComponent(ComponentType type) const;
		Component* AddComponent(ComponentType type);

		template <typename T>
		T* AddComponent(ComponentType type)
		{
			return reinterpret_cast<T*>(AddComponent(type));
		}

		template <typename T>
		T* GetComponent(ComponentType type) const
		{
			return reinterpret_cast<T*>(GetComponent(type));
		}

	private:
		uint32_t _guid;
		std::vector<std::unique_ptr<Component>> _components;
	};
} // namespace Ailurus