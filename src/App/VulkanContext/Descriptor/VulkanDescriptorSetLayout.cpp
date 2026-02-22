#include "Ailurus/Application/Application.h"
#include "Ailurus/Application/RenderSystem/Uniform/UniformSet.h"
#include "VulkanContext/Helper/VulkanHelper.h"
#include "VulkanContext/VulkanContext.h"
#include "VulkanDescriptorSetLayout.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
	VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(UniformSet* pUniformSet, const std::vector<TextureBindingInfo>& textureBindings)
	{
		std::vector<vk::DescriptorSetLayoutBinding> layoutBindings;

		// Add uniform buffer bindings
		const auto& allBindingPoints = pUniformSet->GetAllBindingPoints();
		for (const auto& [bindingId, pBindingPoint]: allBindingPoints)
		{
			_requirement[vk::DescriptorType::eUniformBuffer]++;

			vk::ShaderStageFlags usedShaderStage;
			const auto& shaderStages = pBindingPoint->GetUsingStages();
			for (const auto& shaderStage: shaderStages)
				usedShaderStage |= VulkanHelper::GetShaderStage(shaderStage);

			vk::DescriptorSetLayoutBinding bindingInfo;
			bindingInfo.setBinding(bindingId)
				.setStageFlags(usedShaderStage)
				.setDescriptorCount(1)	// Uniform buffers count (always one because we use dynamic offset)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer);

			layoutBindings.push_back(bindingInfo);
		}

		// Add texture bindings (combined image samplers)
		for (const auto& textureBinding : textureBindings)
		{
			_requirement[vk::DescriptorType::eCombinedImageSampler]++;

			vk::DescriptorSetLayoutBinding bindingInfo;
			bindingInfo.setBinding(textureBinding.bindingId)
				.setStageFlags(textureBinding.shaderStages)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);

			layoutBindings.push_back(bindingInfo);
		}

		vk::DescriptorSetLayoutCreateInfo layoutInfo;
		layoutInfo.setBindings(layoutBindings);

		try
		{
			_descriptorSetLayout = VulkanContext::GetDevice().createDescriptorSetLayout(layoutInfo);
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to create descriptor set layout: {}", e.what());
		}
	}

	VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
	{
		try
		{
			VulkanContext::GetDevice().destroyDescriptorSetLayout(_descriptorSetLayout);
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to destroy descriptor set layout: {}", e.what());
		}
	}

	VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(const std::vector<vk::DescriptorSetLayoutBinding>& bindings)
	{
		for (const auto& binding : bindings)
			_requirement[binding.descriptorType]++;

		vk::DescriptorSetLayoutCreateInfo layoutInfo;
		layoutInfo.setBindings(bindings);

		try
		{
			_descriptorSetLayout = VulkanContext::GetDevice().createDescriptorSetLayout(layoutInfo);
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to create descriptor set layout: {}", e.what());
		}
	}

	vk::DescriptorSetLayout VulkanDescriptorSetLayout::GetDescriptorSetLayout() const
	{
		return _descriptorSetLayout;
	}

	const std::unordered_map<vk::DescriptorType, uint32_t>& VulkanDescriptorSetLayout::GetRequirement() const
	{
		return _requirement;
	}
} // namespace Ailurus