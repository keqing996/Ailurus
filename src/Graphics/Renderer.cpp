#include <array>
#include "Ailurus/Graphics/Renderer.h"

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

        auto waitFence = vkLogicalDevice.waitForFences(1, &vkFences[_currentFlight], true, timeout);
        if (waitFence != vk::Result::eSuccess)
            return;

        auto resetFence = vkLogicalDevice.resetFences(1, &vkFences[_currentFlight]);
        if (resetFence != vk::Result::eSuccess)
            return;

        auto acquireImage = vkLogicalDevice.acquireNextImageKHR(vkSwapChain, timeout, imageAvailableSemaphores[_currentFlight]);
        if (acquireImage.result != vk::Result::eSuccess)
            return;

        uint32_t imageIndex = acquireImage.value;

        auto vkCommandBuffer = vkCommandBuffers[_currentFlight];
        vkCommandBuffer.reset();
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
            return;

        vk::PresentInfoKHR presentInfo;
        presentInfo.setWaitSemaphores(signalSemaphores)
                .setSwapchains(vkSwapChain)
                .setImageIndices(imageIndex);

        auto present = vkPresentQueue.presentKHR(presentInfo);
        if (present != vk::Result::eSuccess)
            return;

        _currentFlight = (_currentFlight + 1) % VulkanContext::PARALLEL_FRAME_COUNT;
    }

    void Renderer::RecordCommand(vk::CommandBuffer commandBuffer, vk::RenderPass renderPass,
        vk::Framebuffer targetFrameBuffer)
    {
        auto extent = _pSwapChain->GetSwapChainConfig().extent;

        vk::CommandBufferBeginInfo beginInfo;
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
}
