#include "Ailurus/Systems/RenderSystem/PostProcess/Effects/DeferredLightingEffect.h"
#include "Ailurus/Systems/RenderSystem/PostProcess/PostProcessPipelineFactory.h"
#include "Ailurus/Systems/RenderSystem/PostProcess/PostProcessResourcePool.h"
#include "Ailurus/Systems/RenderSystem/Descriptor/SamplerSchema.h"
#include "Ailurus/Systems/RenderSystem/Descriptor/SamplerSetData.h"
#include "Ailurus/Systems/RenderSystem/Shader/Shader.h"
#include "Ailurus/Systems/RenderSystem/Shader/ShaderLibrary.h"
#include "Ailurus/Systems/RenderSystem/Shader/ShaderStage.h"
#include "VulkanContext/Pipeline/VulkanPipeline.h"
#include "VulkanContext/CommandBuffer/VulkanCommandBuffer.h"
#include "VulkanContext/Resource/VulkanResourceManager.h"
#include "VulkanContext/Resource/Image/VulkanSampler.h"
#include "VulkanContext/Descriptor/VulkanDescriptorSetLayout.h"
#include "VulkanContext/VulkanContext.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
    const std::string DeferredLightingEffect::EFFECT_NAME = "DeferredLighting";
    const char* DeferredLightingEffect::FRAG_SHADER_PATH =
        "./Assets/ShaderBin/PostProcess/deferred_lighting.frag.spv";

    DeferredLightingEffect::DeferredLightingEffect() = default;

    DeferredLightingEffect::~DeferredLightingEffect()
    {
        Shutdown();
    }

    const std::string& DeferredLightingEffect::GetName() const
    {
        return EFFECT_NAME;
    }

    void DeferredLightingEffect::Init(PostProcessResourcePool& /*pool*/,
        PostProcessPipelineFactory& /*factory*/,
        uint32_t /*width*/, uint32_t /*height*/, vk::Format /*format*/)
    {
        // This effect is not initialized via the PostProcessChain.
        // Use InitDeferred() instead.
    }

    void DeferredLightingEffect::OnResize(PostProcessResourcePool& /*pool*/,
        PostProcessPipelineFactory& /*factory*/,
        uint32_t /*width*/, uint32_t /*height*/, vk::Format /*format*/)
    {
        // Resize handled externally via RebuildPipeline().
    }

    void DeferredLightingEffect::InitDeferred(ShaderLibrary* pShaderLibrary,
        vk::DescriptorSetLayout globalDescSetLayout,
        vk::Format outputFormat)
    {
        // Set 1: G-Buffer samplers — binding 0-3 (Normal+AO, Albedo+Roughness, Metallic, Depth)
        _gBufferSamplerSchema = std::make_unique<SamplerSchema>(
            std::vector<SamplerBindingDesc>{ {0}, {1}, {2}, {3} });

        // Nearest-filter sampler for reading G-Buffer textures
        _sampler = VulkanContext::GetResourceManager()->CreateSampler();

        BuildPipelineInternal(pShaderLibrary, globalDescSetLayout, outputFormat);
    }

    void DeferredLightingEffect::RebuildPipeline(ShaderLibrary* pShaderLibrary,
        vk::DescriptorSetLayout globalDescSetLayout,
        vk::Format outputFormat)
    {
        BuildPipelineInternal(pShaderLibrary, globalDescSetLayout, outputFormat);
    }

    void DeferredLightingEffect::BuildPipelineInternal(ShaderLibrary* pShaderLibrary,
        vk::DescriptorSetLayout globalDescSetLayout,
        vk::Format outputFormat)
    {
        if (!pShaderLibrary || !_gBufferSamplerSchema)
            return;

        // Load shaders
        Shader* pVertShader = pShaderLibrary->GetShader(ShaderStage::Vertex,
            "./Assets/ShaderBin/PostProcess/fullscreen.vert.spv");
        Shader* pFragShader = pShaderLibrary->GetShader(ShaderStage::Fragment, FRAG_SHADER_PATH);

        if (!pVertShader || !pFragShader)
        {
            Logger::LogError("DeferredLightingEffect: Failed to load deferred lighting shaders");
            return;
        }

        StageShaderArray shaderArray{};
        shaderArray[ShaderStage::Vertex]   = pVertShader;
        shaderArray[ShaderStage::Fragment] = pFragShader;

        // Pipeline descriptor layouts:
        //   set 0 = global uniform (camera, lights, shadow maps, IBL)
        //   set 1 = G-Buffer samplers (Normal+AO, Albedo+Roughness, Metallic, Depth)
        vk::DescriptorSetLayout gBufferLayout =
            _gBufferSamplerSchema->GetDescriptorSetLayout()->GetDescriptorSetLayout();

        std::vector<vk::DescriptorSetLayout> layouts = { globalDescSetLayout, gBufferLayout };

        // Post-process pipeline variant: no vertex input, no depth, push constant = fragment
        _pipeline = std::make_unique<VulkanPipeline>(
            outputFormat,
            shaderArray,
            layouts,
            static_cast<uint32_t>(sizeof(PushConstants)),
            /*blendEnabled=*/false);
    }

    void DeferredLightingEffect::Render(VulkanCommandBuffer* pCmdBuffer,
        vk::ImageView /*inputImageView*/,
        vk::ImageView outputImageView,
        vk::Extent2D extent,
        VulkanDescriptorAllocator* pDescriptorAllocator)
    {
        if (!_pipeline || !_gBufferSamplerSchema || !_sampler)
            return;
        if (!_gBufferNormalAO || !_gBufferAlbedoRoughness || !_gBufferMetallic || !_depthImageView)
            return;
        if (!_globalDescriptorSet)
            return;

        // Render into the offscreen HDR RT (no depth attachment for a fullscreen quad)
        pCmdBuffer->BeginRendering(outputImageView, nullptr, nullptr, extent,
            /*clearColor=*/true, /*useDepth=*/false);

        pCmdBuffer->BindPipeline(_pipeline.get());
        pCmdBuffer->SetViewportAndScissor(extent.width, extent.height);

        // Build G-Buffer sampler descriptor set (set 1) for this frame
        SamplerSetData samplerData;
        samplerData.SetImage(0, _gBufferNormalAO,        _sampler->GetSampler());
        samplerData.SetImage(1, _gBufferAlbedoRoughness, _sampler->GetSampler());
        samplerData.SetImage(2, _gBufferMetallic,        _sampler->GetSampler());
        samplerData.SetImage(3, _depthImageView,         _sampler->GetSampler());
        vk::DescriptorSet gBufferSet =
            samplerData.AllocateAndWrite(pDescriptorAllocator, _gBufferSamplerSchema.get());

        // Bind [set 0 = global uniform, set 1 = G-Buffer samplers]
        std::vector<vk::DescriptorSet> sets = { _globalDescriptorSet, gBufferSet };
        pCmdBuffer->BindDescriptorSet(_pipeline->GetPipelineLayout(), sets);

        // Push constant: inverse view-projection for world position reconstruction
        PushConstants pc{ _inverseViewProj };
        pCmdBuffer->PushConstants(_pipeline.get(),
            vk::ShaderStageFlagBits::eFragment,
            0,
            static_cast<uint32_t>(sizeof(PushConstants)),
            &pc);

        // Draw fullscreen triangle (3 vertices, no vertex buffer)
        pCmdBuffer->DrawNonIndexed(3);

        pCmdBuffer->EndRendering();
    }

    void DeferredLightingEffect::Shutdown()
    {
        _pipeline = nullptr;
        _gBufferSamplerSchema = nullptr;
        _sampler = nullptr;
    }

    void DeferredLightingEffect::SetGBufferViews(vk::ImageView normalAO,
        vk::ImageView albedoRoughness, vk::ImageView metallic)
    {
        _gBufferNormalAO        = normalAO;
        _gBufferAlbedoRoughness = albedoRoughness;
        _gBufferMetallic        = metallic;
    }

    void DeferredLightingEffect::SetDepthImageView(vk::ImageView depth)
    {
        _depthImageView = depth;
    }

    void DeferredLightingEffect::SetGlobalDescriptorSet(vk::DescriptorSet globalSet)
    {
        _globalDescriptorSet = globalSet;
    }

    void DeferredLightingEffect::SetInverseViewProjMatrix(const Matrix4x4f& mat)
    {
        _inverseViewProj = mat;
    }
} // namespace Ailurus
