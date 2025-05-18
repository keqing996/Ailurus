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
			if (pComp->Is(type))
				return pComp.get();
		}

		return nullptr;
	}

	auto Entity::RemoveComponent(ComponentType compType) -> bool
	{
    	bool hasRemoved = false;

    	size_t componentNum = _components.size();
    	for (auto i = 0; i < _components.size(); ++i)
    	{
    		if (_components[i]->Is(compType))
    		{
    			hasRemoved = true;
    			_components[i] = nullptr;
    			componentNum--;
    		}
    	}

    	if (hasRemoved)
    	{
    		std::vector<std::unique_ptr<Component>> temp;
    		temp.reserve(componentNum);
    		for (auto i = 0; i < _components.size(); ++i)
    		{
    			if (_components[i] != nullptr)
    				temp.push_back(std::move(_components[i]));
    		}

    		std::swap(_components, temp);
    	}

    	return hasRemoved;
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
