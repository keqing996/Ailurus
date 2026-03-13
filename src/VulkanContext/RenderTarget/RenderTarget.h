#pragma once

#include <vulkan/vulkan.hpp>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>

namespace Ailurus
{
	struct RenderTargetConfig
	{
		uint32_t width;
		uint32_t height;
		vk::Format format;
		vk::SampleCountFlagBits samples;
		vk::ImageUsageFlags usage;
		vk::ImageAspectFlags aspectMask;
		bool transient; // If true, use TRANSIENT_ATTACHMENT flag for MSAA optimization
		uint32_t mipLevels = 1;
		uint32_t arrayLayers = 1;
		vk::ImageCreateFlags flags = {};           // eCubeCompatible for cubemap
		vk::ImageViewType viewType = vk::ImageViewType::e2D;
	};

	/// @brief Encapsulates a single render target (image + view + memory)
	class RenderTarget : public NonCopyable, public NonMovable
	{
	public:
		explicit RenderTarget(const RenderTargetConfig& config);
		~RenderTarget();

	public:
		vk::Image GetImage() const;
		vk::ImageView GetImageView() const;
		const RenderTargetConfig& GetConfig() const;
		bool IsValid() const;

	private:
		void CreateImage();
		void AllocateMemory();
		void CreateImageView();
		uint32_t FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;

	public:
		// Create a 2D ImageView for a specific layer and mip level (for rendering to individual cubemap faces)
		// Caller is responsible for destroying the returned ImageView.
		vk::ImageView CreateSingleLayerMipView(uint32_t arrayLayer, uint32_t mipLevel) const;

	private:
		RenderTargetConfig _config;
		vk::Image _image = nullptr;
		vk::ImageView _imageView = nullptr;
		vk::DeviceMemory _memory = nullptr;
	};
} // namespace Ailurus
