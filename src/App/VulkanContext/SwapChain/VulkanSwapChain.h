#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>
#include <sys/types.h>
#include "SwapChainConfig.h"
#include "VulkanContext/CommandBuffer/VulkanCommandBuffer.h"
#include "VulkanContext/Descriptor/VulkanDescriptorAllocator.h"
#include "VulkanContext/Fence/VulkanFence.h"
#include "VulkanContext/Semaphore/VulkanSemaphore.h"

namespace Ailurus
{
	class VulkanSwapChain: public NonCopyable, public NonMovable
	{
	public:
		struct FrameContext
		{
			uint64_t frameCount;
			uint32_t imageIndex;
			std::unique_ptr<VulkanCommandBuffer> pRenderingCommandBuffer;
			std::unique_ptr<VulkanDescriptorAllocator> pFrameDescriptorAllocator;
			std::unique_ptr<VulkanSemaphore> pImageReadySemaphore;
			std::unique_ptr<VulkanSemaphore> pRenderFinishedSemaphore;
			std::unique_ptr<VulkanFence> pRenderFinishFence;
		};

    public:
        VulkanSwapChain();
        ~VulkanSwapChain();

	public:
		auto GetConfig() const -> const SwapChainConfig&;
		auto GetSwapChain() const -> const vk::SwapchainKHR&;
		auto GetSwapChainImages() const -> const std::vector<vk::Image>&;
		auto GetSwapChainImageViews() const -> const std::vector<vk::ImageView>&;
		auto GetCurrentImageIndex() const -> uint32_t;
		
	private:
		auto AcquireNextImage(bool* needRebuildSwapChain) -> std::unique_ptr<FrameContext>;

	private:
		SwapChainConfig _swapChainConfig{};
		vk::SwapchainKHR _vkSwapChain = nullptr;
		std::vector<vk::Image> _vkSwapChainImages{};
		std::vector<vk::ImageView> _vkSwapChainImageViews{};
		uint32_t _currentImageIndex = 0;
		std::vector<std::unique_ptr<FrameContext>> _frameContext{};

		// Pool for image ready semaphores.
		std::deque<std::unique_ptr<VulkanSemaphore>> _availableImageReadySemaphores{};
	};
} // namespace Ailurus