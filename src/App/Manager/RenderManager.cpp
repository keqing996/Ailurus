#include "Ailurus/Application/Manager/RenderManager.h"
#include "Ailurus/Application/Application.h"
#include "Ailurus/Application/Material/Material.h"

namespace Ailurus
{
    Material* RenderManager::GetMaterial(const std::string& name) const
    {
        if (const auto itr = _materialMap.find(name); itr != _materialMap.end())
            return itr->second.get();

        return nullptr;
    }

    Material* RenderManager::AddMaterial(const std::string& name)
    {
        _materialMap[name] = std::make_unique<Material>();
        return GetMaterial(name);
    }
}
