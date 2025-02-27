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
        using GetWindowInstanceExtension = std::function<std::vector<const char*>()>;
        using WindowCreateSurfaceCallback = std::function<vk::SurfaceKHR(const vk::Instance&)>;
        using WindowDestroySurfaceCallback = std::function<void(const vk::Instance&, const vk::SurfaceKHR&)>;

    public:
        Renderer(const GetWindowSizeCallback& getWindowSize,
                 const GetWindowInstanceExtension& getWindowInstExt,
                 const WindowCreateSurfaceCallback& createSurface,
                 const WindowDestroySurfaceCallback& destroySurface,
                 bool enableValidationLayer);

        ~Renderer();

        void Render();

        void NeedRecreateSwapChain();

    public: // Getter
        vk::Instance GetInstance() const;
        vk::SurfaceKHR GetSurface() const;
        vk::PhysicalDevice GetPhysicalDevice() const;
        vk::Device GetLogicalDevice() const;
        uint32_t GetGraphicQueueIndex() const;
        uint32_t GetPresentQueueIndex() const;
        vk::Queue GetGraphicQueue() const;
        vk::Queue GetPresentQueue() const;
        vk::CommandPool GetCommandPool() const;
        ShaderLibrary* GetShaderLibrary();
        SwapChain* GetSwapChain() const;

    private:
        // Static Context
        void CreateStaticContext(bool enableValidation, const WindowCreateSurfaceCallback& createSurface);
        void DestroyStaticContext();
        void CreateInstance(bool enableValidation);
        void CreatDebugReportCallbackExt();
        void CreateSurface(const WindowCreateSurfaceCallback& createSurface);
        void ChoosePhysicsDevice();
        void CreateLogicDevice();
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
        GetWindowInstanceExtension _getWindowInstExtensionsCallback;
        WindowDestroySurfaceCallback _windowDestroySurfaceCallback;

        // Static context
        vk::Instance _vkInstance = nullptr;
        vk::DebugReportCallbackEXT _vkDebugReportCallbackExt = nullptr;
        vk::SurfaceKHR _vkSurface = nullptr;
        vk::PhysicalDevice _vkPhysicalDevice = nullptr;
        vk::Device _vkLogicalDevice = nullptr;
        uint32_t _graphicQueueIndex = 0;
        uint32_t _presentQueueIndex = 0;
        vk::Queue _vkGraphicQueue = nullptr;
        vk::Queue _vkPresentQueue = nullptr;
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
