#pragma once

#include <vulkan/vulkan.hpp>

namespace Ailurus
{
    class VulkanDescriptorSet
    {
    public:
		explicit VulkanDescriptorSet(vk::DescriptorSet set);
		~VulkanDescriptorSet();

	public:
		vk::DescriptorSet GetDescriptorSet() const;

	private:
        vk::DescriptorSet _descriptorSet;
    };
} // namespace Ailurus