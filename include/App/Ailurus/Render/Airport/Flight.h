#pragma once

#include <vulkan/vulkan.hpp>

namespace Ailurus
{
    class Renderer;
    class RenderPass;
    class RenderObject;

    struct Flight
    {
    public:
        uint32_t imageIndex;
        vk::CommandBuffer commandBuffer;
        vk::Semaphore imageReadySemaphore;
        vk::Semaphore renderFinishSemaphore;
        vk::Fence fence;

    public:
        explicit Flight(const Renderer* pRenderer);

    public:
        void Begin();
        void End();

        void BeginRenderPass(RenderPass* pRenderPass);
        void EndRenderPass();

        void DrawObject(const RenderObject* pRenderObject);

    private:
        const Renderer* _pRenderer = nullptr;
        RenderPass* _pCurrentRenderPass = nullptr;
    };
}