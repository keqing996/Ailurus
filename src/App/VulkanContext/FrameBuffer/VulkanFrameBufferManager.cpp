#include "VulkanFrameBuffer.h"
#include "VulkanFrameBufferManager.h"
#include "VulkanContext/VulkanContext.h"
#include "VulkanContext/SwapChain/VulkanSwapChain.h"
#include "VulkanContext/RenderPass/VulkanRenderPass.h"

namespace Ailurus
{
	bool VulkanFrameBufferManager::BackBufferKeyEqual::operator()(const BackBufferKey& lhs, const BackBufferKey& rhs) const
	{
		return lhs.pRenderPass == rhs.pRenderPass && lhs.imageIndex == rhs.imageIndex;
	}

	std::size_t VulkanFrameBufferManager::BackBufferKeyHash::operator()(const BackBufferKey& key) const
	{
		return std::hash<VulkanRenderPass*>()(key.pRenderPass) ^ std::hash<uint32_t>()(key.imageIndex);
	}

	VulkanFrameBufferManager::VulkanFrameBufferManager()
	{
	}

	VulkanFrameBufferManager::~VulkanFrameBufferManager()
	{
	}

	void VulkanFrameBufferManager::ClearBackBuffers()
	{
		_vkBackBuffers.clear();
	}

	VulkanFrameBuffer* VulkanFrameBufferManager::GetBackBuffer(VulkanRenderPass* pRenderPass, uint32_t imageIndex)
	{
		auto key = BackBufferKey{ pRenderPass, imageIndex };
		auto it = _vkBackBuffers.find(key);
		if (it != _vkBackBuffers.end())
			return it->second.get();

		const auto vkLogicalDevice = VulkanContext::GetDevice();
		const auto extent = VulkanContext::GetSwapChain()->GetConfig().extent;
		auto& swapChainImageViews = VulkanContext::GetSwapChain()->GetSwapChainImageViews();

		vk::FramebufferCreateInfo framebufferInfo;
		framebufferInfo.setRenderPass(pRenderPass->GetRenderPass())
			.setAttachments(swapChainImageViews[imageIndex])
			.setWidth(extent.width)
			.setHeight(extent.height)
			.setLayers(1);

        auto vkFrameBuffer = vkLogicalDevice.createFramebuffer(framebufferInfo);
        auto pFrameBuffer = std::make_unique<VulkanFrameBuffer>(vkFrameBuffer);

        auto result = pFrameBuffer.get();
        _vkBackBuffers[key] = std::move(pFrameBuffer);
        
        return result;
	}
} // namespace Ailurus