#include "VulkanDescriptorSet.h"

namespace Ailurus
{
	VulkanDescriptorSet::VulkanDescriptorSet(vk::DescriptorSet set)
		: _descriptorSet(set)
	{
	}

	VulkanDescriptorSet::~VulkanDescriptorSet()
	{
	}

	vk::DescriptorSet VulkanDescriptorSet::GetDescriptorSet() const
	{
		return _descriptorSet;
	}

} // namespace Ailurus
