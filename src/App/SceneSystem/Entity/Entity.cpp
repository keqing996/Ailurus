#include "Ailurus/Application/SceneSystem/Entity/Entity.h"
#include "Ailurus/Utility/Logger.h"
#include "Ailurus/Math/Math.hpp"

namespace Ailurus
{
    uint32_t Entity::GetGuid() const
	{
		return _guid;
	}

	Vector3f Entity::GetPosition() const
	{
		return _position;
	}

	void Entity::SetPosition(const Vector3f& position)
	{
		_position = position;
	}

	Quaternionf Entity::GetRotation() const
	{
		return _rotation;
	}

	void Entity::SetRotation(const Quaternionf& rotation)
	{
		_rotation = rotation;
	}

	Vector3f Entity::GetScale() const
	{
		return _scale;
	}

	void Entity::SetScale(const Vector3f& scale)
	{
    	_scale = scale;
	}

	Component* Entity::GetComponent(ComponentType type) const
	{
		for (const auto& pComp : _components)
		{
			const auto compType = pComp->GetType();
			if (compType == type || Component::IsDerivedFrom(compType, type))
				return pComp.get();
		}

		return nullptr;
	}

	Component* Entity::AddComponent(ComponentType type)
	{
		for (const auto& pExistComp : _components)
		{
			if (pExistComp->GetType() == type)
			{
				Logger::LogError("Entity {} already have {}.", _guid,
					EnumReflection<ComponentType>::ToString(type));
				return nullptr;
			}
		}

		std::unique_ptr<Component> pComp = Component::CreateComponent(type);
		if (pComp == nullptr)
			return nullptr;

		_components.push_back(std::move(pComp));
		return _components.back().get();
	}

	Matrix4x4f Entity::GetModelMatrix() const
	{
    	return Math::MakeModelMatrix(GetPosition(), GetRotation(), GetScale());
	}

	Entity::Entity(uint32_t guid)
		: _guid(guid)
	{
	}
} // namespace Ailurus
