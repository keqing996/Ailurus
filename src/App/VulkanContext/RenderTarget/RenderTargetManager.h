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

		// G-Buffer accessors for deferred rendering
		vk::ImageView GetGBufferPositionView() const;
		vk::ImageView GetGBufferNormalView() const;
		vk::ImageView GetGBufferAlbedoView() const;
		vk::ImageView GetGBufferMaterialView() const;
		vk::ImageView GetGBufferDepthView() const;

	private:
        void Clear();
		void CreateDepthTarget(uint32_t width, uint32_t height);
		void CreateMSAATargets(uint32_t width, uint32_t height, vk::Format colorFormat);
		void CreateGBufferTargets(uint32_t width, uint32_t height);

	private:
		// Standard depth buffer (for non-MSAA or as resolve target)
		std::unique_ptr<RenderTarget> _depthTarget = nullptr;

		// MSAA attachments (only created when MSAA is enabled)
		std::unique_ptr<RenderTarget> _msaaColorTarget = nullptr;
		std::unique_ptr<RenderTarget> _msaaDepthTarget = nullptr;

		// G-Buffer targets for deferred rendering
		std::unique_ptr<RenderTarget> _gBufferPosition = nullptr;  // World space position (RGB: xyz, A: unused)
		std::unique_ptr<RenderTarget> _gBufferNormal = nullptr;    // World space normal (RGB: xyz, A: unused)
		std::unique_ptr<RenderTarget> _gBufferAlbedo = nullptr;    // Base color (RGB: color, A: unused)
		std::unique_ptr<RenderTarget> _gBufferMaterial = nullptr;  // Material properties (R: metallic, G: roughness, B: ao, A: unused)
		std::unique_ptr<RenderTarget> _gBufferDepth = nullptr;     // Depth buffer for G-Buffer pass
	};
} // namespace Ailurus
