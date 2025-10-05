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
		void SetImage(VulkanImage* pImage);
		void SetSampler(VulkanSampler* pSampler);
		void SetBindingId(uint32_t bindingId);

	private:
		VulkanImage* _pImage;
		VulkanSampler* _pSampler;
		uint32_t _bindingId = 0;
	};
} // namespace Ailurus
