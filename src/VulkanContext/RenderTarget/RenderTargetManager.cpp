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

		// Create offscreen HDR color target (always needed for post-process)
		CreateOffscreenColorTarget(width, height);

		// Create depth target (always needed)
		CreateDepthTarget(width, height);

		// Create MSAA targets if MSAA is enabled
		if (VulkanContext::GetMSAASamples() != vk::SampleCountFlagBits::e1)
		    CreateMSAATargets(width, height);

		// Create CSM shadow maps
		CreateShadowMapTargets();

		// Create G-Buffer targets
		CreateGBufferTargets(width, height);
    }

	vk::Image RenderTargetManager::GetDepthImage() const
	{
		return _depthTarget ? _depthTarget->GetImage() : nullptr;
	}

	vk::ImageView RenderTargetManager::GetDepthImageView() const
	{
		return _depthTarget ? _depthTarget->GetImageView() : nullptr;
	}

	vk::Image RenderTargetManager::GetMSAAColorImage() const
	{
		return _msaaColorTarget ? _msaaColorTarget->GetImage() : nullptr;
	}

	vk::ImageView RenderTargetManager::GetMSAAColorImageView() const
	{
		return _msaaColorTarget ? _msaaColorTarget->GetImageView() : nullptr;
	}

	vk::Image RenderTargetManager::GetMSAADepthImage() const
	{
		return _msaaDepthTarget ? _msaaDepthTarget->GetImage() : nullptr;
	}

	vk::ImageView RenderTargetManager::GetMSAADepthImageView() const
	{
		return _msaaDepthTarget ? _msaaDepthTarget->GetImageView() : nullptr;
	}

	vk::Image RenderTargetManager::GetResolvedMSAADepthImage() const
	{
		return _resolvedMSAADepthTarget ? _resolvedMSAADepthTarget->GetImage() : nullptr;
	}

	vk::ImageView RenderTargetManager::GetResolvedMSAADepthImageView() const
	{
		return _resolvedMSAADepthTarget ? _resolvedMSAADepthTarget->GetImageView() : nullptr;
	}

	vk::Image RenderTargetManager::GetOffscreenColorImage() const
	{
		return _offscreenColorTarget ? _offscreenColorTarget->GetImage() : nullptr;
	}

	vk::ImageView RenderTargetManager::GetOffscreenColorImageView() const
	{
		return _offscreenColorTarget ? _offscreenColorTarget->GetImageView() : nullptr;
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
		_resolvedMSAADepthTarget = nullptr;
		_offscreenColorTarget = nullptr;
		_shadowMapTargets.clear();
		_gBufferNormalTarget = nullptr;
		_gBufferAlbedoTarget = nullptr;
		_gBufferMetallicTarget = nullptr;
    }

	void RenderTargetManager::CreateDepthTarget(uint32_t width, uint32_t height)
	{
		RenderTargetConfig config;
		config.width = width;
		config.height = height;
		config.format = vk::Format::eD32Sfloat;
		config.samples = vk::SampleCountFlagBits::e1; // Non-MSAA depth
		config.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;
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

	void RenderTargetManager::CreateMSAATargets(uint32_t width, uint32_t height)
	{
		const vk::SampleCountFlagBits msaaSamples = VulkanContext::GetMSAASamples();

		// Create MSAA color target: use HDR format to match offscreen RT for resolve
		// Both MSAA color and the offscreen resolve target must share the same format
		// (OFFSCREEN_COLOR_FORMAT = R16G16B16A16_SFLOAT for HDR rendering)
		{
			RenderTargetConfig config;
			config.width = width;
			config.height = height;
			config.format = OFFSCREEN_COLOR_FORMAT;
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

		// Create single-sampled resolved depth target for SSAO sampling under MSAA.
		// This target is also suitable for a later explicit fallback resolve path.
		{
			RenderTargetConfig config;
			config.width = width;
			config.height = height;
			config.format = vk::Format::eD32Sfloat;
			config.samples = vk::SampleCountFlagBits::e1;
			config.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;
			config.aspectMask = vk::ImageAspectFlagBits::eDepth;
			config.transient = false;

			try
			{
				_resolvedMSAADepthTarget = std::make_unique<RenderTarget>(config);
				Logger::LogInfo("Created resolved MSAA depth target: {}x{}", width, height);
			}
			catch (const std::exception& e)
			{
				Logger::LogError("Failed to create resolved MSAA depth target: {}", e.what());
			}
		}
	}

	void RenderTargetManager::CreateOffscreenColorTarget(uint32_t width, uint32_t height)
	{
		RenderTargetConfig config;
		config.width = width;
		config.height = height;
		config.format = OFFSCREEN_COLOR_FORMAT;
		config.samples = vk::SampleCountFlagBits::e1;
		config.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled
			| vk::ImageUsageFlagBits::eTransferSrc;
		config.aspectMask = vk::ImageAspectFlagBits::eColor;
		config.transient = false;

		try
		{
			_offscreenColorTarget = std::make_unique<RenderTarget>(config);
			Logger::LogInfo("Created offscreen HDR color target: {}x{}", width, height);
		}
		catch (const std::exception& e)
		{
			Logger::LogError("Failed to create offscreen HDR color target: {}", e.what());
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

	vk::Image RenderTargetManager::GetGBufferNormalImage() const
	{
		return _gBufferNormalTarget ? _gBufferNormalTarget->GetImage() : nullptr;
	}

	vk::ImageView RenderTargetManager::GetGBufferNormalImageView() const
	{
		return _gBufferNormalTarget ? _gBufferNormalTarget->GetImageView() : nullptr;
	}

	vk::Image RenderTargetManager::GetGBufferAlbedoImage() const
	{
		return _gBufferAlbedoTarget ? _gBufferAlbedoTarget->GetImage() : nullptr;
	}

	vk::ImageView RenderTargetManager::GetGBufferAlbedoImageView() const
	{
		return _gBufferAlbedoTarget ? _gBufferAlbedoTarget->GetImageView() : nullptr;
	}

	vk::Image RenderTargetManager::GetGBufferMetallicImage() const
	{
		return _gBufferMetallicTarget ? _gBufferMetallicTarget->GetImage() : nullptr;
	}

	vk::ImageView RenderTargetManager::GetGBufferMetallicImageView() const
	{
		return _gBufferMetallicTarget ? _gBufferMetallicTarget->GetImageView() : nullptr;
	}

	void RenderTargetManager::CreateGBufferTargets(uint32_t width, uint32_t height)
	{
		// GBuffer0: World Normal (XYZ) + AO (W)
		{
			RenderTargetConfig config;
			config.width = width;
			config.height = height;
			config.format = GBUFFER_NORMAL_FORMAT;
			config.samples = vk::SampleCountFlagBits::e1;
			config.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
			config.aspectMask = vk::ImageAspectFlagBits::eColor;
			config.transient = false;

			try
			{
				_gBufferNormalTarget = std::make_unique<RenderTarget>(config);
				Logger::LogInfo("Created G-Buffer Normal target: {}x{}", width, height);
			}
			catch (const std::exception& e)
			{
				Logger::LogError("Failed to create G-Buffer Normal target: {}", e.what());
			}
		}

		// GBuffer1: Albedo (RGB) + Roughness (A)
		{
			RenderTargetConfig config;
			config.width = width;
			config.height = height;
			config.format = GBUFFER_ALBEDO_FORMAT;
			config.samples = vk::SampleCountFlagBits::e1;
			config.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
			config.aspectMask = vk::ImageAspectFlagBits::eColor;
			config.transient = false;

			try
			{
				_gBufferAlbedoTarget = std::make_unique<RenderTarget>(config);
				Logger::LogInfo("Created G-Buffer Albedo target: {}x{}", width, height);
			}
			catch (const std::exception& e)
			{
				Logger::LogError("Failed to create G-Buffer Albedo target: {}", e.what());
			}
		}

		// GBuffer2: Metallic (R) + flags (GBA)
		{
			RenderTargetConfig config;
			config.width = width;
			config.height = height;
			config.format = GBUFFER_METALLIC_FORMAT;
			config.samples = vk::SampleCountFlagBits::e1;
			config.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
			config.aspectMask = vk::ImageAspectFlagBits::eColor;
			config.transient = false;

			try
			{
				_gBufferMetallicTarget = std::make_unique<RenderTarget>(config);
				Logger::LogInfo("Created G-Buffer Metallic target: {}x{}", width, height);
			}
			catch (const std::exception& e)
			{
				Logger::LogError("Failed to create G-Buffer Metallic target: {}", e.what());
			}
		}
	}
} // namespace Ailurus
