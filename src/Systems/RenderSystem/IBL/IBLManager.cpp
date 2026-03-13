#include "IBLManager.h"
#include "Ailurus/Utility/Logger.h"
#include "Ailurus/Systems/RenderSystem/Shader/ShaderLibrary.h"
#include "Ailurus/Systems/RenderSystem/Shader/ShaderStage.h"
#include <VulkanContext/VulkanContext.h>
#include <VulkanContext/Resource/VulkanResourceManager.h>
#include <VulkanContext/Resource/Image/VulkanSampler.h>
#include <VulkanContext/Pipeline/VulkanPipeline.h>
#include <VulkanContext/RenderTarget/RenderTarget.h>
#include <VulkanContext/Descriptor/VulkanDescriptorWriter.h>
#include <VulkanContext/CommandBuffer/VulkanCommandBuffer.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Ailurus
{
	// Shader paths
	static const char* FULLSCREEN_VERT_PATH = "./Assets/ShaderBin/PostProcess/fullscreen.vert.spv";
	static const char* IRRADIANCE_FRAG_PATH = "./Assets/ShaderBin/ibl_irradiance.frag.spv";
	static const char* PREFILTER_FRAG_PATH = "./Assets/ShaderBin/ibl_prefilter.frag.spv";
	static const char* BRDF_LUT_FRAG_PATH = "./Assets/ShaderBin/ibl_brdf_lut.frag.spv";

	// 6 cubemap face view matrices (from origin looking at each face)
	static glm::mat4 GetCubemapFaceViewMatrix(uint32_t face)
	{
		switch (face)
		{
			case 0: return glm::lookAt(glm::vec3(0), glm::vec3( 1, 0, 0), glm::vec3(0,-1, 0)); // +X
			case 1: return glm::lookAt(glm::vec3(0), glm::vec3(-1, 0, 0), glm::vec3(0,-1, 0)); // -X
			case 2: return glm::lookAt(glm::vec3(0), glm::vec3( 0, 1, 0), glm::vec3(0, 0, 1)); // +Y
			case 3: return glm::lookAt(glm::vec3(0), glm::vec3( 0,-1, 0), glm::vec3(0, 0,-1)); // -Y
			case 4: return glm::lookAt(glm::vec3(0), glm::vec3( 0, 0, 1), glm::vec3(0,-1, 0)); // +Z
			case 5: return glm::lookAt(glm::vec3(0), glm::vec3( 0, 0,-1), glm::vec3(0,-1, 0)); // -Z
			default: return glm::mat4(1.0f);
		}
	}

	IBLManager::IBLManager() = default;
	IBLManager::~IBLManager() = default;

	void IBLManager::Precompute(vk::ImageView envCubemapView, vk::Sampler envSampler,
								ShaderLibrary* pShaderLibrary)
	{
		auto device = VulkanContext::GetDevice();

		// Create descriptor set layout for environment cubemap (binding 0, fragment stage)
		vk::DescriptorSetLayoutBinding envBinding;
		envBinding.setBinding(0)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setDescriptorCount(1)
			.setStageFlags(vk::ShaderStageFlagBits::eFragment);

		vk::DescriptorSetLayoutCreateInfo envLayoutInfo;
		envLayoutInfo.setBindings(envBinding);
		_envMapDescriptorSetLayout = device.createDescriptorSetLayout(envLayoutInfo);

		// Create descriptor pool for environment cubemap
		vk::DescriptorPoolSize envPoolSize;
		envPoolSize.setType(vk::DescriptorType::eCombinedImageSampler)
			.setDescriptorCount(1);

		vk::DescriptorPoolCreateInfo envPoolInfo;
		envPoolInfo.setMaxSets(1)
			.setPoolSizes(envPoolSize);
		_envMapDescriptorPool = device.createDescriptorPool(envPoolInfo);

		// Allocate and write environment cubemap descriptor set
		vk::DescriptorSetAllocateInfo envAllocInfo;
		envAllocInfo.setDescriptorPool(_envMapDescriptorPool)
			.setSetLayouts(_envMapDescriptorSetLayout);
		_envMapDescriptorSet = device.allocateDescriptorSets(envAllocInfo)[0];

		VulkanDescriptorWriter envWriter;
		envWriter.WriteImage(0, envCubemapView, envSampler);
		envWriter.UpdateSet(_envMapDescriptorSet);

		// Create empty descriptor set layout for BRDF LUT pipeline
		vk::DescriptorSetLayoutCreateInfo emptyLayoutInfo;
		_emptyDescriptorSetLayout = device.createDescriptorSetLayout(emptyLayoutInfo);

		// Create empty descriptor pool and allocate empty set
		vk::DescriptorPoolCreateInfo emptyPoolInfo;
		emptyPoolInfo.setMaxSets(1);
		// Need at least one pool size entry even if unused on some drivers
		vk::DescriptorPoolSize dummyPoolSize;
		dummyPoolSize.setType(vk::DescriptorType::eCombinedImageSampler)
			.setDescriptorCount(1);
		emptyPoolInfo.setPoolSizes(dummyPoolSize);
		_emptyDescriptorPool = device.createDescriptorPool(emptyPoolInfo);

		vk::DescriptorSetAllocateInfo emptyAllocInfo;
		emptyAllocInfo.setDescriptorPool(_emptyDescriptorPool)
			.setSetLayouts(_emptyDescriptorSetLayout);
		_emptyDescriptorSet = device.allocateDescriptorSets(emptyAllocInfo)[0];

		// Submit each IBL compute pass in its own command buffer.
		// Using the engine's VulkanCommandBuffer wrapper ensures the same
		// MoltenVK-compatible dynamic rendering code path as post-process effects.
		ComputeIrradianceMap(envCubemapView, envSampler, pShaderLibrary);
		ComputePrefilteredMap(envCubemapView, envSampler, pShaderLibrary);
		ComputeBRDFLUT(pShaderLibrary);

		_ready = true;
		Logger::LogInfo("IBLManager: Precomputation complete");
	}

	void IBLManager::ComputeIrradianceMap(vk::ImageView envCubemapView, vk::Sampler envSampler,
									   ShaderLibrary* pShaderLibrary)
	{
		auto device = VulkanContext::GetDevice();

		// Create irradiance cubemap render target
		RenderTargetConfig config;
		config.width = IRRADIANCE_SIZE;
		config.height = IRRADIANCE_SIZE;
		config.format = vk::Format::eR16G16B16A16Sfloat;
		config.samples = vk::SampleCountFlagBits::e1;
		config.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
		config.aspectMask = vk::ImageAspectFlagBits::eColor;
		config.transient = false;
		config.mipLevels = 1;
		config.arrayLayers = 6;
		config.flags = vk::ImageCreateFlagBits::eCubeCompatible;
		config.viewType = vk::ImageViewType::eCube;

		_irradianceMap = std::make_unique<RenderTarget>(config);

		// Create sampler
		VulkanSamplerCreateConfig samplerConfig;
		samplerConfig.magFilter = vk::Filter::eLinear;
		samplerConfig.minFilter = vk::Filter::eLinear;
		samplerConfig.addressModeU = vk::SamplerAddressMode::eClampToEdge;
		samplerConfig.addressModeV = vk::SamplerAddressMode::eClampToEdge;
		samplerConfig.addressModeW = vk::SamplerAddressMode::eClampToEdge;
		samplerConfig.anisotropyEnable = false;
		_irradianceSampler = VulkanContext::GetResourceManager()->CreateSampler(samplerConfig);

		// Create pipeline
		auto* pVertShader = pShaderLibrary->GetShader(ShaderStage::Vertex, FULLSCREEN_VERT_PATH);
		auto* pFragShader = pShaderLibrary->GetShader(ShaderStage::Fragment, IRRADIANCE_FRAG_PATH);

		StageShaderArray shaderArray{};
		shaderArray[ShaderStage::Vertex] = pVertShader;
		shaderArray[ShaderStage::Fragment] = pFragShader;

		std::vector<vk::DescriptorSetLayout> layouts = { _envMapDescriptorSetLayout };
		_irradiancePipeline = std::make_unique<VulkanPipeline>(
			vk::Format::eR16G16B16A16Sfloat, shaderArray, layouts,
			static_cast<uint32_t>(sizeof(glm::mat4)), false);

		// Record all 6 cubemap faces into one command buffer
		VulkanCommandBuffer cmd(true);
		cmd.Begin();

		// Transition cubemap to color attachment optimal (all 6 layers)
		cmd.ImageMemoryBarrier(_irradianceMap->GetImage(),
			vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal,
			vk::AccessFlags{}, vk::AccessFlagBits::eColorAttachmentWrite,
			vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::ImageAspectFlagBits::eColor, 0, 1, 0, 6);

		glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

		for (uint32_t face = 0; face < 6; face++)
		{
			vk::ImageView faceView = _irradianceMap->CreateSingleLayerMipView(face, 0);
			glm::mat4 faceVP = proj * GetCubemapFaceViewMatrix(face);

			RenderFullscreenPass(&cmd, faceView, IRRADIANCE_SIZE, IRRADIANCE_SIZE,
				_irradiancePipeline.get(), _envMapDescriptorSet,
				&faceVP, sizeof(glm::mat4));

			device.destroyImageView(faceView);
		}

		// Transition to shader read optimal (all 6 layers)
		cmd.ImageMemoryBarrier(_irradianceMap->GetImage(),
			vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
			vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eShaderRead,
			vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eFragmentShader,
			vk::ImageAspectFlagBits::eColor, 0, 1, 0, 6);

		cmd.End();

		vk::SubmitInfo submitInfo;
		submitInfo.setCommandBuffers(cmd.GetBuffer());
		VulkanContext::GetGraphicQueue().submit(submitInfo);
		VulkanContext::GetGraphicQueue().waitIdle();

		Logger::LogInfo("IBLManager: Irradiance map computed ({}x{} per face)", IRRADIANCE_SIZE, IRRADIANCE_SIZE);
	}

	void IBLManager::ComputePrefilteredMap(vk::ImageView envCubemapView, vk::Sampler envSampler,
											ShaderLibrary* pShaderLibrary)
	{
		auto device = VulkanContext::GetDevice();

		// Create pre-filtered cubemap render target with mip chain
		RenderTargetConfig config;
		config.width = PREFILTER_SIZE;
		config.height = PREFILTER_SIZE;
		config.format = vk::Format::eR16G16B16A16Sfloat;
		config.samples = vk::SampleCountFlagBits::e1;
		config.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
		config.aspectMask = vk::ImageAspectFlagBits::eColor;
		config.transient = false;
		config.mipLevels = PREFILTER_MIP_LEVELS;
		config.arrayLayers = 6;
		config.flags = vk::ImageCreateFlagBits::eCubeCompatible;
		config.viewType = vk::ImageViewType::eCube;

		_prefilteredMap = std::make_unique<RenderTarget>(config);

		// Create sampler with mip levels
		VulkanSamplerCreateConfig samplerConfig;
		samplerConfig.magFilter = vk::Filter::eLinear;
		samplerConfig.minFilter = vk::Filter::eLinear;
		samplerConfig.mipmapMode = vk::SamplerMipmapMode::eLinear;
		samplerConfig.addressModeU = vk::SamplerAddressMode::eClampToEdge;
		samplerConfig.addressModeV = vk::SamplerAddressMode::eClampToEdge;
		samplerConfig.addressModeW = vk::SamplerAddressMode::eClampToEdge;
		samplerConfig.anisotropyEnable = false;
		samplerConfig.minLod = 0.0f;
		samplerConfig.maxLod = static_cast<float>(PREFILTER_MIP_LEVELS - 1);
		_prefilteredSampler = VulkanContext::GetResourceManager()->CreateSampler(samplerConfig);

		// Push constant struct for prefilter shader
		struct PrefilterPushConstants
		{
			glm::mat4 faceVP;
			float roughness;
		};

		// Create pipeline
		auto* pVertShader = pShaderLibrary->GetShader(ShaderStage::Vertex, FULLSCREEN_VERT_PATH);
		auto* pFragShader = pShaderLibrary->GetShader(ShaderStage::Fragment, PREFILTER_FRAG_PATH);

		StageShaderArray shaderArray{};
		shaderArray[ShaderStage::Vertex] = pVertShader;
		shaderArray[ShaderStage::Fragment] = pFragShader;

		std::vector<vk::DescriptorSetLayout> layouts = { _envMapDescriptorSetLayout };
		_prefilterPipeline = std::make_unique<VulkanPipeline>(
			vk::Format::eR16G16B16A16Sfloat, shaderArray, layouts,
			static_cast<uint32_t>(sizeof(PrefilterPushConstants)), false);

		// Record all mip levels and faces into one command buffer
		VulkanCommandBuffer cmd(true);
		cmd.Begin();

		// Transition all layers and mips to color attachment optimal
		cmd.ImageMemoryBarrier(_prefilteredMap->GetImage(),
			vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal,
			vk::AccessFlags{}, vk::AccessFlagBits::eColorAttachmentWrite,
			vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::ImageAspectFlagBits::eColor, 0, PREFILTER_MIP_LEVELS, 0, 6);

		glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

		// Render each mip level and face
		for (uint32_t mip = 0; mip < PREFILTER_MIP_LEVELS; mip++)
		{
			uint32_t mipWidth = PREFILTER_SIZE >> mip;
			uint32_t mipHeight = PREFILTER_SIZE >> mip;
			float roughness = static_cast<float>(mip) / static_cast<float>(PREFILTER_MIP_LEVELS - 1);

			for (uint32_t face = 0; face < 6; face++)
			{
				vk::ImageView faceView = _prefilteredMap->CreateSingleLayerMipView(face, mip);

				PrefilterPushConstants pc;
				pc.faceVP = proj * GetCubemapFaceViewMatrix(face);
				pc.roughness = roughness;

				RenderFullscreenPass(&cmd, faceView, mipWidth, mipHeight,
									 _prefilterPipeline.get(), _envMapDescriptorSet,
									 &pc, sizeof(PrefilterPushConstants));

				device.destroyImageView(faceView);
			}
		}

		// Transition to shader read optimal (all layers and mips)
		cmd.ImageMemoryBarrier(_prefilteredMap->GetImage(),
			vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
			vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eShaderRead,
			vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eFragmentShader,
			vk::ImageAspectFlagBits::eColor, 0, PREFILTER_MIP_LEVELS, 0, 6);

		cmd.End();

		vk::SubmitInfo submitInfo;
		submitInfo.setCommandBuffers(cmd.GetBuffer());
		VulkanContext::GetGraphicQueue().submit(submitInfo);
		VulkanContext::GetGraphicQueue().waitIdle();

		Logger::LogInfo("IBLManager: Pre-filtered map computed ({}x{}, {} mip levels)",
						PREFILTER_SIZE, PREFILTER_SIZE, PREFILTER_MIP_LEVELS);
	}

	void IBLManager::ComputeBRDFLUT(ShaderLibrary* pShaderLibrary)
	{
		// Create 2D render target for BRDF LUT
		RenderTargetConfig config;
		config.width = BRDF_LUT_SIZE;
		config.height = BRDF_LUT_SIZE;
		config.format = vk::Format::eR16G16Sfloat;
		config.samples = vk::SampleCountFlagBits::e1;
		config.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
		config.aspectMask = vk::ImageAspectFlagBits::eColor;
		config.transient = false;
		config.mipLevels = 1;
		config.arrayLayers = 1;
		config.flags = {};
		config.viewType = vk::ImageViewType::e2D;

		_brdfLUT = std::make_unique<RenderTarget>(config);

		// Create sampler
		VulkanSamplerCreateConfig samplerConfig;
		samplerConfig.magFilter = vk::Filter::eLinear;
		samplerConfig.minFilter = vk::Filter::eLinear;
		samplerConfig.addressModeU = vk::SamplerAddressMode::eClampToEdge;
		samplerConfig.addressModeV = vk::SamplerAddressMode::eClampToEdge;
		samplerConfig.addressModeW = vk::SamplerAddressMode::eClampToEdge;
		samplerConfig.anisotropyEnable = false;
		_brdfLUTSampler = VulkanContext::GetResourceManager()->CreateSampler(samplerConfig);

		// Create pipeline (no input textures, empty descriptor set layout)
		auto* pVertShader = pShaderLibrary->GetShader(ShaderStage::Vertex, FULLSCREEN_VERT_PATH);
		auto* pFragShader = pShaderLibrary->GetShader(ShaderStage::Fragment, BRDF_LUT_FRAG_PATH);

		StageShaderArray shaderArray{};
		shaderArray[ShaderStage::Vertex] = pVertShader;
		shaderArray[ShaderStage::Fragment] = pFragShader;

		std::vector<vk::DescriptorSetLayout> layouts = { _emptyDescriptorSetLayout };
		_brdfLUTPipeline = std::make_unique<VulkanPipeline>(
			vk::Format::eR16G16Sfloat, shaderArray, layouts, 0, false);

		VulkanCommandBuffer cmd(true);
		cmd.Begin();

		// Transition BRDF LUT image to color attachment optimal
		cmd.ImageMemoryBarrier(_brdfLUT->GetImage(),
			vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal,
			vk::AccessFlags{}, vk::AccessFlagBits::eColorAttachmentWrite,
			vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput);

		// Render BRDF LUT (no push constants)
		RenderFullscreenPass(&cmd, _brdfLUT->GetImageView(), BRDF_LUT_SIZE, BRDF_LUT_SIZE,
							 _brdfLUTPipeline.get(), _emptyDescriptorSet,
							 nullptr, 0);

		// Transition to shader read optimal
		cmd.ImageMemoryBarrier(_brdfLUT->GetImage(),
			vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
			vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eShaderRead,
			vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eFragmentShader);

		cmd.End();

		vk::SubmitInfo submitInfo;
		submitInfo.setCommandBuffers(cmd.GetBuffer());
		VulkanContext::GetGraphicQueue().submit(submitInfo);
		VulkanContext::GetGraphicQueue().waitIdle();

		Logger::LogInfo("IBLManager: BRDF LUT computed ({}x{})", BRDF_LUT_SIZE, BRDF_LUT_SIZE);
	}

	void IBLManager::RenderFullscreenPass(VulkanCommandBuffer* pCmdBuffer,
										   vk::ImageView colorView, uint32_t width, uint32_t height,
										   VulkanPipeline* pPipeline, vk::DescriptorSet descriptorSet,
										   const void* pushConstantData, uint32_t pushConstantSize)
	{
		vk::Extent2D extent{ width, height };
		pCmdBuffer->BeginRendering(colorView, nullptr, nullptr, extent, true, false);
		pCmdBuffer->BindPipeline(pPipeline);
		pCmdBuffer->SetViewportAndScissor(width, height);

		std::vector<vk::DescriptorSet> sets{ descriptorSet };
		pCmdBuffer->BindDescriptorSet(pPipeline->GetPipelineLayout(), sets);

		if (pushConstantData && pushConstantSize > 0)
		{
			pCmdBuffer->PushConstants(pPipeline, vk::ShaderStageFlagBits::eFragment,
				0, pushConstantSize, pushConstantData);
		}

		pCmdBuffer->DrawNonIndexed(3);
		pCmdBuffer->EndRendering();
	}

	vk::ImageView IBLManager::GetIrradianceMapView() const
	{
		return _irradianceMap ? _irradianceMap->GetImageView() : nullptr;
	}

	vk::ImageView IBLManager::GetPrefilteredMapView() const
	{
		return _prefilteredMap ? _prefilteredMap->GetImageView() : nullptr;
	}

	vk::ImageView IBLManager::GetBRDFLUTView() const
	{
		return _brdfLUT ? _brdfLUT->GetImageView() : nullptr;
	}

	VulkanSampler* IBLManager::GetIrradianceSampler() const
	{
		return _irradianceSampler;
	}

	VulkanSampler* IBLManager::GetPrefilteredSampler() const
	{
		return _prefilteredSampler;
	}

	VulkanSampler* IBLManager::GetBRDFLUTSampler() const
	{
		return _brdfLUTSampler;
	}

	void IBLManager::Shutdown()
	{
		auto device = VulkanContext::GetDevice();

		_irradiancePipeline.reset();
		_prefilterPipeline.reset();
		_brdfLUTPipeline.reset();

		_irradianceMap.reset();
		_prefilteredMap.reset();
		_brdfLUT.reset();

		if (_envMapDescriptorPool)
		{
			device.destroyDescriptorPool(_envMapDescriptorPool);
			_envMapDescriptorPool = nullptr;
		}

		if (_envMapDescriptorSetLayout)
		{
			device.destroyDescriptorSetLayout(_envMapDescriptorSetLayout);
			_envMapDescriptorSetLayout = nullptr;
		}

		if (_emptyDescriptorPool)
		{
			device.destroyDescriptorPool(_emptyDescriptorPool);
			_emptyDescriptorPool = nullptr;
		}

		if (_emptyDescriptorSetLayout)
		{
			device.destroyDescriptorSetLayout(_emptyDescriptorSetLayout);
			_emptyDescriptorSetLayout = nullptr;
		}

		// Samplers are owned by VulkanResourceManager
		_irradianceSampler = nullptr;
		_prefilteredSampler = nullptr;
		_brdfLUTSampler = nullptr;

		_ready = false;
	}
} // namespace Ailurus
