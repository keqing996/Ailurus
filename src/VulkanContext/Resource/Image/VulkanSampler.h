#pragma once

#include "VulkanContext/VulkanPch.h"
#include "VulkanContext/Resource/VulkanResource.h"

namespace Ailurus
{
	// Configuration for creating VulkanSampler with flexible parameters
	struct VulkanSamplerCreateConfig
	{
		vk::Filter magFilter = vk::Filter::eLinear;
		vk::Filter minFilter = vk::Filter::eLinear;
		vk::SamplerMipmapMode mipmapMode = vk::SamplerMipmapMode::eLinear;
		vk::SamplerAddressMode addressModeU = vk::SamplerAddressMode::eRepeat;
		vk::SamplerAddressMode addressModeV = vk::SamplerAddressMode::eRepeat;
		vk::SamplerAddressMode addressModeW = vk::SamplerAddressMode::eRepeat;
		float maxAnisotropy = 16.0f;
		bool anisotropyEnable = true;
		float minLod = 0.0f;
		float maxLod = 0.0f;
		vk::BorderColor borderColor = vk::BorderColor::eIntOpaqueBlack;
		bool compareEnable = false;
		vk::CompareOp compareOp = vk::CompareOp::eAlways;
	};

	class VulkanSampler : public VulkanResource
	{
		friend class VulkanResourceManager;

	public:
		explicit VulkanSampler(vk::Sampler sampler);

	public:
		auto GetSampler() const -> vk::Sampler { return _sampler; }

	private:
		vk::Sampler _sampler;

	private:
		friend class VulkanResourceManager;
		static VulkanResourcePtr Create();
		static VulkanResourcePtr CreateFromConfig(const VulkanSamplerCreateConfig& config);
	};
} // namespace Ailurus
