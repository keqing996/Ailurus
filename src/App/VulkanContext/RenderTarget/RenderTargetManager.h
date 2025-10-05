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

	private:
        void Clear();
		void CreateDepthTarget(uint32_t width, uint32_t height);
		void CreateMSAATargets(uint32_t width, uint32_t height, vk::Format colorFormat);

	private:
		// Standard depth buffer (for non-MSAA or as resolve target)
		std::unique_ptr<RenderTarget> _depthTarget = nullptr;

		// MSAA attachments (only created when MSAA is enabled)
		std::unique_ptr<RenderTarget> _msaaColorTarget = nullptr;
		std::unique_ptr<RenderTarget> _msaaDepthTarget = nullptr;

		// Future: G-Buffer targets for deferred rendering
		// std::vector<std::unique_ptr<RenderTarget>> _gBufferTargets;
	};
} // namespace Ailurus
