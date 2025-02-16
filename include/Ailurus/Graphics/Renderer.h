#pragma once

#include <memory>
#include <functional>
#include <vulkan/vulkan.hpp>
#include "Airport/Airport.h"
#include "FrameBuffer/BackBuffer.h"
#include "Pipeline/Pipeline.h"
#include "RenderPass/RenderPass.h"
#include "Shader/Shader.h"
#include "SwapChain/SwapChain.h"
#include "../Math/Vector.hpp"

namespace Ailurus
{
    class Renderer
    {
        using GetWindowSizeCallback = std::function<Vector2i()>;
        using GetWindowInstanceExtension = std::function<std::vector<const char*>()>;
        using WindowCreateSurfaceCallback = std::function<vk::SurfaceKHR(const vk::Instance&)>;
        using WindowDestroySurfaceCallback = std::function<void(const vk::Instance&, const vk::SurfaceKHR&)>;

    public:
        struct QueueIndex
        {
            std::optional<uint32_t> graphicQueueIndex = std::nullopt;
            std::optional<uint32_t> presentQueueIndex = std::nullopt;
        };

    public:
        Renderer(const GetWindowSizeCallback& getWindowSize,
                 const GetWindowInstanceExtension& getWindowInstExt,
                 const WindowCreateSurfaceCallback& createSurface,
                 const WindowDestroySurfaceCallback& destroySurface,
                 bool enableValidationLayer);

        ~Renderer();

        void Render();

        void RecordCommand(vk::CommandBuffer commandBuffer, vk::RenderPass renderPass, vk::Framebuffer targetFrameBuffer);

        void NeedRecreateSwapChain();

    public: // Getter
        vk::Instance GetInstance() const;
        vk::SurfaceKHR GetSurface() const;
        vk::PhysicalDevice GetPhysicalDevice() const;
        vk::Device GetLogicalDevice() const;
        const QueueIndex& GetQueueIndex() const;
        vk::Queue GetGraphicQueue() const;
        vk::Queue GetPresentQueue() const;
        vk::CommandPool GetCommandPool() const;

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
        QueueIndex _queueIndex;
        vk::Queue _vkGraphicQueue = nullptr;
        vk::Queue _vkPresentQueue = nullptr;
        vk::CommandPool _vkGraphicCommandPool = nullptr;

        // Dynamic context
        std::unique_ptr<SwapChain> _pSwapChain = nullptr;
        std::unique_ptr<RenderPass> _pRenderPass = nullptr;
        std::unique_ptr<Pipeline> _pPipeline = nullptr;
        std::unique_ptr<BackBuffer> _pBackBuffer = nullptr;
        std::unique_ptr<Airport> _pAirport = nullptr;

        // temp
        std::unique_ptr<Shader> _pVertShader = nullptr;
        std::unique_ptr<Shader> _pFragShader = nullptr;
    };
}
