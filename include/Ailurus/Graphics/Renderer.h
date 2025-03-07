#pragma once

#include <memory>
#include <functional>
#include <vulkan/vulkan.hpp>
#include "Airport/Airport.h"
#include "RenderPass/RenderPassForward.h"
#include "Shader/ShaderLibrary.h"
#include "SwapChain/SwapChain.h"
#include "Ailurus/Math/Vector.hpp"
#include "RenderObject/RenderObject.h"

namespace Ailurus
{
    class Renderer
    {
        using GetWindowSizeCallback = std::function<Vector2i()>;

    public:
        explicit Renderer(const GetWindowSizeCallback& getWindowSize);
        ~Renderer();

        void Render();

        void NeedRecreateSwapChain();

    public: // Getter
        vk::SurfaceKHR GetSurface() const;
        vk::CommandPool GetCommandPool() const;
        ShaderLibrary* GetShaderLibrary();
        SwapChain* GetSwapChain() const;

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

        // Static context
        vk::SurfaceKHR _vkSurface = nullptr;
        vk::CommandPool _vkGraphicCommandPool = nullptr;
        std::unique_ptr<ShaderLibrary> _pShaderLibrary = nullptr;

        // temp
        std::unique_ptr<RenderObject> _pRenderObj = nullptr;

        // Dynamic context
        std::unique_ptr<SwapChain> _pSwapChain = nullptr;
        std::unique_ptr<RenderPassForward> _pForwardPass = nullptr;
        std::unique_ptr<Airport> _pAirport = nullptr;
    };
}
