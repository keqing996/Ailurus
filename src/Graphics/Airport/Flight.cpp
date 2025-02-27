#include "Ailurus/Graphics/Airport/Flight.h"
#include "Ailurus/Graphics/Renderer.h"
#include "Ailurus/Graphics/RenderPass/RenderPass.h"
#include "Ailurus/Graphics/RenderObject/RenderObject.h"
#include "Ailurus/Graphics/DataBuffer/VertexBuffer.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
    Flight::Flight(const Renderer* pRenderer)
        : _pRenderer(pRenderer)
    {
    }

    void Flight::Begin()
    {
        vk::CommandBufferBeginInfo beginInfo;
        beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        commandBuffer.begin(beginInfo);
    }

    void Flight::End()
    {
        commandBuffer.end();
    }

    void Flight::BeginRenderPass(RenderPass* pRenderPass)
    {
        if (_pCurrentRenderPass != nullptr)
        {
            Logger::LogError("Command buffer begin render pass while last render pass not ended");
            return;
        }

        _pCurrentRenderPass = pRenderPass;

        commandBuffer.beginRenderPass(pRenderPass->GetRenderPassBeginInfo(*this), {});
    }

    void Flight::EndRenderPass()
    {
        commandBuffer.endRenderPass();
        _pCurrentRenderPass = nullptr;
    }

    void Flight::DrawObject(const RenderObject* pRenderObject)
    {
        if (_pCurrentRenderPass == nullptr)
        {
            Logger::LogError("Command buffer draw object but not in any render pass");
            return;
        }

        auto optStageShaders = pRenderObject->GetRenderPassShaders(_pCurrentRenderPass->GetRenderPassType());
        if (!optStageShaders.has_value())
            return; // This object should not be drawn under this pass;

        PipelineConfig pipelineConfig;
        pipelineConfig.pInputAssemble = pRenderObject->GetInputAssemble();
        pipelineConfig.shaderStages = *optStageShaders.value();

        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pCurrentRenderPass->GetPipeline(pipelineConfig)->GetPipeline());

        auto extent = _pRenderer->GetSwapChain()->GetSwapChainConfig().extent;
        vk::Viewport viewport(0.0f, 0.0f, extent.width, extent.height, 0.0f, 1.0f);
        vk::Rect2D scissor(vk::Offset2D{0, 0}, extent);

        commandBuffer.setViewport(0, 1, &viewport);
        commandBuffer.setScissor(0, 1, &scissor);

        vk::Buffer vertexBuffers[] = { pRenderObject->GetInputAssemble()->GetVertexBuffer()->GetBuffer() };
        vk::DeviceSize offsets[] = { 0 };
        commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

        commandBuffer.draw(pRenderObject->GetInputAssemble()->GetVertexCount(), 1, 0, 0);
    }
}
