#include "Skybox.h"
#include "Ailurus/Utility/Logger.h"
#include "Ailurus/Systems/RenderSystem/Shader/ShaderLibrary.h"
#include "Ailurus/Systems/RenderSystem/Shader/ShaderStage.h"
#include <VulkanContext/VulkanContext.h>
#include <VulkanContext/Resource/VulkanResourceManager.h>
#include <VulkanContext/Resource/Image/VulkanImage.h>
#include <VulkanContext/Resource/Image/VulkanSampler.h>
#include <VulkanContext/Pipeline/VulkanPipeline.h>
#include <VulkanContext/CommandBuffer/VulkanCommandBuffer.h>
#include <VulkanContext/Descriptor/VulkanDescriptorWriter.h>
#include <VulkanContext/Descriptor/VulkanDescriptorSetLayout.h>
#include <cmath>
#include <vector>
#include <array>

namespace Ailurus
{
	static const char* SKYBOX_VERT_PATH = "./Assets/ShaderBin/skybox.vert.spv";
	static const char* SKYBOX_FRAG_PATH = "./Assets/ShaderBin/skybox.frag.spv";

	Skybox::~Skybox() = default;

	void Skybox::Init(ShaderLibrary* pShaderLibrary, vk::Format colorFormat, vk::Format depthFormat)
	{
		_pShaderLibrary = pShaderLibrary;

		GenerateGradientCubemap();
		CreateDescriptorResources();
		CreatePipeline(pShaderLibrary, colorFormat, depthFormat);
	}

	void Skybox::GenerateGradientCubemap()
	{
		// Sky colors
		constexpr float skyTop[3]     = { 0.15f, 0.35f, 0.85f }; // Deep blue zenith
		constexpr float skyHorizon[3] = { 0.6f,  0.75f, 0.95f }; // Light blue horizon
		constexpr float ground[3]     = { 0.25f, 0.2f,  0.15f }; // Dark earthy brown

		// Generate 6 face images, each FACE_SIZE x FACE_SIZE x 4 channels (RGBA)
		const uint32_t facePixelCount = FACE_SIZE * FACE_SIZE * 4;
		std::vector<uint8_t> allFaceData(facePixelCount * 6);

		for (uint32_t face = 0; face < 6; face++)
		{
			uint8_t* faceData = allFaceData.data() + face * facePixelCount;

			for (uint32_t row = 0; row < FACE_SIZE; row++)
			{
				for (uint32_t col = 0; col < FACE_SIZE; col++)
				{
					// Compute normalized UV for this pixel [0, 1]
					float u = (static_cast<float>(col) + 0.5f) / static_cast<float>(FACE_SIZE);
					float v = (static_cast<float>(row) + 0.5f) / static_cast<float>(FACE_SIZE);

					// Map to [-1, 1]
					float s = u * 2.0f - 1.0f;
					float t = v * 2.0f - 1.0f;

					// Compute 3D direction based on cubemap face
					float dx, dy, dz;
					switch (face)
					{
						case 0: dx =  1.0f; dy = -t;    dz = -s;    break; // +X
						case 1: dx = -1.0f; dy = -t;    dz =  s;    break; // -X
						case 2: dx =  s;    dy =  1.0f; dz =  t;    break; // +Y
						case 3: dx =  s;    dy = -1.0f; dz = -t;    break; // -Y
						case 4: dx =  s;    dy = -t;    dz =  1.0f; break; // +Z
						case 5: dx = -s;    dy = -t;    dz = -1.0f; break; // -Z
						default: dx = dy = dz = 0.0f; break;
					}

					// Normalize direction
					float len = std::sqrt(dx * dx + dy * dy + dz * dz);
					float ny = dy / len; // Only need Y for vertical gradient

					// Blend color based on Y component
					float r, g, b;
					if (ny > 0.0f)
					{
						// Sky: blend from horizon to zenith
						float t2 = std::pow(ny, 0.6f); // Softer gradient
						r = skyHorizon[0] + (skyTop[0] - skyHorizon[0]) * t2;
						g = skyHorizon[1] + (skyTop[1] - skyHorizon[1]) * t2;
						b = skyHorizon[2] + (skyTop[2] - skyHorizon[2]) * t2;
					}
					else
					{
						// Ground: blend from horizon to ground
						float t2 = std::pow(-ny, 0.4f);
						r = skyHorizon[0] + (ground[0] - skyHorizon[0]) * t2;
						g = skyHorizon[1] + (ground[1] - skyHorizon[1]) * t2;
						b = skyHorizon[2] + (ground[2] - skyHorizon[2]) * t2;
					}

					uint32_t idx = (row * FACE_SIZE + col) * 4;
					faceData[idx + 0] = static_cast<uint8_t>(std::min(r * 255.0f, 255.0f));
					faceData[idx + 1] = static_cast<uint8_t>(std::min(g * 255.0f, 255.0f));
					faceData[idx + 2] = static_cast<uint8_t>(std::min(b * 255.0f, 255.0f));
					faceData[idx + 3] = 255;
				}
			}
		}

		// Create cubemap image via VulkanResourceManager
		VulkanImageCreateConfig config;
		config.width = FACE_SIZE;
		config.height = FACE_SIZE;
		config.format = vk::Format::eR8G8B8A8Srgb;
		config.mipLevels = 1;
		config.arrayLayers = 6;
		config.flags = vk::ImageCreateFlagBits::eCubeCompatible;
		config.viewType = vk::ImageViewType::eCube;
		config.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
		config.aspectMask = vk::ImageAspectFlagBits::eColor;

		_pCubemapImage = VulkanContext::GetResourceManager()->CreateImageFromConfig(
			config, allFaceData.data(), static_cast<uint32_t>(allFaceData.size()));

		// Create sampler with linear filtering
		VulkanSamplerCreateConfig samplerConfig;
		samplerConfig.magFilter = vk::Filter::eLinear;
		samplerConfig.minFilter = vk::Filter::eLinear;
		samplerConfig.addressModeU = vk::SamplerAddressMode::eClampToEdge;
		samplerConfig.addressModeV = vk::SamplerAddressMode::eClampToEdge;
		samplerConfig.addressModeW = vk::SamplerAddressMode::eClampToEdge;
		samplerConfig.anisotropyEnable = false;

		_pSampler = VulkanContext::GetResourceManager()->CreateSampler(samplerConfig);
	}

	void Skybox::CreateDescriptorResources()
	{
		auto device = VulkanContext::GetDevice();

		// Create descriptor set layout: single cubemap sampler at binding 0
		vk::DescriptorSetLayoutBinding binding;
		binding.setBinding(0)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setDescriptorCount(1)
			.setStageFlags(vk::ShaderStageFlagBits::eFragment);

		_descriptorSetLayout = std::make_unique<VulkanDescriptorSetLayout>(
			std::vector<vk::DescriptorSetLayoutBinding>{ binding });

		// Create descriptor pool
		vk::DescriptorPoolSize poolSize;
		poolSize.setType(vk::DescriptorType::eCombinedImageSampler)
			.setDescriptorCount(1);

		vk::DescriptorPoolCreateInfo poolInfo;
		poolInfo.setMaxSets(1)
			.setPoolSizes(poolSize);
		_descriptorPool = device.createDescriptorPool(poolInfo);

		// Allocate descriptor set
		auto rawLayout = _descriptorSetLayout->GetDescriptorSetLayout();
		vk::DescriptorSetAllocateInfo allocInfo;
		allocInfo.setDescriptorPool(_descriptorPool)
			.setSetLayouts(rawLayout);
		_descriptorSet = device.allocateDescriptorSets(allocInfo)[0];

		// Write cubemap image to descriptor set
		VulkanDescriptorWriter writer;
		writer.WriteImage(0, _pCubemapImage->GetImageView(), _pSampler->GetSampler());
		writer.UpdateSet(_descriptorSet);
	}

	void Skybox::CreatePipeline(ShaderLibrary* pShaderLibrary, vk::Format colorFormat, vk::Format depthFormat)
	{
		auto* pVertShader = pShaderLibrary->GetShader(ShaderStage::Vertex, SKYBOX_VERT_PATH);
		auto* pFragShader = pShaderLibrary->GetShader(ShaderStage::Fragment, SKYBOX_FRAG_PATH);

		StageShaderArray shaderArray{};
		shaderArray[ShaderStage::Vertex] = pVertShader;
		shaderArray[ShaderStage::Fragment] = pFragShader;

		std::vector<vk::DescriptorSetLayout> layouts = { _descriptorSetLayout->GetDescriptorSetLayout() };

		_pPipeline = std::make_unique<VulkanPipeline>(
			colorFormat, depthFormat, shaderArray, layouts, static_cast<uint32_t>(sizeof(Matrix4x4f)));
	}

	void Skybox::Render(VulkanCommandBuffer* pCmdBuffer, const Matrix4x4f& inverseVP)
	{
		if (_pPipeline == nullptr)
			return;

		pCmdBuffer->BindPipeline(_pPipeline.get());
		pCmdBuffer->SetViewportAndScissor();

		// Bind cubemap descriptor set
		std::vector<vk::DescriptorSet> descriptorSets = { _descriptorSet };
		pCmdBuffer->BindDescriptorSet(_pPipeline->GetPipelineLayout(), descriptorSets);

		// Push inverse view-projection matrix
		pCmdBuffer->PushConstantModelMatrix(_pPipeline.get(), inverseVP);

		// Draw fullscreen triangle
		pCmdBuffer->DrawNonIndexed(3);
	}

	void Skybox::RebuildPipeline(vk::Format colorFormat, vk::Format depthFormat)
	{
		_pPipeline.reset();
		CreatePipeline(_pShaderLibrary, colorFormat, depthFormat);
	}

	void Skybox::Shutdown()
	{
		auto device = VulkanContext::GetDevice();

		_pPipeline.reset();

		if (_descriptorPool)
		{
			device.destroyDescriptorPool(_descriptorPool);
			_descriptorPool = nullptr;
		}

		_descriptorSetLayout.reset();

		// VulkanImage and VulkanSampler are owned by VulkanResourceManager, not destroyed here
		_pCubemapImage = nullptr;
		_pSampler = nullptr;
	}

	vk::ImageView Skybox::GetCubemapImageView() const
	{
		return _pCubemapImage ? _pCubemapImage->GetImageView() : nullptr;
	}

	VulkanSampler* Skybox::GetCubemapSampler() const
	{
		return _pSampler;
	}
} // namespace Ailurus
