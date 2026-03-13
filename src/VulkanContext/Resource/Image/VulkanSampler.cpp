#include "VulkanSampler.h"
#include "VulkanContext/VulkanContext.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
	static void SamplerDeleter(VulkanResource* pResource)
	{
		auto ptr = static_cast<VulkanSampler*>(pResource);
		auto device = VulkanContext::GetDevice();

		auto sampler = ptr->GetSampler();
		if (sampler)
			device.destroySampler(sampler);
	}

	VulkanSampler::VulkanSampler(vk::Sampler sampler)
		: _sampler(sampler)
	{
	}

	VulkanResourcePtr VulkanSampler::Create()
	{
		const auto device = VulkanContext::GetDevice();

		try
		{
			vk::SamplerCreateInfo samplerInfo;
			samplerInfo.setMagFilter(vk::Filter::eLinear)
				.setMinFilter(vk::Filter::eLinear)
				.setMipmapMode(vk::SamplerMipmapMode::eLinear)
				.setAddressModeU(vk::SamplerAddressMode::eRepeat)
				.setAddressModeV(vk::SamplerAddressMode::eRepeat)
				.setAddressModeW(vk::SamplerAddressMode::eRepeat)
				.setMipLodBias(0.0f)
				.setAnisotropyEnable(VK_TRUE)
				.setMaxAnisotropy(16.0f)
				.setCompareEnable(VK_FALSE)
				.setCompareOp(vk::CompareOp::eAlways)
				.setMinLod(0.0f)
				.setMaxLod(0.0f)
				.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
				.setUnnormalizedCoordinates(VK_FALSE);

			vk::Sampler sampler = device.createSampler(samplerInfo);

			VulkanSampler* pSamplerRaw = new VulkanSampler(sampler);
			return VulkanResourcePtr(pSamplerRaw, &SamplerDeleter);
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to create vulkan sampler: {}", e.what());
			return nullptr;
		}
	}

	VulkanResourcePtr VulkanSampler::CreateFromConfig(const VulkanSamplerCreateConfig& config)
	{
		const auto device = VulkanContext::GetDevice();

		try
		{
			vk::SamplerCreateInfo samplerInfo;
			samplerInfo.setMagFilter(config.magFilter)
				.setMinFilter(config.minFilter)
				.setMipmapMode(config.mipmapMode)
				.setAddressModeU(config.addressModeU)
				.setAddressModeV(config.addressModeV)
				.setAddressModeW(config.addressModeW)
				.setMipLodBias(0.0f)
				.setAnisotropyEnable(config.anisotropyEnable ? VK_TRUE : VK_FALSE)
				.setMaxAnisotropy(config.maxAnisotropy)
				.setCompareEnable(config.compareEnable ? VK_TRUE : VK_FALSE)
				.setCompareOp(config.compareOp)
				.setMinLod(config.minLod)
				.setMaxLod(config.maxLod)
				.setBorderColor(config.borderColor)
				.setUnnormalizedCoordinates(VK_FALSE);

			vk::Sampler sampler = device.createSampler(samplerInfo);

			VulkanSampler* pSamplerRaw = new VulkanSampler(sampler);
			return VulkanResourcePtr(pSamplerRaw, &SamplerDeleter);
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to create vulkan sampler from config: {}", e.what());
			return nullptr;
		}
	}
} // namespace Ailurus
