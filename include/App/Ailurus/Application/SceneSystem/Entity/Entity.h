#pragma once

#include <vector>
#include <cstdint>
#include <memory>
#include "Ailurus/Math/Vector3.hpp"
#include "Ailurus/Math/Quaternion.hpp"
#include "Ailurus/Math/Matrix4x4.hpp"
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
		/// Get the unique identifier of the entity
		auto GetGuid() const -> uint32_t;

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

		/// Get a component of a specified type attached to the entity
		auto GetComponent(ComponentType type) const -> Component*;

		/// Add a new component of a specified type to the entity
		auto AddComponent(ComponentType type) -> Component*;

		/// Add a new component of a specified type and cast it to type T
		template <typename T>
		auto AddComponent(ComponentType type) -> T*;

		/// Get a component of a specified type and cast it to type T
		template <typename T>
		auto GetComponent(ComponentType type) const -> T*;

		/// Get the model matrix of the entity
		auto GetModelMatrix() const -> Matrix4x4f;

	private:
		// Global uid
		uint32_t _guid;

		// Transform
		Vector3f _position = Vector3f::Zero;
		Quaternionf _rotation = Quaternionf::Identity;
		Vector3f _scale = Vector3f::One;

		// Components
		std::vector<std::unique_ptr<Component>> _components;
	};

	template <typename T>
	T* Entity::AddComponent(ComponentType type)
	{
		return reinterpret_cast<T*>(AddComponent(type));
	}

	template <typename T>
	T* Entity::GetComponent(ComponentType type) const
	{
		return reinterpret_cast<T*>(GetComponent(type));
	}
} // namespace Ailurus