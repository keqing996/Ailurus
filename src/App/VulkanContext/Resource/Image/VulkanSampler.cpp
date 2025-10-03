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
} // namespace Ailurus
