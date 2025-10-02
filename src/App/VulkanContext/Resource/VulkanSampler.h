#pragma once

#include "VulkanContext/VulkanPch.h"
#include "VulkanResource.h"

namespace Ailurus
{
	class VulkanSampler : public VulkanResource
	{
		friend class VulkanResourceManager;

	public:
		explicit VulkanSampler(vk::Sampler sampler);
		~VulkanSampler() override;

	public:
		auto GetSampler() const -> vk::Sampler { return _sampler; }

	private:
		vk::Sampler _sampler;
	};
} // namespace Ailurus
