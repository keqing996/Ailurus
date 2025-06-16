#include "VulkanPipeline.h"
#include "Ailurus/Utility/Logger.h"
#include "Ailurus/Application/Application.h"
#include "Ailurus/Application/RenderSystem/RenderPass/RenderPass.h"
#include "Ailurus/Application/RenderSystem/Shader/Shader.h"
#include "Ailurus/Application/AssetsSystem/Material/Material.h"
#include "VulkanSystem/VulkanSystem.h"
#include "VulkanSystem/RenderPass/VulkanRenderPass.h"
#include "VulkanSystem/Shader/VulkanShader.h"
#include "VulkanSystem/Vertex/VulkanVertexLayout.h"

namespace Ailurus
{
	VulkanPipeline::VulkanPipeline(const RenderPass* pRenderPass, const Material* pMaterial, 
			const VulkanVertexLayout* pVertexLayout)
	{
		// Prepare & Check
		auto renderPass = pRenderPass->GetRenderPassType();
		auto pShaderArray = pMaterial->GetPassShaderArray(renderPass);
		if (pShaderArray == nullptr)

		// Create pipeline layout
		vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
		pipelineLayoutInfo.setSetLayouts(nullptr);

		_vkPipelineLayout = Application::Get<VulkanSystem>()->GetDevice().createPipelineLayout(pipelineLayoutInfo);

		// Shader stages
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		for (auto i = 0; i < StageShaderArray::Size(); i++)
		{
			const Shader* pShader = config.shaderStages[i];
			if (pShader == nullptr)
				continue;

			const auto* pRHIShader = pShader->GetImpl();
			if (pRHIShader == nullptr)
				continue;

			shaderStages.push_back(pRHIShader->GeneratePipelineCreateInfo(pShader->GetStage()));
		}

		// Vertex input description
		vk::VertexInputBindingDescription vertexInputDesc;
		vertexInputDesc.setBinding(0)
			.setStride(config.pMesh->GetInputAttribute().GetStride())
			.setInputRate(vk::VertexInputRate::eVertex);

		// Vertex input attribute description
		std::vector<vk::VertexInputAttributeDescription> vertexAttrDesc =
			GetMeshVulkanAttributeDescription(config.pMesh);

		// Vertex input
		vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
		vertexInputInfo.setVertexBindingDescriptions(vertexInputDesc)
			.setVertexAttributeDescriptions(vertexAttrDesc);

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

		// Create pipeline
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

		auto pipelineCreateResult = Application::Get<VulkanSystem>()->GetDevice().createGraphicsPipeline(nullptr, pipelineInfo);
		if (pipelineCreateResult.result == vk::Result::eSuccess)
			_vkPipeline = pipelineCreateResult.value;
		else
			Logger::LogError("Failed to create graphics pipeline");
	}

	VulkanPipeline::~VulkanPipeline()
	{
		Application::Get<VulkanSystem>()->GetDevice().destroyPipelineLayout(_vkPipelineLayout);
		Application::Get<VulkanSystem>()->GetDevice().destroyPipeline(_vkPipeline);
	}

	vk::Pipeline VulkanPipeline::GetPipeline() const
	{
		return _vkPipeline;
	}
} // namespace Ailurus
