#pragma once

#include <memory>
#include <unordered_map>
#include "Shader.h"

namespace Ailurus
{
    class Renderer;

    class ShaderLibrary
    {
    public:
        explicit ShaderLibrary(const Renderer* pRenderer);

    public:
        void Clear();
        Shader* GetShader(ShaderStage stage, const std::string& path);

        template<ShaderStage stage>
        Shader* GetShader(const std::string& path)
        {
            return GetShader(stage, path);
        }

    private:
        Shader* LoadShader(ShaderStage stage, const std::string& path);

    private:
        const Renderer* _pRenderer;
        std::unordered_map<ShaderStage, std::unordered_map<std::string, std::unique_ptr<Shader>>> _library;
    };
}
