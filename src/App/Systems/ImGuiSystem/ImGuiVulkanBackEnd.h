#pragma once

#include <memory>
#include <vulkan/vulkan.hpp>

namespace Ailurus
{
    class VulkanCommandBuffer;
    class VulkanRenderPassImGui;

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
        vk::DescriptorPool _descriptorPool;
        vk::RenderPass _renderPass;
    };
}