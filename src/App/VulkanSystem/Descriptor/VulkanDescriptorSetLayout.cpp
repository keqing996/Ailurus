#include "Ailurus/Application/Application.h"
#include "Ailurus/Application/RenderSystem/Uniform/UniformSet.h"
#include "VulkanSystem/Helper/VulkanHelper.h"
#include "VulkanSystem/VulkanSystem.h"
#include "VulkanDescriptorSetLayout.h"

namespace Ailurus
{

	VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(UniformSet* pUniformSet)
	{
		std::vector<vk::DescriptorSetLayoutBinding> layoutBindings;

		const auto& allBindingPoints = pUniformSet->GetAllBindingPoints();
		for (const auto& [bindingId, pBindingPoint]: allBindingPoints)
		{
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

		vk::DescriptorSetLayoutCreateInfo layoutInfo;
		layoutInfo.setBindings(layoutBindings);

		_descriptorSetLayout = Application::Get<VulkanSystem>()->GetDevice().createDescriptorSetLayout(layoutInfo);
	}

	VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
	{
		Application::Get<VulkanSystem>()->GetDevice().destroyDescriptorSetLayout(_descriptorSetLayout);
	}
} // namespace Ailurus