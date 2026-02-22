#include "VulkanPipeline.h"
#include "Ailurus/Utility/Logger.h"
#include "Ailurus/Application/Application.h"
#include "Ailurus/Application/RenderSystem/Shader/Shader.h"
#include "Ailurus/Application/RenderSystem/Uniform/UniformSet.h"
#include "VulkanContext/VulkanContext.h"
#include "VulkanContext/Shader/VulkanShader.h"
#include "VulkanContext/Vertex/VulkanVertexLayout.h"
#include "VulkanContext/Descriptor/VulkanDescriptorSetLayout.h"

namespace Ailurus
{
	VulkanPipeline::VulkanPipeline(
		vk::Format colorFormat,
		vk::Format depthFormat,
		const StageShaderArray& shaderArray,
		const VulkanVertexLayout* pVertexLayout,
		const std::vector<const UniformSet*>& uniformSets,
		uint32_t pushConstantSize)
	{
		const bool depthOnly = (colorFormat == vk::Format::eUndefined);
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
			.setSize(pushConstantSize);

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
		vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
		if (pVertexLayout != nullptr)
		{
			vk::VertexInputBindingDescription vertexInputDesc;
			vertexInputDesc.setBinding(0)
				.setStride(pVertexLayout->GetStride())
				.setInputRate(vk::VertexInputRate::eVertex);

			vertexInputInfo.setVertexBindingDescriptions(vertexInputDesc)
				.setVertexAttributeDescriptions(pVertexLayout->GetVulkanAttributeDescription());
		}
		// else: empty vertex input (valid for full-screen triangle using gl_VertexIndex)

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
			.setRasterizationSamples(depthOnly ? vk::SampleCountFlagBits::e1 : VulkanContext::GetMSAASamples());

		// Color blend (only for passes with color output)
		vk::PipelineColorBlendAttachmentState colorBlendAttachment;
		colorBlendAttachment.setBlendEnable(false)
			.setColorWriteMask(vk::ColorComponentFlagBits::eR
				| vk::ColorComponentFlagBits::eG
				| vk::ColorComponentFlagBits::eB
				| vk::ColorComponentFlagBits::eA);

		vk::PipelineColorBlendStateCreateInfo colorBlending;
		colorBlending.setLogicOpEnable(false)
			.setLogicOp(vk::LogicOp::eCopy)
			.setBlendConstants(std::array{ 0.0f, 0.0f, 0.0f, 0.0f });

		if (!depthOnly)
			colorBlending.setAttachments(colorBlendAttachment);

		// Depth and stencil state
		vk::PipelineDepthStencilStateCreateInfo depthStencil;
		depthStencil.setDepthTestEnable(true)
			.setDepthWriteEnable(true)
			.setDepthCompareOp(vk::CompareOp::eLess)
			.setDepthBoundsTestEnable(false)
			.setStencilTestEnable(false);

		// Dynamic state
		std::array dynamicStates = {
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor
		};

		vk::PipelineDynamicStateCreateInfo dynamicState;
		dynamicState.setDynamicStates(dynamicStates);

		// Pipeline rendering create info for dynamic rendering
		vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo;
		pipelineRenderingCreateInfo.setDepthAttachmentFormat(depthFormat)
			.setStencilAttachmentFormat(vk::Format::eUndefined);

		if (!depthOnly)
			pipelineRenderingCreateInfo.setColorAttachmentFormats(colorFormat);

		// Create the pipeline
		vk::GraphicsPipelineCreateInfo pipelineInfo;
		pipelineInfo.setStages(shaderStages)
			.setPVertexInputState(&vertexInputInfo)
			.setPInputAssemblyState(&inputAssembly)
			.setPViewportState(&viewportState)
			.setPRasterizationState(&rasterizer)
			.setPMultisampleState(&multisampling)
			.setPColorBlendState(&colorBlending)
			.setPDepthStencilState(&depthStencil)
			.setPDynamicState(&dynamicState)
			.setLayout(_vkPipelineLayout)
			.setPNext(&pipelineRenderingCreateInfo)
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

	VulkanPipeline::VulkanPipeline(
		vk::Format colorFormat,
		const StageShaderArray& shaderArray,
		const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts,
		uint32_t pushConstantSize,
		bool blendEnabled)
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

		// Push constant range (fragment stage for post-process)
		vk::PushConstantRange pushConstantRange;
		pushConstantRange.setStageFlags(vk::ShaderStageFlagBits::eFragment)
			.setOffset(0)
			.setSize(pushConstantSize);

		// Create pipeline layout
		vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
		pipelineLayoutInfo.setSetLayouts(descriptorSetLayouts);
		if (pushConstantSize > 0)
			pipelineLayoutInfo.setPushConstantRanges(pushConstantRange);

		try
		{
			_vkPipelineLayout = VulkanContext::GetDevice().createPipelineLayout(pipelineLayoutInfo);
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to create post-process pipeline layout: {}", e.what());
		}

		// Empty vertex input (full-screen triangle uses gl_VertexIndex)
		vk::PipelineVertexInputStateCreateInfo vertexInputInfo;

		// Input assembly
		vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
		inputAssembly.setTopology(vk::PrimitiveTopology::eTriangleList)
			.setPrimitiveRestartEnable(false);

		// Viewport & Scissor (dynamic)
		vk::PipelineViewportStateCreateInfo viewportState;
		viewportState.setViewportCount(1)
			.setScissorCount(1);

		// Rasterization: no cull for full-screen triangle
		vk::PipelineRasterizationStateCreateInfo rasterizer;
		rasterizer.setDepthClampEnable(false)
			.setDepthBiasEnable(false)
			.setRasterizerDiscardEnable(false)
			.setPolygonMode(vk::PolygonMode::eFill)
			.setLineWidth(1.0f)
			.setCullMode(vk::CullModeFlagBits::eNone)
			.setFrontFace(vk::FrontFace::eClockwise);

		// Multisample: single sample for post-process
		vk::PipelineMultisampleStateCreateInfo multisampling;
		multisampling.setSampleShadingEnable(false)
			.setAlphaToOneEnable(false)
			.setPSampleMask(nullptr)
			.setAlphaToCoverageEnable(false)
			.setRasterizationSamples(vk::SampleCountFlagBits::e1);

		// Color blend
		vk::PipelineColorBlendAttachmentState colorBlendAttachment;
		colorBlendAttachment.setColorWriteMask(vk::ColorComponentFlagBits::eR
			| vk::ColorComponentFlagBits::eG
			| vk::ColorComponentFlagBits::eB
			| vk::ColorComponentFlagBits::eA);

		if (blendEnabled)
		{
			colorBlendAttachment.setBlendEnable(true)
				.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
				.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
				.setColorBlendOp(vk::BlendOp::eAdd)
				.setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
				.setDstAlphaBlendFactor(vk::BlendFactor::eZero)
				.setAlphaBlendOp(vk::BlendOp::eAdd);
		}
		else
		{
			colorBlendAttachment.setBlendEnable(false);
		}

		vk::PipelineColorBlendStateCreateInfo colorBlending;
		colorBlending.setLogicOpEnable(false)
			.setLogicOp(vk::LogicOp::eCopy)
			.setAttachments(colorBlendAttachment)
			.setBlendConstants(std::array{ 0.0f, 0.0f, 0.0f, 0.0f });

		// Depth stencil: no depth test for post-process
		vk::PipelineDepthStencilStateCreateInfo depthStencil;
		depthStencil.setDepthTestEnable(false)
			.setDepthWriteEnable(false)
			.setDepthCompareOp(vk::CompareOp::eAlways)
			.setDepthBoundsTestEnable(false)
			.setStencilTestEnable(false);

		// Dynamic state
		std::array dynamicStates = {
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor
		};

		vk::PipelineDynamicStateCreateInfo dynamicState;
		dynamicState.setDynamicStates(dynamicStates);

		// Pipeline rendering create info (no depth attachment)
		vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo;
		pipelineRenderingCreateInfo.setColorAttachmentFormats(colorFormat)
			.setDepthAttachmentFormat(vk::Format::eUndefined)
			.setStencilAttachmentFormat(vk::Format::eUndefined);

		// Create the pipeline
		vk::GraphicsPipelineCreateInfo pipelineInfo;
		pipelineInfo.setStages(shaderStages)
			.setPVertexInputState(&vertexInputInfo)
			.setPInputAssemblyState(&inputAssembly)
			.setPViewportState(&viewportState)
			.setPRasterizationState(&rasterizer)
			.setPMultisampleState(&multisampling)
			.setPColorBlendState(&colorBlending)
			.setPDepthStencilState(&depthStencil)
			.setPDynamicState(&dynamicState)
			.setLayout(_vkPipelineLayout)
			.setPNext(&pipelineRenderingCreateInfo)
			.setSubpass(0)
			.setBasePipelineHandle(nullptr);

		try
		{
			auto pipelineCreateResult = VulkanContext::GetDevice().createGraphicsPipeline(nullptr, pipelineInfo);
			if (pipelineCreateResult.result == vk::Result::eSuccess)
				_vkPipeline = pipelineCreateResult.value;
			else
				Logger::LogError("Failed to create post-process graphics pipeline");
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to create post-process graphics pipeline: {}", e.what());
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
