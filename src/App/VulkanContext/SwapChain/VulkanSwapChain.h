#pragma once

#include <cstdint>
#include <optional>
#include <vector>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>
#include "SwapChainConfig.h"
#include "VulkanContext/CommandBuffer/VulkanCommandBuffer.h"
#include "VulkanContext/Semaphore/VulkanSemaphore.h"

namespace Ailurus
{
	class VulkanSwapChain: public NonCopyable, public NonMovable
	{
    public:
        VulkanSwapChain();
        ~VulkanSwapChain();

	public:
		auto GetConfig() const -> const SwapChainConfig&;
		auto GetSwapChain() const -> const vk::SwapchainKHR&;
		auto GetSwapChainImages() const -> const std::vector<vk::Image>&;
		auto GetSwapChainImageViews() const -> const std::vector<vk::ImageView>&;
		auto AcquireNextImage(VulkanSemaphore* imageReadySem, bool* needRebuildSwapChain) -> std::optional<uint32_t>;

	private:
		SwapChainConfig _swapChainConfig{};
		vk::SwapchainKHR _vkSwapChain = nullptr;
		std::vector<vk::Image> _vkSwapChainImages{};
		std::vector<vk::ImageView> _vkSwapChainImageViews{};
	};
} // namespace Ailurus