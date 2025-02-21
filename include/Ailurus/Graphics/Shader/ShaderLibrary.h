#pragma once

#include <memory>
#include <unordered_map>
#include "Ailurus/Graphics/Renderer.h"
#include "Shader.h"

namespace Ailurus
{
    class ShaderLibrary
    {
    public:
        explicit ShaderLibrary(const Renderer* pRenderer);

    public:
        Shader* GetShader(ShaderStage stage, const std::string& path);
        void Clear();

    private:
        Shader* LoadShader(ShaderStage stage, const std::string& path);

    private:
        const Renderer* _pRenderer;
        std::unordered_map<ShaderStage, std::unordered_map<std::string, std::unique_ptr<Shader>>> _library;
    };
}
