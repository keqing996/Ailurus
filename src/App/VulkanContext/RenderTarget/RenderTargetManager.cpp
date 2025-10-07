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

		// Create G-Buffer targets for deferred rendering
		CreateGBufferTargets(width, height);
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

	vk::ImageView RenderTargetManager::GetGBufferPositionView() const
	{
		return _gBufferPosition ? _gBufferPosition->GetImageView() : nullptr;
	}

	vk::ImageView RenderTargetManager::GetGBufferNormalView() const
	{
		return _gBufferNormal ? _gBufferNormal->GetImageView() : nullptr;
	}

	vk::ImageView RenderTargetManager::GetGBufferAlbedoView() const
	{
		return _gBufferAlbedo ? _gBufferAlbedo->GetImageView() : nullptr;
	}

	vk::ImageView RenderTargetManager::GetGBufferMaterialView() const
	{
		return _gBufferMaterial ? _gBufferMaterial->GetImageView() : nullptr;
	}

	vk::ImageView RenderTargetManager::GetGBufferDepthView() const
	{
		return _gBufferDepth ? _gBufferDepth->GetImageView() : nullptr;
	}

    void RenderTargetManager::Clear()
    {
        _depthTarget = nullptr;
		_msaaColorTarget = nullptr;
		_msaaDepthTarget = nullptr;
		_gBufferPosition = nullptr;
		_gBufferNormal = nullptr;
		_gBufferAlbedo = nullptr;
		_gBufferMaterial = nullptr;
		_gBufferDepth = nullptr;
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

	void RenderTargetManager::CreateGBufferTargets(uint32_t width, uint32_t height)
	{
		// G-Buffer Position (world space position)
		{
			RenderTargetConfig config;
			config.width = width;
			config.height = height;
			config.format = vk::Format::eR16G16B16A16Sfloat; // High precision for position
			config.samples = vk::SampleCountFlagBits::e1;
			config.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
			config.aspectMask = vk::ImageAspectFlagBits::eColor;
			config.transient = false;

			try
			{
				_gBufferPosition = std::make_unique<RenderTarget>(config);
				Logger::LogInfo("Created G-Buffer Position target: {}x{}", width, height);
			}
			catch (const std::exception& e)
			{
				Logger::LogError("Failed to create G-Buffer Position target: {}", e.what());
			}
		}

		// G-Buffer Normal (world space normal)
		{
			RenderTargetConfig config;
			config.width = width;
			config.height = height;
			config.format = vk::Format::eR16G16B16A16Sfloat; // High precision for normals
			config.samples = vk::SampleCountFlagBits::e1;
			config.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
			config.aspectMask = vk::ImageAspectFlagBits::eColor;
			config.transient = false;

			try
			{
				_gBufferNormal = std::make_unique<RenderTarget>(config);
				Logger::LogInfo("Created G-Buffer Normal target: {}x{}", width, height);
			}
			catch (const std::exception& e)
			{
				Logger::LogError("Failed to create G-Buffer Normal target: {}", e.what());
			}
		}

		// G-Buffer Albedo (base color)
		{
			RenderTargetConfig config;
			config.width = width;
			config.height = height;
			config.format = vk::Format::eR8G8B8A8Unorm; // Standard color format
			config.samples = vk::SampleCountFlagBits::e1;
			config.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
			config.aspectMask = vk::ImageAspectFlagBits::eColor;
			config.transient = false;

			try
			{
				_gBufferAlbedo = std::make_unique<RenderTarget>(config);
				Logger::LogInfo("Created G-Buffer Albedo target: {}x{}", width, height);
			}
			catch (const std::exception& e)
			{
				Logger::LogError("Failed to create G-Buffer Albedo target: {}", e.what());
			}
		}

		// G-Buffer Material Properties (metallic, roughness, ao, etc.)
		{
			RenderTargetConfig config;
			config.width = width;
			config.height = height;
			config.format = vk::Format::eR8G8B8A8Unorm; // Standard format for material properties
			config.samples = vk::SampleCountFlagBits::e1;
			config.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
			config.aspectMask = vk::ImageAspectFlagBits::eColor;
			config.transient = false;

			try
			{
				_gBufferMaterial = std::make_unique<RenderTarget>(config);
				Logger::LogInfo("Created G-Buffer Material target: {}x{}", width, height);
			}
			catch (const std::exception& e)
			{
				Logger::LogError("Failed to create G-Buffer Material target: {}", e.what());
			}
		}

		// G-Buffer Depth
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
				_gBufferDepth = std::make_unique<RenderTarget>(config);
				Logger::LogInfo("Created G-Buffer Depth target: {}x{}", width, height);
			}
			catch (const std::exception& e)
			{
				Logger::LogError("Failed to create G-Buffer Depth target: {}", e.what());
			}
		}
	}
} // namespace Ailurus
