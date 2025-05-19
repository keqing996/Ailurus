#pragma once

#include "ComponentMeta.h"

namespace Ailurus
{
	class Component
	{
	public:
		virtual ~Component() = default;

		virtual ComponentType GetType() const
		{
			return ComponentType::Component;
		}

		static constexpr ComponentType StaticType = ComponentType::Component;
	};

	template <ComponentType Type, typename BaseComponent, bool AllowMultipleInstance = false>
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
				ComponentMeta::Register<Type, BaseComponent::StaticType, AllowMultipleInstance>();
			}
		};

		static inline Registrar _typeRegistrar{};
	};
}