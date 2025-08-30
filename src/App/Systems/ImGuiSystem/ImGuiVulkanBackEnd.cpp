#include <backends/imgui_impl_vulkan.h>
#include "ImGuiVulkanBackEnd.h"
#include "Ailurus/Application/Application.h"
#include "Ailurus/Application/RenderSystem/RenderSystem.h"
#include "VulkanContext/VulkanContext.h"
#include "VulkanContext/SwapChain/VulkanSwapChain.h"
#include "VulkanContext/RenderPass/VulkanRenderPass.h"

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
		ImDrawData* draw_data = ImGui::GetDrawData();
		const bool minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
		if (!minimized)
		{
			//pCommandBuffer->BeginRenderPass(, VulkanFrameBuffer * pTargetFrameBuffer)
			ImGui_ImplVulkan_RenderDrawData(draw_data, pCommandBuffer->GetBuffer());
		}
	}

	void ImGuiVulkanBackEnd::Init()
	{
        auto pRenderSystem = Application::Get<RenderSystem>();
		auto pRenderPass = pRenderSystem->GetRenderPass(RenderPassType::ImGui);

		ImGui_ImplVulkan_InitInfo imGuiVkInitInfo = {};
		imGuiVkInitInfo.ApiVersion = VulkanContext::GetApiVersion();
		imGuiVkInitInfo.Instance = VulkanContext::GetInstance();
		imGuiVkInitInfo.PhysicalDevice = VulkanContext::GetPhysicalDevice();
		imGuiVkInitInfo.Device = VulkanContext::GetDevice();
		imGuiVkInitInfo.QueueFamily = VulkanContext::GetGraphicQueueIndex();
		imGuiVkInitInfo.Queue = VulkanContext::GetGraphicQueue();
		imGuiVkInitInfo.DescriptorPool = _descriptorPool;
		imGuiVkInitInfo.RenderPass = pRenderPass->GetRHIRenderPass()->GetRenderPass();
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

	void ImGuiVulkanBackEnd::PreRebuildSwapChain()
	{
        Shutdown();
	}

	void ImGuiVulkanBackEnd::PostRebuildSwapChain()
	{
		Init();
	}
} // namespace Ailurus