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

		vk::ImageView GetDepthImageView() const;
		vk::ImageView GetMSAAColorImageView() const;

		vk::ImageView GetMSAADepthImageView() const;

		// Offscreen HDR color target
		vk::Image GetOffscreenColorImage() const;
		vk::ImageView GetOffscreenColorImageView() const;

		// CSM shadow maps
		uint32_t GetShadowMapCascadeCount() const;
		vk::ImageView GetShadowMapImageView(uint32_t cascadeIndex) const;
		vk::Image GetShadowMapImage(uint32_t cascadeIndex) const;

	private:
        void Clear();
		void CreateDepthTarget(uint32_t width, uint32_t height);
		void CreateMSAATargets(uint32_t width, uint32_t height);
		void CreateOffscreenColorTarget(uint32_t width, uint32_t height);
		void CreateShadowMapTargets();

	private:
		// Standard depth buffer (for non-MSAA or as resolve target)
		std::unique_ptr<RenderTarget> _depthTarget = nullptr;

		// MSAA attachments (only created when MSAA is enabled)
		// Both MSAA color and resolve target use R16G16B16A16_SFLOAT to match offscreen HDR RT
		std::unique_ptr<RenderTarget> _msaaColorTarget = nullptr;
		std::unique_ptr<RenderTarget> _msaaDepthTarget = nullptr;

		// Offscreen HDR color render target (R16G16B16A16_SFLOAT)
		// Forward pass renders here; post-process chain reads from here
		static constexpr vk::Format OFFSCREEN_COLOR_FORMAT = vk::Format::eR16G16B16A16Sfloat;
		std::unique_ptr<RenderTarget> _offscreenColorTarget = nullptr;

		// CSM shadow maps (one per cascade)
		static constexpr uint32_t SHADOW_MAP_CASCADE_COUNT = 4;
		static constexpr uint32_t SHADOW_MAP_RESOLUTION = 2048;
		std::vector<std::unique_ptr<RenderTarget>> _shadowMapTargets;

		// Future: G-Buffer targets for deferred rendering
		// std::vector<std::unique_ptr<RenderTarget>> _gBufferTargets;
	};
} // namespace Ailurus
