#pragma once

#include <functional>
#include <memory>
#include <vulkan/vulkan.hpp>
#include "Ailurus/Math/Vector.hpp"
#include "Shader/ShaderLibrary.h"

namespace Ailurus
{
    class Render
    {
        using GetWindowSizeCallback = std::function<Vector2i()>;

    public:
        explicit Render(const GetWindowSizeCallback& getWindowSize);
        ~Render();

        void RenderScene();

        void NeedRecreateSwapChain();

    public: // Getter
        vk::SurfaceKHR GetSurface() const;
        vk::CommandPool GetCommandPool() const;
        ShaderLibrary* GetShaderLibrary();
        class SwapChain* GetSwapChain() const;

    private:
        // Static Context
        void CreateStaticContext();
        void DestroyStaticContext();
        void CreateCommandPool();

        // Dynamic Context
        void CreateDynamicContext();
        void DestroyDynamicContext();

        // Rebuild
        void RecreateSwapChain();

    private:
        bool _needRebuildSwapChain;
        uint32_t _currentFlight = 0;

        // Callback
        GetWindowSizeCallback _getWindowSizeCallback;



        std::unique_ptr<ShaderLibrary> _pShaderLibrary = nullptr;

        // temp
        std::unique_ptr<class RenderObject> _pRenderObj = nullptr;

        // Dynamic context
        std::unique_ptr<SwapChain> _pSwapChain = nullptr;
        std::unique_ptr<class RenderPassForward> _pForwardPass = nullptr;
        std::unique_ptr<class Airport> _pAirport = nullptr;
    };
}
