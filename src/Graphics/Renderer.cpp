#include <array>
#include "Ailurus/Graphics/Renderer.h"

#include <Ailurus/Utility/Logger.h>

namespace Ailurus
{
    Renderer::Renderer(const GetWindowSizeCallback& getWindowSize, const GetWindowInstanceExtension& getWindowInstExt,
        const WindowCreateSurfaceCallback& createSurface, const WindowDestroySurfaceCallback& destroySurface,
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

    Renderer::~Renderer()
    {
    }

    void Renderer::Render()
    {
        if (_needRebuildSwapChain)
            RecreateSwapChain();

        auto vkLogicalDevice = _pContext->GetLogicalDevice();
        auto imageAvailableSemaphores = _pContext->GetImageAvailableSemaphores();
        auto renderFinishSemaphores = _pContext->GetRenderFinishSemaphores();
        auto vkFences = _pContext->GetFences();
        auto vkSwapChain = _pSwapChain->GetSwapChain();
        auto vkCommandBuffers = _pContext->GetCommandBuffers();
        auto vkRenderPass = _pRenderPass->GetRenderPass();
        auto vkBackBuffers = _pBackBuffer->GetBackBuffers();
        auto vkGraphicsQueue = _pContext->GetGraphicQueue();
        auto vkPresentQueue = _pContext->GetPresentQueue();
        auto timeout = std::numeric_limits<uint64_t>::max();

        auto acquireImage = vkLogicalDevice.acquireNextImageKHR(vkSwapChain, timeout, imageAvailableSemaphores[_currentFlight]);
        if (acquireImage.result == vk::Result::eErrorOutOfDateKHR || acquireImage.result == vk::Result::eSuboptimalKHR)
            _needRebuildSwapChain = true;

        if (acquireImage.result == vk::Result::eErrorOutOfDateKHR)
            return;

        if (acquireImage.result != vk::Result::eSuccess)
        {
            Logger::LogError("Fail to acquire next image, result = {}", static_cast<int>(acquireImage.result));
            return;
        }

        auto waitFence = vkLogicalDevice.waitForFences(vkFences[_currentFlight], true, timeout);
        if (waitFence != vk::Result::eSuccess)
        {
            Logger::LogError("Fail to wait fences, result = {}", static_cast<int>(waitFence));
            return;
        }

        vkLogicalDevice.resetFences(vkFences[_currentFlight]);

        uint32_t imageIndex = acquireImage.value;

        auto vkCommandBuffer = vkCommandBuffers[_currentFlight];
        RecordCommand(vkCommandBuffer, vkRenderPass, vkBackBuffers[imageIndex]);

        std::array waitSemaphores = { imageAvailableSemaphores[_currentFlight] };
        std::array signalSemaphores = { renderFinishSemaphores[_currentFlight] };
        std::array<vk::PipelineStageFlags, 1> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

        vk::SubmitInfo submitInfo;
        submitInfo.setWaitSemaphores(waitSemaphores)
                .setSignalSemaphores(signalSemaphores)
                .setWaitDstStageMask(waitStages)
                .setCommandBuffers(vkCommandBuffer);

        auto submit = vkGraphicsQueue.submit(1, &submitInfo, vkFences[_currentFlight]);
        if (submit != vk::Result::eSuccess)
        {
            Logger::LogError("Fail to submit command buffer, result = {}", static_cast<int>(submit));
            return;
        }

        vk::PresentInfoKHR presentInfo;
        presentInfo.setWaitSemaphores(signalSemaphores)
                .setSwapchains(vkSwapChain)
                .setImageIndices(imageIndex);

        auto present = vkPresentQueue.presentKHR(presentInfo);
        if (present == vk::Result::eErrorOutOfDateKHR || present == vk::Result::eSuboptimalKHR)
            _needRebuildSwapChain = true;

        if (present == vk::Result::eErrorOutOfDateKHR)
            return;

        if (present != vk::Result::eSuccess)
        {
            Logger::LogError("Fail to present, result = {}", static_cast<int>(present));
            return;
        }

        _currentFlight = (_currentFlight + 1) % VulkanContext::PARALLEL_FRAME_COUNT;
    }

    void Renderer::RecordCommand(vk::CommandBuffer commandBuffer, vk::RenderPass renderPass,
        vk::Framebuffer targetFrameBuffer)
    {
        commandBuffer.reset();

        auto extent = _pSwapChain->GetSwapChainConfig().extent;

        vk::CommandBufferBeginInfo beginInfo;
        beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

        commandBuffer.begin(beginInfo);
        {
            vk::ClearValue clearColor({0.0f, 0.0f, 0.0f, 1.0f});

            vk::RenderPassBeginInfo renderPassInfo;
            renderPassInfo.setRenderPass(renderPass)
                    .setFramebuffer(targetFrameBuffer)
                    .setRenderArea(vk::Rect2D{
                        vk::Offset2D{0, 0},
                        extent
                    })
                    .setClearValues(clearColor);

            commandBuffer.beginRenderPass(renderPassInfo, {});
            {
                vk::Viewport viewport(0.0f, 0.0f, extent.width, extent.height, 0.0f, 1.0f);
                vk::Rect2D scissor(vk::Offset2D{0, 0}, extent);

                commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pPipeline->GetPipeline());
                commandBuffer.setViewport(0, 1, &viewport);
                commandBuffer.setScissor(0, 1, &scissor);
                commandBuffer.draw(3, 1, 0, 0);
            }
            commandBuffer.endRenderPass();
        }
        commandBuffer.end();
    }

    void Renderer::NeedRecreateSwapChain()
    {
        _needRebuildSwapChain = true;
    }

    void Renderer::RecreateSwapChain()
    {
        _pContext->GetLogicalDevice().waitIdle();

        _pBackBuffer.reset();
        _pSwapChain.reset();

        auto windowSize = _pContext->GetWindowSize();
        _pSwapChain = SwapChain::Create(_pContext.get(), windowSize.x(), windowSize.y());
        _pBackBuffer = std::make_unique<BackBuffer>(_pContext.get(), _pSwapChain.get(), _pRenderPass.get());

        _needRebuildSwapChain = false;
    }
}
