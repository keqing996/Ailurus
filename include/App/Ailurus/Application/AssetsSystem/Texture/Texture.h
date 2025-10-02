#pragma once

#include <memory>
#include <cstdint>
#include "Ailurus/Application/AssetsSystem/Asset.h"

namespace Ailurus
{
	class VulkanImage;
	class VulkanSampler;

	class Texture : public TypedAsset<AssetType::Texture>
	{
	public:
        Texture(uint64_t assetId);
		~Texture();

	public:
		auto GetImage() const -> VulkanImage*;
		auto GetSampler() const -> VulkanSampler*;
		auto GetBindingId() const -> uint32_t;
		void SetImage(std::unique_ptr<VulkanImage>&& pImage);
		void SetSampler(std::unique_ptr<VulkanSampler>&& pSampler);
		void SetBindingId(uint32_t bindingId);

	private:
		std::unique_ptr<VulkanImage> _pImage;
		std::unique_ptr<VulkanSampler> _pSampler;
		uint32_t _bindingId = 0;
	};
} // namespace Ailurus
