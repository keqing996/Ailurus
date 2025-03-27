#include "Ailurus/Application/Component/BaseComponent.h"

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
} // namespace Ailurus