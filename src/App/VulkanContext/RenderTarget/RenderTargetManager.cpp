#include "RenderTargetManager.h"
#include "Ailurus/Application/Application.h"
#include "VulkanContext/VulkanContext.h"
#include "VulkanContext/SwapChain/VulkanSwapChain.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
	RenderTargetManager::RenderTargetManager()
	{
        Rebuild();
	}

	RenderTargetManager::~RenderTargetManager()
	{
		Clear();
	}

    void RenderTargetManager::Rebuild()
    {
        Clear();

        const auto& swapChainConfig = VulkanContext::GetSwapChain()->GetConfig();
        const uint32_t width = swapChainConfig.extent.width;
        const uint32_t height = swapChainConfig.extent.height;
        const vk::Format colorFormat = swapChainConfig.surfaceFormat.format;

		// Create depth target (always needed)
		CreateDepthTarget(width, height);

		// Create MSAA targets if MSAA is enabled
		CreateMSAATargets(width, height, colorFormat);
    }

	vk::ImageView RenderTargetManager::GetDepthImageView() const
	{
		return _depthTarget ? _depthTarget->GetImageView() : nullptr;
	}

	vk::ImageView RenderTargetManager::GetMSAAColorImageView() const
	{
		return _msaaColorTarget ? _msaaColorTarget->GetImageView() : nullptr;
	}

	vk::ImageView RenderTargetManager::GetMSAADepthImageView() const
	{
		return _msaaDepthTarget ? _msaaDepthTarget->GetImageView() : nullptr;
	}

    void RenderTargetManager::Clear()
    {
        _depthTarget = nullptr;
		_msaaColorTarget = nullptr;
		_msaaDepthTarget = nullptr;
    }

	void RenderTargetManager::CreateDepthTarget(uint32_t width, uint32_t height)
	{
		RenderTargetConfig config;
		config.width = width;
		config.height = height;
		config.format = vk::Format::eD32Sfloat;
		config.samples = vk::SampleCountFlagBits::e1; // Non-MSAA depth
		config.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
		config.aspectMask = vk::ImageAspectFlagBits::eDepth;
		config.transient = false;

		try
		{
			_depthTarget = std::make_unique<RenderTarget>(config);
			Logger::LogInfo("Created depth target: {}x{}", width, height);
		}
		catch (const std::exception& e)
		{
			Logger::LogError("Failed to create depth target: {}", e.what());
		}
	}

	void RenderTargetManager::CreateMSAATargets(uint32_t width, uint32_t height, vk::Format colorFormat)
	{
		bool enabled = Application::GraphicsSetting::IsMSAAEnabled();
		if (!enabled)
			return;

		const vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e4;

		// Create MSAA color target
		{
			RenderTargetConfig config;
			config.width = width;
			config.height = height;
			config.format = colorFormat;
			config.samples = msaaSamples;
			config.usage = vk::ImageUsageFlagBits::eColorAttachment;
			config.aspectMask = vk::ImageAspectFlagBits::eColor;
			config.transient = true; // Enable transient optimization for MSAA

			try
			{
				_msaaColorTarget = std::make_unique<RenderTarget>(config);
				Logger::LogInfo("Created MSAA color target: {}x{} ({}x samples)", 
					width, height, static_cast<uint32_t>(msaaSamples));
			}
			catch (const std::exception& e)
			{
				Logger::LogError("Failed to create MSAA color target: {}", e.what());
			}
		}

		// Create MSAA depth target
		{
			RenderTargetConfig config;
			config.width = width;
			config.height = height;
			config.format = vk::Format::eD32Sfloat;
			config.samples = msaaSamples;
			config.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
			config.aspectMask = vk::ImageAspectFlagBits::eDepth;
			config.transient = true; // Enable transient optimization for MSAA

			try
			{
				_msaaDepthTarget = std::make_unique<RenderTarget>(config);
				Logger::LogInfo("Created MSAA depth target: {}x{} ({}x samples)", 
					width, height, static_cast<uint32_t>(msaaSamples));
			}
			catch (const std::exception& e)
			{
				Logger::LogError("Failed to create MSAA depth target: {}", e.what());
			}
		}
	}
} // namespace Ailurus
