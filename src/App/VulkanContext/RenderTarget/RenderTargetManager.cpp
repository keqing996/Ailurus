#include "RenderTargetManager.h"
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
		if (VulkanContext::GetMSAASamples() != vk::SampleCountFlagBits::e1)
		    CreateMSAATargets(width, height, colorFormat);

		// Create CSM shadow maps
		CreateShadowMapTargets();
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

	uint32_t RenderTargetManager::GetShadowMapCascadeCount() const
	{
		return SHADOW_MAP_CASCADE_COUNT;
	}

	vk::ImageView RenderTargetManager::GetShadowMapImageView(uint32_t cascadeIndex) const
	{
		if (cascadeIndex >= _shadowMapTargets.size())
			return nullptr;
		return _shadowMapTargets[cascadeIndex] ? _shadowMapTargets[cascadeIndex]->GetImageView() : nullptr;
	}

	vk::Image RenderTargetManager::GetShadowMapImage(uint32_t cascadeIndex) const
	{
		if (cascadeIndex >= _shadowMapTargets.size())
			return nullptr;
		return _shadowMapTargets[cascadeIndex] ? _shadowMapTargets[cascadeIndex]->GetImage() : nullptr;
	}

    void RenderTargetManager::Clear()
    {
        _depthTarget = nullptr;
		_msaaColorTarget = nullptr;
		_msaaDepthTarget = nullptr;
		_shadowMapTargets.clear();
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
		const vk::SampleCountFlagBits msaaSamples = VulkanContext::GetMSAASamples();

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

	void RenderTargetManager::CreateShadowMapTargets()
	{
		_shadowMapTargets.clear();
		_shadowMapTargets.reserve(SHADOW_MAP_CASCADE_COUNT);

		for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
		{
			RenderTargetConfig config;
			config.width = SHADOW_MAP_RESOLUTION;
			config.height = SHADOW_MAP_RESOLUTION;
			config.format = vk::Format::eD32Sfloat;
			config.samples = vk::SampleCountFlagBits::e1;
			config.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;
			config.aspectMask = vk::ImageAspectFlagBits::eDepth;
			config.transient = false;

			try
			{
				_shadowMapTargets.push_back(std::make_unique<RenderTarget>(config));
				Logger::LogInfo("Created shadow map cascade {}: {}x{}", i, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION);
			}
			catch (const std::exception& e)
			{
				Logger::LogError("Failed to create shadow map cascade {}: {}", i, e.what());
				_shadowMapTargets.push_back(nullptr);
			}
		}
	}
} // namespace Ailurus
