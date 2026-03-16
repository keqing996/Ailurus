#pragma once

#include <memory>
#include <vulkan/vulkan.hpp>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>
#include "RenderTarget.h"

namespace Ailurus
{
	class RenderTargetManager : public NonCopyable, public NonMovable
	{
	public:
		RenderTargetManager();
		~RenderTargetManager();

	public:
        void Rebuild();

		vk::Image GetDepthImage() const;
		vk::ImageView GetDepthImageView() const;
		vk::Image GetMSAAColorImage() const;
		vk::ImageView GetMSAAColorImageView() const;

		vk::Image GetMSAADepthImage() const;
		vk::ImageView GetMSAADepthImageView() const;
		vk::Image GetResolvedMSAADepthImage() const;
		vk::ImageView GetResolvedMSAADepthImageView() const;

		// Offscreen HDR color target
		vk::Image GetOffscreenColorImage() const;
		vk::ImageView GetOffscreenColorImageView() const;

		// CSM shadow maps
		uint32_t GetShadowMapCascadeCount() const;
		vk::ImageView GetShadowMapImageView(uint32_t cascadeIndex) const;
		vk::Image GetShadowMapImage(uint32_t cascadeIndex) const;

		// G-Buffer targets (for deferred rendering)
		vk::Image GetGBufferNormalImage() const;
		vk::ImageView GetGBufferNormalImageView() const;
		vk::Image GetGBufferAlbedoImage() const;
		vk::ImageView GetGBufferAlbedoImageView() const;
		vk::Image GetGBufferMetallicImage() const;
		vk::ImageView GetGBufferMetallicImageView() const;

		static constexpr vk::Format GetGBufferNormalFormat()   { return GBUFFER_NORMAL_FORMAT; }
		static constexpr vk::Format GetGBufferAlbedoFormat()   { return GBUFFER_ALBEDO_FORMAT; }
		static constexpr vk::Format GetGBufferMetallicFormat() { return GBUFFER_METALLIC_FORMAT; }

	private:
        void Clear();
		void CreateDepthTarget(uint32_t width, uint32_t height);
		void CreateMSAATargets(uint32_t width, uint32_t height);
		void CreateOffscreenColorTarget(uint32_t width, uint32_t height);
		void CreateShadowMapTargets();
		void CreateGBufferTargets(uint32_t width, uint32_t height);

	private:
		// Standard depth buffer (for non-MSAA or as resolve target)
		std::unique_ptr<RenderTarget> _depthTarget = nullptr;

		// MSAA attachments (only created when MSAA is enabled)
		// Both MSAA color and resolve target use R16G16B16A16_SFLOAT to match offscreen HDR RT
		std::unique_ptr<RenderTarget> _msaaColorTarget = nullptr;
		std::unique_ptr<RenderTarget> _msaaDepthTarget = nullptr;
		std::unique_ptr<RenderTarget> _resolvedMSAADepthTarget = nullptr;

		// Offscreen HDR color render target (R16G16B16A16_SFLOAT)
		// Forward pass renders here; post-process chain reads from here
		static constexpr vk::Format OFFSCREEN_COLOR_FORMAT = vk::Format::eR16G16B16A16Sfloat;
		std::unique_ptr<RenderTarget> _offscreenColorTarget = nullptr;

		// CSM shadow maps (one per cascade)
		static constexpr uint32_t SHADOW_MAP_CASCADE_COUNT = 4;
		static constexpr uint32_t SHADOW_MAP_RESOLUTION = 2048;
		std::vector<std::unique_ptr<RenderTarget>> _shadowMapTargets;

		// G-Buffer targets for deferred rendering
		// GBuffer0: World Normal (XYZ) + AO (W) — R16G16B16A16_SFLOAT
		// GBuffer1: Albedo (RGB) + Roughness (A) — R8G8B8A8_UNORM
		// GBuffer2: Metallic (R) + flags (GBA) — R8G8B8A8_UNORM
		static constexpr vk::Format GBUFFER_NORMAL_FORMAT  = vk::Format::eR16G16B16A16Sfloat;
		static constexpr vk::Format GBUFFER_ALBEDO_FORMAT  = vk::Format::eR8G8B8A8Unorm;
		static constexpr vk::Format GBUFFER_METALLIC_FORMAT = vk::Format::eR8G8B8A8Unorm;
		std::unique_ptr<RenderTarget> _gBufferNormalTarget;
		std::unique_ptr<RenderTarget> _gBufferAlbedoTarget;
		std::unique_ptr<RenderTarget> _gBufferMetallicTarget;
	};
} // namespace Ailurus
