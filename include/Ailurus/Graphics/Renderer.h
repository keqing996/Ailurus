#pragma once

#include <memory>
#include <functional>
#include <vulkan/vulkan.hpp>
#include "Context/VulkanContext.h"
#include "FrameBuffer/BackBuffer.h"
#include "Pipeline/Pipeline.h"
#include "RenderPass/RenderPass.h"
#include "Shader/Shader.h"
#include "SwapChain/SwapChain.h"

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

        void RecordCommand(vk::CommandBuffer commandBuffer, vk::RenderPass renderPass, vk::Framebuffer targetFrameBuffer);

    private:
        uint32_t _currentFlight = 0;

        std::unique_ptr<VulkanContext> _pContext = nullptr;
        std::unique_ptr<SwapChain> _pSwapChain = nullptr;
        std::unique_ptr<RenderPass> _pRenderPass = nullptr;

        // temp
        std::unique_ptr<Shader> _pVertShader = nullptr;
        std::unique_ptr<Shader> _pFragShader = nullptr;

        std::unique_ptr<Pipeline> _pPipeline = nullptr;
        std::unique_ptr<BackBuffer> _pBackBuffer = nullptr;
    };
}
