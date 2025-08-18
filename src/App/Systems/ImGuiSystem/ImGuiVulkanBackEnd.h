#pragma once

#include <vulkan/vulkan.hpp>

namespace Ailurus
{
    class ImGuiVulkanBackEnd
    {
    public:
        ImGuiVulkanBackEnd();
        ~ImGuiVulkanBackEnd();

    public:
        void Init();
        void Shutdown();

    private:
        vk::DescriptorPool _descriptorPool;
        vk::RenderPass _renderPass;
    };
}