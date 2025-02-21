#include "Ailurus/Graphics/Shader/ShaderLibrary.h"

namespace Ailurus
{
    ShaderLibrary::ShaderLibrary(const Renderer* pRenderer)
        : _pRenderer(pRenderer)
    {
    }

    Shader* ShaderLibrary::GetShader(ShaderStage stage, const std::string& path)
    {
        auto& map = _library[stage];
        if (const auto itr = map.find(path); itr == map.end())
            return LoadShader(stage, path);
        else
            return itr->second.get();
    }

    void ShaderLibrary::Clear()
    {
        _library.clear();
    }

    Shader* ShaderLibrary::LoadShader(ShaderStage stage, const std::string& path)
    {
        _library[stage][path] = std::make_unique<Shader>(_pRenderer, stage, path);
        return _library[stage][path].get();
    }
}
