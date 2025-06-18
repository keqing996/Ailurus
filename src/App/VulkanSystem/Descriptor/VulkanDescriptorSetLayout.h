#pragma once

#include <unordered_map>
#include <vulkan/vulkan.hpp>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>

namespace Ailurus
{
	class VulkanDescriptorSetLayout: public NonCopyable, public NonMovable
	{
	public:
		explicit VulkanDescriptorSetLayout(class UniformSet* pUniformSet);
		~VulkanDescriptorSetLayout();

	public:
		vk::DescriptorSetLayout GetDescriptorSetLayout() const;
		const std::unordered_map<vk::DescriptorType, uint32_t>& GetRequirement() const;

	private:
		vk::DescriptorSetLayout _descriptorSetLayout;
		std::unordered_map<vk::DescriptorType, uint32_t> _requirement;
	};
}