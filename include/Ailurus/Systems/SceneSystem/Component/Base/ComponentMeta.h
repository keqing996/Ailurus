#pragma once

#include <unordered_map>
#include "ComponentType.h"
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"

namespace Ailurus
{
	class ComponentMeta : NonCopyable, NonMovable
	{
	public:
		ComponentMeta() = delete;

	private:
		struct MetaData
		{
			ComponentType baseType;
			bool allowMultipleInstance = false;
		};

		using MetaMap = std::unordered_map<ComponentType, MetaData>;

		static MetaMap& GetTypeHierarchyMap()
		{
			static MetaMap typeMap;
			return typeMap;
		}

	public:
		template <typename Derived, typename Base>
		static bool Is()
		{
			return Is(Derived::StaticType, Base::StaticType);
		}

		static bool Is(ComponentType derived, ComponentType base)
		{
			auto& typeMap = GetTypeHierarchyMap();

			while (true)
			{
				if (derived == base)
					return true;
				auto it = typeMap.find(derived);
				if (it == typeMap.end() || it->second.baseType == derived)
					break;
				derived = it->second.baseType;
			}
			return false;
		}

		template <typename T>
		static bool AllowMultipleInstance()
		{
			return AllowMultipleInstance(T::StaticType);
		}

		static bool AllowMultipleInstance(ComponentType type)
		{
			return GetTypeHierarchyMap()[type].allowMultipleInstance;
		}

		template <ComponentType Type, ComponentType BaseType, bool AllowMultipleInstance>
		struct Register
		{
			Register()
			{
				GetTypeHierarchyMap()[Type] = { BaseType, AllowMultipleInstance };
			}
		};
	};
}