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
                 bool enableValidationLayer)
        {
            _pContext = std::make_unique<VulkanContext>(getWindowSize, getWindowInstExt,
                                                        createSurface, destroySurface, enableValidationLayer);

            Vector2i windowSize = getWindowSize();
            _pSwapChain = SwapChain::Create(_pContext.get(), windowSize.x(), windowSize.y());
            _pRenderPass = RenderPass::Create(_pContext.get(), _pSwapChain.get());
            _pBackBuffer = std::make_unique<BackBuffer>(_pContext.get(), _pSwapChain.get(), _pRenderPass.get());

            _pVertShader = Shader::Create(_pContext.get(), ShaderStage::Vertex, "./triangle.vert.spv");
            _pFragShader = Shader::Create(_pContext.get(), ShaderStage::Fragment, "./triangle.frag.spv");

            _pPipeline = std::make_unique<Pipeline>(_pContext.get(), _pRenderPass.get());
            _pPipeline->AddShader(_pVertShader.get());
            _pPipeline->AddShader(_pFragShader.get());
            _pPipeline->GeneratePipeline();
        }

        ~Renderer()
        {
        }

    private:
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
