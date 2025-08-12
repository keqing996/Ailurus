#pragma once

#include <cstdint>
#include <unordered_map>
#include <memory>
#include <optional>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>
#include "SwapChainConfig.h"
#include "VulkanContext/Fence/VulkanFence.h"
#include "VulkanContext/Semaphore/VulkanSemaphore.h"

namespace Ailurus
{
	class VulkanSwapChain: public NonCopyable, public NonMovable
	{
	public:
		struct AcquireResult
		{
			uint32_t imageIndex;
			VulkanSemaphore* pImageReadySemaphore;
			VulkanSemaphore* pRenderFinishedSemaphore;
			VulkanFence* pRenderFinishFence;
		};

    public:
        VulkanSwapChain();
        ~VulkanSwapChain();

	public:
		const SwapChainConfig& GetConfig() const;
		const vk::SwapchainKHR& GetSwapChain() const;
		const std::vector<vk::Image>& GetSwapChainImages() const;
		const std::vector<vk::ImageView>& GetSwapChainImageViews() const;
		uint32_t GetCurrentImageIndex() const;
		std::optional<AcquireResult> AcquireNextImage(bool* needRebuildSwapChain);

	private:
		struct FrameSyncObjects
		{
			std::unique_ptr<VulkanSemaphore> pImageReadySemaphore;
			std::unique_ptr<VulkanSemaphore> pRenderFinishedSemaphore;
			std::unique_ptr<VulkanFence> pRenderFinishFence;
		};

		SwapChainConfig _swapChainConfig{};
		vk::SwapchainKHR _vkSwapChain = nullptr;
		std::vector<vk::Image> _vkSwapChainImages{};
		std::vector<vk::ImageView> _vkSwapChainImageViews{};
		uint32_t _currentImageIndex = 0;
		std::unordered_map<uint32_t, FrameSyncObjects> _frameSyncObjectsMap{};
	};
} // namespace Ailurus