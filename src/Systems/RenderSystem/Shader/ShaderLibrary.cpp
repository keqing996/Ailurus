#include "Ailurus/Systems/RenderSystem/Shader/ShaderLibrary.h"
#include "Ailurus/OS/Path.h"

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

    ShaderLibrary::~ShaderLibrary()
    {
    }

    void ShaderLibrary::Clear()
    {
        _library.clear();
    }

    Shader* ShaderLibrary::LoadShader(ShaderStage stage, const std::string& path)
    {
        auto resolvedPath = Path::ResolvePath(path);
        _library[stage][path] = std::make_unique<Shader>(stage, resolvedPath);
        return _library[stage][path].get();
    }
}
