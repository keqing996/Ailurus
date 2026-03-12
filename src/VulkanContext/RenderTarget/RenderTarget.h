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

	private:
		RenderTargetConfig _config;
		vk::Image _image = nullptr;
		vk::ImageView _imageView = nullptr;
		vk::DeviceMemory _memory = nullptr;
	};
} // namespace Ailurus
