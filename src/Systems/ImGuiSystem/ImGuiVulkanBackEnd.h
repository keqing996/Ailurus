#pragma once

#include <vulkan/vulkan.hpp>

namespace Ailurus
{
    class VulkanCommandBuffer;
    class RenderPass;

    class ImGuiVulkanBackEnd
    {
    public:
        ImGuiVulkanBackEnd();
        ~ImGuiVulkanBackEnd();

    public:
    	void NewFrame();
        void Render(VulkanCommandBuffer* pCommandBuffer);
        void Init();
        void Shutdown();

    private:
        void PreRebuildSwapChain();
        void PostRebuildSwapChain();

    private:
        vk::DescriptorPool _descriptorPool;
    };
}