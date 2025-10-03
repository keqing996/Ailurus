#pragma once

#include "VulkanContext/VulkanPch.h"
#include "VulkanContext/Resource/VulkanResource.h"

namespace Ailurus
{
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
	};
} // namespace Ailurus
