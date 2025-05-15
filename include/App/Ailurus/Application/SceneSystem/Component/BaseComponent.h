#pragma once

#include <memory>
#include <unordered_map>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/EnumReflection.h"

namespace Ailurus
{
	REFLECTION_ENUM(ComponentType,
		Transform,
		Render,
		MeshRender,
		Light)

	class Component : public NonCopyable
	{
		static const std::unordered_map<ComponentType, ComponentType> _hierarchy;

	public:
		virtual ~Component() = default;
		virtual ComponentType GetType() const = 0;

	public:
		static bool IsDerivedFrom(ComponentType derived, ComponentType base);
		static std::unique_ptr<Component> CreateComponent(ComponentType type);
	};
} // namespace Ailurus