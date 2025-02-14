#include "Ailurus/Graphics/Pipeline/Pipeline.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
    Pipeline::Pipeline(const VulkanContext* pContext, const RenderPass* pRenderPass)
        : _pContext(pContext)
        , _pRenderPass(pRenderPass)
    {
    }

    Pipeline::~Pipeline()
    {
        _pContext->GetLogicalDevice().destroyPipelineLayout(_vkPipelineLayout);
        _pContext->GetLogicalDevice().destroyPipeline(_vkPipeline);
    }

    void Pipeline::AddShader(ShaderStage stage, Shader* pShader)
    {
        _shaderMap[stage] = pShader;
    }

    void Pipeline::GeneratePipeline()
    {
        auto vkLogicDevice = _pContext->GetLogicalDevice();

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
        vertexInputInfo.setVertexBindingDescriptionCount(0)
            .setVertexAttributeDescriptionCount(0);

        vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
        inputAssembly.setTopology(vk::PrimitiveTopology::eTriangleList)
            .setPrimitiveRestartEnable(false);

        vk::PipelineViewportStateCreateInfo viewportState;
        viewportState.setViewportCount(1)
            .setScissorCount(1);

        vk::PipelineRasterizationStateCreateInfo rasterizer;
        rasterizer.setDepthClampEnable(false)
            .setRasterizerDiscardEnable(false)
            .setPolygonMode(vk::PolygonMode::eFill)
            .setLineWidth(1.0f)
            .setCullMode(vk::CullModeFlagBits::eBack)
            .setFrontFace(vk::FrontFace::eClockwise)
            .setDepthBiasEnable(false);

        vk::PipelineMultisampleStateCreateInfo multisampling;
        multisampling.setSampleShadingEnable(false)
            .setRasterizationSamples(vk::SampleCountFlagBits::e1);

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

        std::array dynamicStates = {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor
        };

        vk::PipelineDynamicStateCreateInfo dynamicState;
        dynamicState.setDynamicStates(dynamicStates);

        vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
        pipelineLayoutInfo.setSetLayouts(nullptr);

        vk::PipelineLayout pipelineLayout = vkLogicDevice.createPipelineLayout(pipelineLayoutInfo);

        std::vector<vk::PipelineShaderStageCreateInfo> shaderStageCreateInfoList;
        for (auto [stage, pShader]: _shaderMap)
        {
            if (pShader != nullptr)
                shaderStageCreateInfoList.push_back(pShader->GeneratePipelineCreateInfo());
        }

        vk::GraphicsPipelineCreateInfo pipelineInfo;
        pipelineInfo.setStages(shaderStageCreateInfoList)
            .setPVertexInputState(&vertexInputInfo)
            .setPInputAssemblyState(&inputAssembly)
            .setPViewportState(&viewportState)
            .setPRasterizationState(&rasterizer)
            .setPMultisampleState(&multisampling)
            .setPColorBlendState(&colorBlending)
            .setPDynamicState(&dynamicState)
            .setLayout(pipelineLayout)
            .setRenderPass(_pRenderPass->GetRenderPass())
            .setSubpass(0)
            .setBasePipelineHandle(nullptr);

        auto pipelineCreateResult = vkLogicDevice.createGraphicsPipeline(nullptr, pipelineInfo);
        if (pipelineCreateResult.result == vk::Result::eSuccess)
            _vkPipeline = pipelineCreateResult.value;
        else
            Logger::LogError("Failed to create graphics pipeline");
        }
}
