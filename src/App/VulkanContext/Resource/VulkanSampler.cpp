#include "VulkanSampler.h"
#include "VulkanContext/VulkanContext.h"

namespace Ailurus
{
	VulkanSampler::VulkanSampler(vk::Sampler sampler)
		: _sampler(sampler)
	{
	}

	VulkanSampler::~VulkanSampler()
	{
		// Check if VulkanContext is still valid before destroying resources
		if (!VulkanContext::Initialized())
			return;

		auto device = VulkanContext::GetDevice();

		if (_sampler)
		{
			device.destroySampler(_sampler);
			_sampler = nullptr;
		}
	}
} // namespace Ailurus
