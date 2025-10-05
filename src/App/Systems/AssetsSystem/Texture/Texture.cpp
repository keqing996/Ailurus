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
		if (_pImage)
			_pImage->MarkDelete();
		if (_pSampler)
			_pSampler->MarkDelete();
	}

	auto Texture::GetImage() const -> VulkanImage*
	{
		return _pImage;
	}

	auto Texture::GetSampler() const -> VulkanSampler*
	{
		return _pSampler;
	}

	auto Texture::GetBindingId() const -> uint32_t
	{
		return _bindingId;
	}

	void Texture::SetImage(VulkanImage* pImage)
	{
		_pImage = pImage;
	}

	void Texture::SetSampler(VulkanSampler* pSampler)
	{
		_pSampler = pSampler;
	}

	void Texture::SetBindingId(uint32_t bindingId)
	{
		_bindingId = bindingId;
	}
} // namespace Ailurus
