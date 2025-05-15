#include "Ailurus/Application/SceneSystem/Component/BaseComponent.h"
#include "Ailurus/Utility/Logger.h"
#include "Ailurus/Application/SceneSystem/Component/CompMeshRender.h"

namespace Ailurus
{
	const std::unordered_map<ComponentType, ComponentType> Component::_hierarchy = {
		{ ComponentType::MeshRender, ComponentType::Render },
	};

	bool Component::IsDerivedFrom(ComponentType derived, ComponentType base)
	{
		if (const auto it = _hierarchy.find(derived); it != _hierarchy.end())
			return it->second == base || IsDerivedFrom(it->second, base);

		return false;
	}

	std::unique_ptr<Component> Component::CreateComponent(ComponentType type)
	{
		switch (type)
		{
			case ComponentType::MeshRender:
				return std::make_unique<CompMeshRender>();
			default:
				Logger::LogError("Component type {} can not be created.",
					EnumReflection<ComponentType>::ToString(type));
				return nullptr;
		}
	}
} // namespace Ailurus