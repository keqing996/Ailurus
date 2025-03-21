#include "RHIPipeline.h"
#include "Ailurus/Utility/Logger.h"
#include "Vulkan/RenderPass/RHIRenderPass.h"
#include "Vulkan/Context/VulkanContext.h"
#include "Vulkan/InputAssemble/InputAssemble.h"
#include "Vulkan/Shader/Shader.h"

namespace Ailurus
{
    Pipeline::Pipeline(const RenderPass* pRenderPass, const PipelineConfig& config)
    {
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
        pipelineLayoutInfo.setSetLayouts(nullptr);

        _vkPipelineLayout = VulkanContext::GetDevice().createPipelineLayout(pipelineLayoutInfo);

        // Shader stages
        std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
        for (const Shader* pShader: config.shaderStages)
        {
            if (pShader == nullptr)
                continue;

            shaderStages.push_back(pShader->GeneratePipelineCreateInfo());
        }

        // Vertex input description
        vk::VertexInputBindingDescription vertexInputDesc;
        vertexInputDesc.setBinding(0)
            .setStride(config.pInputAssemble->GetInputAttribute().GetStride())
            .setInputRate(vk::VertexInputRate::eVertex);

        // Vertex input attribute description
        std::vector<vk::VertexInputAttributeDescription> vertexAttrDesc =
            config.pInputAssemble->GetInputAttribute().GetAttributeDescription();

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
            .setBlendConstants(std::array{0.0f, 0.0f, 0.0f, 0.0f});

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

        auto pipelineCreateResult = VulkanContext::GetDevice().createGraphicsPipeline(nullptr, pipelineInfo);
        if (pipelineCreateResult.result == vk::Result::eSuccess)
            _vkPipeline = pipelineCreateResult.value;
        else
            Logger::LogError("Failed to create graphics pipeline");
    }

    Pipeline::~Pipeline()
    {
        VulkanContext::GetDevice().destroyPipelineLayout(_vkPipelineLayout);
        VulkanContext::GetDevice().destroyPipeline(_vkPipeline);
    }

    vk::Pipeline Pipeline::GetPipeline() const
    {
        return _vkPipeline;
    }
}
