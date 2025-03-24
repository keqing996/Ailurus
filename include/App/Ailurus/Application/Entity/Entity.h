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
		Entity(const Entity& rhs) = delete;			   // temp
		Entity& operator=(const Entity& rhs) = delete; // temp

	public:
		uint32_t GetGuid() const;

	private:
		friend class SceneManager;
		explicit Entity(uint32_t guid);

	private:
		uint32_t _guid;
		std::vector<std::unique_ptr<BaseComponent>> _components;
	};
} // namespace Ailurus