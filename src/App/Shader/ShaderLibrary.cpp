#include "ShaderLibrary.h"

namespace Ailurus
{
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
        _library[stage][path] = std::make_unique<Shader>(stage, path);
        return _library[stage][path].get();
    }
}
