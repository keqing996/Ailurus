#include "RhiShaderProgram.h"
#include "Ailurus/Utility/Logger.h"
#include "Rhi/RhiContext.h"

namespace Ailurus
{
	static vk::DescriptorType UniformBindingTypeToDescriptorType(UniformBindingType type)
	{
		switch (type)
		{
			case UniformBindingType::Uniform:
				return vk::DescriptorType::eUniformBuffer;
		}

		Logger::LogError("Unknow uniform binding type: {}", EnumReflection<UniformBindingType>::ToString(type));
		return vk::DescriptorType::eSampler;
	}

	static vk::ShaderStageFlags GenerateShaderStageFlags(const std::vector<ShaderStage>& stages)
	{
		vk::ShaderStageFlags result;
		for (const auto stage : stages)
		{
			switch (stage)
			{
				case ShaderStage::Vertex:
					result |= vk::ShaderStageFlagBits::eVertex;
					break;
				case ShaderStage::Fragment:
					result |= vk::ShaderStageFlagBits::eFragment;
					break;
				default:
					Logger::LogError("Can not convert {} to vk::ShaderStageFlags",
						EnumReflection<ShaderStage>::ToString(stage));
					break;
			}
		}

		return result;
	}

	RhiShaderProgram::RhiShaderProgram()
	{
	}

	RhiShaderProgram::~RhiShaderProgram()
	{
		DestroySetLayout();
	}

	void RhiShaderProgram::SetUniformLayout(const std::vector<UniformLayout>& uniformLayoutVec)
	{
		DestroySetLayout();

		std::vector<vk::DescriptorSetLayoutBinding> layoutBinds(uniformLayoutVec.size());
		for (auto i = 0; i < uniformLayoutVec.size(); i++)
		{
			const UniformLayout& bindingConfig = uniformLayoutVec[i];
			vk::DescriptorSetLayoutBinding& binding = layoutBinds[i];

			const auto descriptorType = UniformBindingTypeToDescriptorType(bindingConfig.bindingType);
			const auto shaderStages = GenerateShaderStageFlags(bindingConfig.usedStage);

			binding.setBinding(bindingConfig.bindingPoint)
				.setDescriptorType(descriptorType)
				.setDescriptorCount(bindingConfig.arraySize)
				.setStageFlags(shaderStages);
		}

		vk::DescriptorSetLayoutCreateInfo layoutCreateInfo;
		layoutCreateInfo.setBindings(layoutBinds);

		_descriptorSetLayout = RhiContext::GetDevice().createDescriptorSetLayout(layoutCreateInfo);
	}

	vk::DescriptorSetLayout RhiShaderProgram::GetDescriptorSetLayout() const
	{
		return _descriptorSetLayout;
	}

	void RhiShaderProgram::DestroySetLayout()
	{
		if (_descriptorSetLayout != nullptr)
		{
			RhiContext::GetDevice().destroyDescriptorSetLayout(_descriptorSetLayout);
			_descriptorSetLayout = nullptr;
		}
	}
} // namespace Ailurus