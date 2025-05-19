#pragma once

#include <memory>
#include <unordered_map>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/EnumReflection.h"

namespace Ailurus
{
	REFLECTION_ENUM(ComponentType,
		Component,
		Render,
		MeshRender,
		Light,
		Camera)

	struct ComponentTypeRegistry
	{
		using TypeMap = std::unordered_map<ComponentType, ComponentType>;

		static TypeMap& GetTypeHierarchyMap()
		{
			static TypeMap typeMap;
			return typeMap;
		}

		template <ComponentType Type, ComponentType BaseType>
		struct Register
		{
			Register()
			{
				GetTypeHierarchyMap()[Type] = BaseType;
			}
		};
	};

	class Component
	{
	public:
		virtual ~Component() = default;

		static constexpr ComponentType StaticType = ComponentType::Component;

		virtual ComponentType GetType() const
		{
			return ComponentType::Component;
		}

		template <typename T>
		bool Is() const
		{
			return Is(T::StaticType());
		}

		bool Is(ComponentType target) const
		{
			ComponentType current = GetType();
			auto& typeMap = ComponentTypeRegistry::GetTypeHierarchyMap();

			while (true)
			{
				if (current == target)
					return true;
				auto it = typeMap.find(current);
				if (it == typeMap.end() || it->second == current)
					break;
				current = it->second;
			}
			return false;
		}
	};

	template <typename Self, ComponentType Type, typename BaseComponent>
	class TComponent : public BaseComponent
	{
	public:
		static const ComponentType StaticType = Type;

		ComponentType GetType() const override
		{
			return StaticType;
		}

	protected:
		struct Registrar
		{
			Registrar()
			{
				ComponentTypeRegistry::Register<Type, BaseComponent::StaticType>();
			}
		};

		static inline Registrar _registrar{};
	};
} // namespace Ailurus