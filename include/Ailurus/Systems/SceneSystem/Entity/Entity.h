#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include "Ailurus/Math/Vector3.hpp"
#include "Ailurus/Math/Quaternion.hpp"
#include "Ailurus/Math/Matrix4x4.hpp"
#include "Ailurus/Systems/SceneSystem/Component/Base/Component.h"

namespace Ailurus
{
	class Entity
	{
	public:
		explicit Entity(uint32_t guid);
		Entity(const Entity& rhs) = delete;
		Entity& operator=(const Entity& rhs) = delete;

	public:
		/// Get the unique identifier of the entity
		auto GetGuid() const -> uint32_t;

		/// Get entity name
		auto GetName() const -> const std::string&;

		/// Set entity name
		auto SetName(const std::string& name) -> void;

		/// Get parent entity
		auto GetParent() const -> Entity*;

		/// Set parent entity (nullptr to unparent)
		auto SetParent(Entity* parent) -> void;

		/// Get children entities
		auto GetChildren() const -> const std::vector<Entity*>&;

		/// Get the position of the entity in 3D space
		auto GetPosition() const -> Vector3f;

		/// Set the position of the entity in 3D space
		auto SetPosition(const Vector3f& position) -> void;

		/// Get the rotation of the entity as a quaternion
		auto GetRotation() const -> Quaternionf;

		/// Set the rotation of the entity using a quaternion
		auto SetRotation(const Quaternionf& rotation) -> void;

		/// Get the scale of the entity in 3D space
		auto GetScale() const -> Vector3f;

		/// Set the scale of the entity in 3D space
		auto SetScale(const Vector3f& scale) -> void;

		/// Add a new component of a specified type and cast it to type T
		template <typename T, typename... Types>
		auto AddComponent(Types&&... Args) -> T*;

		/// Add a new component of a specified type and cast it to type T
		template <typename T>
		auto AddComponent() -> T*;

		/// Get a component of a specified type and cast it to type T
		template <typename T>
		auto GetComponent() const -> T*;

		/// Get a component of a specified type
		auto GetComponent(ComponentType type) const -> Component*;

		/// Remove the component of a specified type
		auto RemoveComponent(ComponentType compType) -> bool;

		/// Remove the component of specified typee
		template <typename T>
		auto RemoveComponent() -> bool;

		/// Get the model matrix of the entity
		auto GetModelMatrix() const -> Matrix4x4f;

	private:
		friend class SceneSystem;

		// Global uid
		uint32_t _guid;

		// Name
		std::string _name;

		// Hierarchy
		Entity* _parent = nullptr;
		std::vector<Entity*> _children;

		// Transform
		Vector3f _position = Vector3f::Zero;
		Quaternionf _rotation = Quaternionf::Identity;
		Vector3f _scale = Vector3f::One;

		// Components
		std::unordered_map<ComponentType, std::vector<std::unique_ptr<Component>>> _components;
	};

	template <typename T, typename... Types>
	T* Entity::AddComponent(Types&&... Args)
	{
		if (!ComponentMeta::AllowMultipleInstance<T>())
			RemoveComponent<T>();

		std::unique_ptr<T> pComp = std::make_unique<T>(std::forward<Types>(Args)...);
		pComp->_parentEntity = this;

		auto& vec = _components[T::StaticType];
		vec.push_back(std::move(pComp));
		T* result = static_cast<T*>(vec.back().get());
		result->OnAttach();
		return result;
	}

	template <typename T>
	T* Entity::AddComponent()
	{
		if (!ComponentMeta::AllowMultipleInstance<T>())
			RemoveComponent<T>();

		std::unique_ptr<T> pComp = std::make_unique<T>();
		pComp->_parentEntity = this;

		auto& vec = _components[T::StaticType];
		vec.push_back(std::move(pComp));
		T* result = static_cast<T*>(vec.back().get());
		result->OnAttach();
		return result;
	}

	template <typename T>
	T* Entity::GetComponent() const
	{
		return reinterpret_cast<T*>(GetComponent(T::StaticType));
	}

	template <typename T>
	bool Entity::RemoveComponent()
	{
		return RemoveComponent(T::StaticType);
	}
} // namespace Ailurus