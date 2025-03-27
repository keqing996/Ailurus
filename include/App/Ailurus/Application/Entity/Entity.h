#pragma once

#include <vector>
#include <cstdint>
#include <memory>
#include "Ailurus/Application/Component/BaseComponent.h"

namespace Ailurus
{
	class Entity
	{
	public:
		Entity(const Entity& rhs) = delete;
		Entity& operator=(const Entity& rhs) = delete;

	public:
		uint32_t GetGuid() const;
		Component* GetComponent(ComponentType type) const;

		template <typename T>
		T* GetComponent(ComponentType type) const
		{
			return reinterpret_cast<T*>(GetComponent(type));
		}

	private:
		friend class SceneManager;
		explicit Entity(uint32_t guid);

	private:
		uint32_t _guid;
		std::vector<std::unique_ptr<Component>> _components;
	};
} // namespace Ailurus