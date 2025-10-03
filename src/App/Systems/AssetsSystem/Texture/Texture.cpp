#include "Ailurus/Application/AssetsSystem/Texture/Texture.h"
#include "VulkanContext/Resource/Image/VulkanImage.h"
#include "VulkanContext/Resource/Image/VulkanSampler.h"

namespace Ailurus
{
	Texture::Texture(uint64_t assetId)
		: TypedAsset<AssetType::Texture>(assetId)
	{
	}

	Texture::~Texture()
	{
	}

	auto Texture::GetImage() const -> VulkanImage*
	{
		return _pImage.get();
	}

	auto Texture::GetSampler() const -> VulkanSampler*
	{
		return _pSampler.get();
	}

	auto Texture::GetBindingId() const -> uint32_t
	{
		return _bindingId;
	}

	void Texture::SetImage(std::unique_ptr<VulkanImage>&& pImage)
	{
		_pImage = std::move(pImage);
	}

	void Texture::SetSampler(std::unique_ptr<VulkanSampler>&& pSampler)
	{
		_pSampler = std::move(pSampler);
	}

	void Texture::SetBindingId(uint32_t bindingId)
	{
		_bindingId = bindingId;
	}
} // namespace Ailurus
