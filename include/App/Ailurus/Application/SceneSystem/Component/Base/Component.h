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

		class Entity* GetEntity() const
		{
			return _parentEntity;
		}

		static constexpr ComponentType StaticType = ComponentType::Component;

	protected:
		friend class Entity;
		Entity* _parentEntity = nullptr;
	};

	template <ComponentType Type, typename BaseType, bool AllowMultipleInstance = false>
	class TComponent : public BaseType
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
				ComponentMeta::Register<Type, BaseType::StaticType, AllowMultipleInstance>();
			}
		};

		static inline Registrar _typeRegistrar{};
	};
}