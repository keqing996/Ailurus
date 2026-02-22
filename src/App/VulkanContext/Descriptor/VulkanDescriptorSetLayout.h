#pragma once

#include "VulkanContext/VulkanPch.h"
#include <unordered_map>
#include <vector>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>

namespace Ailurus
{
	// Structure to hold texture binding information
	struct TextureBindingInfo
	{
		uint32_t bindingId;
		vk::ShaderStageFlags shaderStages;
	};

	class VulkanDescriptorSetLayout: public NonCopyable, public NonMovable
	{
	public:
		explicit VulkanDescriptorSetLayout(class UniformSet* pUniformSet, const std::vector<TextureBindingInfo>& textureBindings = {});
		explicit VulkanDescriptorSetLayout(const std::vector<vk::DescriptorSetLayoutBinding>& bindings);
		~VulkanDescriptorSetLayout();

	public:
		vk::DescriptorSetLayout GetDescriptorSetLayout() const;
		const std::unordered_map<vk::DescriptorType, uint32_t>& GetRequirement() const;

	private:
		vk::DescriptorSetLayout _descriptorSetLayout;
		std::unordered_map<vk::DescriptorType, uint32_t> _requirement;
	};
}