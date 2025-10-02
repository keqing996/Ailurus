#include <backends/imgui_impl_vulkan.h>
#include "ImGuiVulkanBackEnd.h"
#include "Ailurus/Application/Application.h"
#include "Ailurus/Application/RenderSystem/RenderSystem.h"
#include "VulkanContext/VulkanContext.h"
#include "VulkanContext/SwapChain/VulkanSwapChain.h"

namespace Ailurus
{
	ImGuiVulkanBackEnd::ImGuiVulkanBackEnd()
	{
        auto pRenderSystem = Application::Get<RenderSystem>();
        pRenderSystem->AddCallbackPreSwapChainRebuild(this, [this]() { PreRebuildSwapChain(); });
        pRenderSystem->AddCallbackPostSwapChainRebuild(this, [this]() { PostRebuildSwapChain(); });

		// Create the descriptor pool
		std::vector<vk::DescriptorPoolSize> poolSizes{
			{ vk::DescriptorType::eCombinedImageSampler, IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE }
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
	}

	ImGuiVulkanBackEnd::~ImGuiVulkanBackEnd()
	{
        // Remove callbacks
        auto pRenderSystem = Application::Get<RenderSystem>();
        pRenderSystem->RemoveCallbackPreSwapChainRebuild(this);
        pRenderSystem->RemoveCallbackPostSwapChainRebuild(this);

		// Destroy the descriptor pool
		if (_descriptorPool)
			VulkanContext::GetDevice().destroyDescriptorPool(_descriptorPool);
	}

	void ImGuiVulkanBackEnd::NewFrame()
	{
		ImGui_ImplVulkan_NewFrame();
	}

	void ImGuiVulkanBackEnd::Render(VulkanCommandBuffer* pCommandBuffer)
	{
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), pCommandBuffer->GetBuffer());
	}

	void ImGuiVulkanBackEnd::Init()
	{
        auto pSwapChain = VulkanContext::GetSwapChain();
		vk::Format colorFormat = pSwapChain->GetConfig().surfaceFormat.format;

		ImGui_ImplVulkan_InitInfo imGuiVkInitInfo = {};
		imGuiVkInitInfo.ApiVersion = VulkanContext::GetApiVersion();
		imGuiVkInitInfo.Instance = VulkanContext::GetInstance();
		imGuiVkInitInfo.PhysicalDevice = VulkanContext::GetPhysicalDevice();
		imGuiVkInitInfo.Device = VulkanContext::GetDevice();
		imGuiVkInitInfo.QueueFamily = VulkanContext::GetGraphicQueueIndex();
		imGuiVkInitInfo.Queue = VulkanContext::GetGraphicQueue();
		imGuiVkInitInfo.DescriptorPool = _descriptorPool;
		imGuiVkInitInfo.RenderPass = VK_NULL_HANDLE; // Dynamic rendering, no render pass
		imGuiVkInitInfo.Subpass = 0;
		imGuiVkInitInfo.MinImageCount = 2;
		imGuiVkInitInfo.ImageCount = pSwapChain->GetConfig().imageCount;
		imGuiVkInitInfo.MSAASamples = static_cast<VkSampleCountFlagBits>(vk::SampleCountFlagBits::e1);
		imGuiVkInitInfo.UseDynamicRendering = true;
		
		// For dynamic rendering, we need to specify the color attachment format
		imGuiVkInitInfo.PipelineRenderingCreateInfo = {};
		imGuiVkInitInfo.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
		imGuiVkInitInfo.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
		imGuiVkInitInfo.PipelineRenderingCreateInfo.pColorAttachmentFormats = reinterpret_cast<VkFormat*>(&colorFormat);
		
		ImGui_ImplVulkan_Init(&imGuiVkInitInfo);
	}

	void ImGuiVulkanBackEnd::Shutdown()
	{
		ImGui_ImplVulkan_Shutdown();
	}

	void ImGuiVulkanBackEnd::PreRebuildSwapChain()
	{
        Shutdown();
	}

	void ImGuiVulkanBackEnd::PostRebuildSwapChain()
	{
		Init();
	}
} // namespace Ailurus