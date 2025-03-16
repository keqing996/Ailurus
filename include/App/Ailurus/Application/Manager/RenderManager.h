#pragma once

#include <string>
#include <unordered_map>
#include <memory>

namespace Ailurus
{
    class Material;
    class MeshRender;

    class RenderManager
    {
    public:

    public:
        // Material
        Material* GetMaterial(const std::string& name) const;
        Material* AddMaterial(const std::string& name);

        // Draw
        void DrawRender(const MeshRender* pRender);

    private:
        std::unordered_map<std::string, std::unique_ptr<Material>> _materialMap;
    };
}