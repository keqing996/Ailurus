#pragma once

#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>
#include "SwapChainConfig.h"

namespace Ailurus
{
	class VulkanSwapChain: public NonCopyable, public NonMovable
	{
    public:
        VulkanSwapChain();
        ~VulkanSwapChain();

	public:
		const SwapChainConfig& GetConfig() const;
		const vk::SwapchainKHR& GetSwapChain() const;
		const std::vector<vk::Image>& GetSwapChainImages() const;
		const std::vector<vk::ImageView>& GetSwapChainImageViews() const;

	private:
		SwapChainConfig _swapChainConfig{};
		vk::SwapchainKHR _vkSwapChain = nullptr;
		std::vector<vk::Image> _vkSwapChainImages{};
		std::vector<vk::ImageView> _vkSwapChainImageViews{};
	};
} // namespace Ailurus