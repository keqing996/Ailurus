#include <backends/imgui_impl_vulkan.h>
#include "ImGuiVulkanBackEnd.h"
#include "VulkanContext/VulkanContext.h"
#include "VulkanContext/SwapChain/VulkanSwapChain.h"

namespace Ailurus
{
	ImGuiVulkanBackEnd::ImGuiVulkanBackEnd()
	{
        // Create the descriptor pool
        std::vector<vk::DescriptorPoolSize> poolSizes {
            {vk::DescriptorType::eCombinedImageSampler, IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE}
        };

        uint32_t maxSets = 0;
        for (const vk::DescriptorPoolSize& pool_size : poolSizes)
            maxSets += pool_size.descriptorCount;

        vk::DescriptorPoolCreateInfo poolCreateInfo{};
        poolCreateInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
            .setMaxSets(maxSets)
            .setPoolSizeCount(poolSizes.size())
            .setPoolSizes(poolSizes);

        _descriptorPool = VulkanContext::GetDevice().createDescriptorPool(poolCreateInfo);
        
        // Create the render pass
        vk::AttachmentDescription attachment = {};
        attachment.setFormat(VulkanContext::GetSwapChain()->GetConfig().surfaceFormat.format)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

        vk::AttachmentReference colorAttachment = {};
        colorAttachment.setAttachment(0)
            .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

        vk::SubpassDescription subpass = {};
        subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
            .setColorAttachments(colorAttachment);

        vk::SubpassDependency dependency = {};
        dependency.setSrcSubpass(vk::SubpassExternal)
            .setDstSubpass(0)
            .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setSrcAccessMask(vk::AccessFlagBits::eNone)
            .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

        vk::RenderPassCreateInfo renderPassCreateInfo = {};
        renderPassCreateInfo.setAttachments(attachment)
            .setSubpasses(subpass)
            .setDependencies(dependency);

        _renderPass = VulkanContext::GetDevice().createRenderPass(renderPassCreateInfo);
	}

	ImGuiVulkanBackEnd::~ImGuiVulkanBackEnd()
	{
        // Destroy the render pass
        if (_renderPass)
            VulkanContext::GetDevice().destroyRenderPass(_renderPass);

        // Destroy the descriptor pool
        if (_descriptorPool)
            VulkanContext::GetDevice().destroyDescriptorPool(_descriptorPool);
	}

	void ImGuiVulkanBackEnd::NewFrame()
	{
		ImGui_ImplVulkan_NewFrame();
	}

	void ImGuiVulkanBackEnd::Init()
    {
        ImGui_ImplVulkan_InitInfo imGuiVkInitInfo = {};
        imGuiVkInitInfo.ApiVersion = VulkanContext::GetApiVersion();
        imGuiVkInitInfo.Instance = VulkanContext::GetInstance();
        imGuiVkInitInfo.PhysicalDevice = VulkanContext::GetPhysicalDevice();
        imGuiVkInitInfo.Device = VulkanContext::GetDevice();
        imGuiVkInitInfo.QueueFamily = VulkanContext::GetGraphicQueueIndex();
        imGuiVkInitInfo.Queue = VulkanContext::GetGraphicQueue();
        imGuiVkInitInfo.DescriptorPool = _descriptorPool;
        imGuiVkInitInfo.RenderPass = _renderPass;
        imGuiVkInitInfo.Subpass = 0;
        imGuiVkInitInfo.MinImageCount = 2;
        imGuiVkInitInfo.ImageCount = VulkanContext::GetSwapChain()->GetConfig().imageCount;
        imGuiVkInitInfo.MSAASamples = static_cast<VkSampleCountFlagBits>(vk::SampleCountFlagBits::e1);
        ImGui_ImplVulkan_Init(&imGuiVkInitInfo);
    }

    void ImGuiVulkanBackEnd::Shutdown()
    {
        ImGui_ImplVulkan_Shutdown();
    }
} // namespace Ailurus