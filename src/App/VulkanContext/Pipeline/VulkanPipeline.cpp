#include "VulkanPipeline.h"
#include "Ailurus/Utility/Logger.h"
#include "Ailurus/Application/Application.h"
#include "Ailurus/Application/RenderSystem/Shader/Shader.h"
#include "Ailurus/Application/RenderSystem/Uniform/UniformSet.h"
#include "VulkanContext/VulkanContext.h"
#include "VulkanContext/RenderPass/VulkanRenderPass.h"
#include "VulkanContext/Shader/VulkanShader.h"
#include "VulkanContext/Vertex/VulkanVertexLayout.h"
#include "VulkanContext/Descriptor/VulkanDescriptorSetLayout.h"

namespace Ailurus
{
	VulkanPipeline::VulkanPipeline(
		const VulkanRenderPass* pRenderPass,
		const StageShaderArray& shaderArray,
		const VulkanVertexLayout* pVertexLayout,
		const std::vector<const UniformSet*>& uniformSets)
	{
		// Shader stages
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		for (auto i = 0; i < StageShaderArray::Size(); i++)
		{
			const Shader* pShader = shaderArray[i];
			if (pShader == nullptr)
				continue;

			const auto* pRHIShader = pShader->GetImpl();
			if (pRHIShader == nullptr)
				continue;

			shaderStages.push_back(pRHIShader->GeneratePipelineCreateInfo(pShader->GetStage()));
		}

		// Push constant range
		vk::PushConstantRange pushConstantRange;
		pushConstantRange.setStageFlags(vk::ShaderStageFlagBits::eVertex)
			.setOffset(0)
			.setSize(sizeof(Matrix4x4f));	// One materix for model matrix 

		// Descriptor set layouts
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
		for (const auto* pUniformSet: uniformSets)
			descriptorSetLayouts.push_back(pUniformSet->GetDescriptorSetLayout()->GetDescriptorSetLayout());

		// Create pipeline layout
		vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
		pipelineLayoutInfo.setSetLayouts(descriptorSetLayouts)
			.setPushConstantRanges(pushConstantRange);
		try
		{
			_vkPipelineLayout = VulkanContext::GetDevice().createPipelineLayout(pipelineLayoutInfo);
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to create pipeline layout: {}", e.what());
		}

		// Vertex input description
		vk::VertexInputBindingDescription vertexInputDesc;
		vertexInputDesc.setBinding(0)
			.setStride(pVertexLayout->GetStride())
			.setInputRate(vk::VertexInputRate::eVertex);

		// Vertex input
		vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
		vertexInputInfo.setVertexBindingDescriptions(vertexInputDesc)
			.setVertexAttributeDescriptions(pVertexLayout->GetVulkanAttributeDescription());

		// Input assemble
		vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
		inputAssembly.setTopology(vk::PrimitiveTopology::eTriangleList)
			.setPrimitiveRestartEnable(false);

		// Viewport & Scissor
		vk::PipelineViewportStateCreateInfo viewportState;
		viewportState.setViewportCount(1)
			.setScissorCount(1);

		// Rasterization
		vk::PipelineRasterizationStateCreateInfo rasterizer;
		rasterizer.setDepthClampEnable(false)
			.setDepthBiasEnable(false)
			.setRasterizerDiscardEnable(false)
			.setPolygonMode(vk::PolygonMode::eFill)
			.setLineWidth(1.0f)
			.setCullMode(vk::CullModeFlagBits::eBack)
			.setFrontFace(vk::FrontFace::eClockwise);

		// Multisample
		vk::PipelineMultisampleStateCreateInfo multisampling;
		multisampling.setSampleShadingEnable(false)
			.setAlphaToOneEnable(false)
			.setPSampleMask(nullptr)
			.setAlphaToCoverageEnable(false) // todo may true ?
			.setRasterizationSamples(vk::SampleCountFlagBits::e1);

		// Color blend
		vk::PipelineColorBlendAttachmentState colorBlendAttachment;
		colorBlendAttachment.setBlendEnable(false)
			.setColorWriteMask(vk::ColorComponentFlagBits::eR
				| vk::ColorComponentFlagBits::eG
				| vk::ColorComponentFlagBits::eB
				| vk::ColorComponentFlagBits::eA);

		vk::PipelineColorBlendStateCreateInfo colorBlending;
		colorBlending.setLogicOpEnable(false)
			.setLogicOp(vk::LogicOp::eCopy)
			.setAttachments(colorBlendAttachment)
			.setBlendConstants(std::array{ 0.0f, 0.0f, 0.0f, 0.0f });

		// Dynamic state
		std::array dynamicStates = {
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor
		};

		vk::PipelineDynamicStateCreateInfo dynamicState;
		dynamicState.setDynamicStates(dynamicStates);

		// Create the pipeline
		vk::GraphicsPipelineCreateInfo pipelineInfo;
		pipelineInfo.setStages(shaderStages)
			.setPVertexInputState(&vertexInputInfo)
			.setPInputAssemblyState(&inputAssembly)
			.setPViewportState(&viewportState)
			.setPRasterizationState(&rasterizer)
			.setPMultisampleState(&multisampling)
			.setPColorBlendState(&colorBlending)
			.setPDynamicState(&dynamicState)
			.setLayout(_vkPipelineLayout)
			.setRenderPass(pRenderPass->GetRenderPass())
			.setSubpass(0)
			.setBasePipelineHandle(nullptr);

		try
		{
			auto pipelineCreateResult = VulkanContext::GetDevice().createGraphicsPipeline(nullptr, pipelineInfo);
			if (pipelineCreateResult.result == vk::Result::eSuccess)
				_vkPipeline = pipelineCreateResult.value;
			else
				Logger::LogError("Failed to create graphics pipeline");
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to create graphics pipeline: {}", e.what());
		}
	}

	VulkanPipeline::~VulkanPipeline()
	{
		try
		{
			VulkanContext::GetDevice().destroyPipeline(_vkPipeline);
			VulkanContext::GetDevice().destroyPipelineLayout(_vkPipelineLayout);
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to destroy pipeline: {}", e.what());
		}
	}

	vk::Pipeline VulkanPipeline::GetPipeline() const
	{
		return _vkPipeline;
	}

	vk::PipelineLayout VulkanPipeline::GetPipelineLayout() const
	{
		return _vkPipelineLayout;
	}
} // namespace Ailurus
