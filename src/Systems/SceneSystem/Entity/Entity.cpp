#include "Ailurus/Systems/SceneSystem/Entity/Entity.h"
#include "Ailurus/Utility/Logger.h"
#include "Ailurus/Math/Math.hpp"
#include <algorithm>

namespace Ailurus
{
    uint32_t Entity::GetGuid() const
	{
		return _guid;
	}

	const std::string& Entity::GetName() const
	{
		return _name;
	}

	void Entity::SetName(const std::string& name)
	{
		_name = name;
	}

	Entity* Entity::GetParent() const
	{
		return _parent;
	}

	static bool IsDescendantOf(const Entity* node, const Entity* potentialAncestor)
	{
		const Entity* current = node;
		while (current != nullptr)
		{
			if (current == potentialAncestor)
				return true;
			current = current->GetParent();
		}
		return false;
	}

	void Entity::SetParent(Entity* parent)
	{
		if (parent == this)
			return;

		// Prevent circular reference
		if (parent != nullptr && IsDescendantOf(parent, this))
		{
			Logger::LogError("Entity::SetParent: circular reference detected");
			return;
		}

		// Remove from old parent
		if (_parent != nullptr)
		{
			auto& siblings = _parent->_children;
			siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
		}

		_parent = parent;

		// Add to new parent
		if (_parent != nullptr)
			_parent->_children.push_back(this);
	}

	const std::vector<Entity*>& Entity::GetChildren() const
	{
		return _children;
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
		// O(1) exact match
		auto it = _components.find(type);
		if (it != _components.end() && !it->second.empty())
			return it->second.front().get();

		// Fallback: inheritance-based lookup (e.g. GetComponent<CompRender>() finds CompStaticMeshRender)
		for (const auto& [compType, vec] : _components)
		{
			if (!vec.empty() && ComponentMeta::Is(compType, type))
				return vec.front().get();
		}

		return nullptr;
	}

	auto Entity::RemoveComponent(ComponentType compType) -> bool
	{
		bool hasRemoved = false;

		// Exact match
		if (auto it = _components.find(compType); it != _components.end())
		{
			if (!it->second.empty())
			{
				for (auto& pComp : it->second)
					pComp->OnDetach();
				hasRemoved = true;
			}
			_components.erase(it);
		}

		// Inheritance-based removal (e.g. RemoveComponent<CompRender>() removes CompStaticMeshRender)
		for (auto it = _components.begin(); it != _components.end(); )
		{
			if (ComponentMeta::Is(it->first, compType))
			{
				if (!it->second.empty())
				{
					for (auto& pComp : it->second)
						pComp->OnDetach();
					hasRemoved = true;
				}
				it = _components.erase(it);
			}
			else
			{
				++it;
			}
		}

		return hasRemoved;
	}

	Matrix4x4f Entity::GetModelMatrix() const
	{
		Matrix4x4f localMatrix = Math::TranslateMatrix(GetPosition())
			* Math::QuaternionToRotateMatrix(GetRotation())
			* Math::ScaleMatrix(GetScale());

		if (_parent != nullptr)
			return _parent->GetModelMatrix() * localMatrix;

		return localMatrix;
	}

	Entity::Entity(uint32_t guid)
		: _guid(guid)
	{
	}
} // namespace Ailurus
