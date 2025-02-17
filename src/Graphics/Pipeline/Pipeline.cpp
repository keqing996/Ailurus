#include "Ailurus/Graphics/Pipeline/Pipeline.h"
#include "Ailurus/Utility/Logger.h"
#include "Ailurus/Graphics/Renderer.h"

namespace Ailurus
{
    Pipeline::Pipeline(const Renderer* pRenderer, const RenderPass* pRenderPass)
        : _pRenderer(pRenderer)
        , _pRenderPass(pRenderPass)
    {
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
        pipelineLayoutInfo.setSetLayouts(nullptr);

        _vkPipelineLayout = _pRenderer->GetLogicalDevice().createPipelineLayout(pipelineLayoutInfo);
    }

    Pipeline::~Pipeline()
    {
        _pRenderer->GetLogicalDevice().destroyPipelineLayout(_vkPipelineLayout);
        _pRenderer->GetLogicalDevice().destroyPipeline(_vkPipeline);
    }

    void Pipeline::AddShader(Shader* pShader)
    {
        _shaderMap[pShader->GetStage()] = pShader;
    }

    void Pipeline::GeneratePipeline()
    {
        auto vkLogicDevice = _pRenderer->GetLogicalDevice();

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
            .setLayout(_vkPipelineLayout)
            .setRenderPass(_pRenderPass->GetRenderPass())
            .setSubpass(0)
            .setBasePipelineHandle(nullptr);

        auto pipelineCreateResult = vkLogicDevice.createGraphicsPipeline(nullptr, pipelineInfo);
        if (pipelineCreateResult.result == vk::Result::eSuccess)
            _vkPipeline = pipelineCreateResult.value;
        else
            Logger::LogError("Failed to create graphics pipeline");
    }

    vk::Pipeline Pipeline::GetPipeline() const
    {
        return _vkPipeline;
    }
}
