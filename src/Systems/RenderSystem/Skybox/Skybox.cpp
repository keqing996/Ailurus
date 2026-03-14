#include "Skybox.h"
#include <Ailurus/OS/Path.h>
#include "Ailurus/Utility/Logger.h"
#include <Ailurus/Utility/Image.h>
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
#include <glm/gtc/packing.hpp>
#include <algorithm>
#include <cmath>
#include <vector>
#include <array>

namespace Ailurus
{
	namespace
	{
		constexpr float PI = 3.14159265358979323846f;

		void ComputeFaceDirection(uint32_t face, float s, float t, float& dx, float& dy, float& dz)
		{
			switch (face)
			{
				case 0: dx =  1.0f; dy = -t;    dz = -s;    break;
				case 1: dx = -1.0f; dy = -t;    dz =  s;    break;
				case 2: dx =  s;    dy =  1.0f; dz =  t;    break;
				case 3: dx =  s;    dy = -1.0f; dz = -t;    break;
				case 4: dx =  s;    dy = -t;    dz =  1.0f; break;
				case 5: dx = -s;    dy = -t;    dz = -1.0f; break;
				default: dx = dy = dz = 0.0f; break;
			}
		}

		std::array<float, 4> LoadPixelAsFloat4(const Image& image, uint32_t x, uint32_t y)
		{
			auto [width, height] = image.GetPixelSize();
			if (width == 0 || height == 0)
				return { 0.0f, 0.0f, 0.0f, 1.0f };

			x = std::min(x, width - 1);
			y = std::min(y, height - 1);

			const size_t index = (static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)) * 4;
			if (image.IsHDR())
			{
				const float* pData = image.GetHDRData();
				return { pData[index + 0], pData[index + 1], pData[index + 2], pData[index + 3] };
			}

			const std::uint8_t* pData = image.GetBytesData();
			return {
				static_cast<float>(pData[index + 0]) / 255.0f,
				static_cast<float>(pData[index + 1]) / 255.0f,
				static_cast<float>(pData[index + 2]) / 255.0f,
				static_cast<float>(pData[index + 3]) / 255.0f
			};
		}

		std::array<float, 4> SampleEquirectangular(const Image& image, float u, float v)
		{
			auto [width, height] = image.GetPixelSize();
			if (width == 0 || height == 0)
				return { 0.0f, 0.0f, 0.0f, 1.0f };

			const float wrappedU = u - std::floor(u);
			const float clampedV = std::clamp(v, 0.0f, 1.0f);
			const float x = wrappedU * static_cast<float>(width - 1);
			const float y = clampedV * static_cast<float>(height - 1);

			const uint32_t x0 = static_cast<uint32_t>(std::floor(x));
			const uint32_t y0 = static_cast<uint32_t>(std::floor(y));
			const uint32_t x1 = width > 1 ? (x0 + 1) % width : x0;
			const uint32_t y1 = std::min(y0 + 1, height - 1);

			const float tx = x - static_cast<float>(x0);
			const float ty = y - static_cast<float>(y0);

			auto c00 = LoadPixelAsFloat4(image, x0, y0);
			auto c10 = LoadPixelAsFloat4(image, x1, y0);
			auto c01 = LoadPixelAsFloat4(image, x0, y1);
			auto c11 = LoadPixelAsFloat4(image, x1, y1);

			std::array<float, 4> result{};
			for (size_t channel = 0; channel < result.size(); ++channel)
			{
				const float top = c00[channel] + (c10[channel] - c00[channel]) * tx;
				const float bottom = c01[channel] + (c11[channel] - c01[channel]) * tx;
				result[channel] = top + (bottom - top) * ty;
			}

			return result;
		}
	}

	static const char* SKYBOX_VERT_PATH = "./Assets/ShaderBin/skybox.vert.spv";
	static const char* SKYBOX_FRAG_PATH = "./Assets/ShaderBin/skybox.frag.spv";

	Skybox::~Skybox() = default;

	void Skybox::Init(ShaderLibrary* pShaderLibrary, vk::Format colorFormat, vk::Format depthFormat,
		const std::string& hdrTexturePath)
	{
		_pShaderLibrary = pShaderLibrary;

		bool loadedFromTexture = false;
		if (!hdrTexturePath.empty())
			loadedFromTexture = LoadEquirectangularCubemap(hdrTexturePath);

		if (!loadedFromTexture)
		{
			if (!hdrTexturePath.empty())
				Logger::LogWarn("Skybox: Failed to load texture '{}', falling back to generated gradient cubemap", hdrTexturePath);

			GenerateGradientCubemap();
		}

		if (_pCubemapImage == nullptr || _pSampler == nullptr)
		{
			Logger::LogError("Skybox: Initialization failed because cubemap resources were not created");
			return;
		}

		CreateDescriptorResources();
		CreatePipeline(pShaderLibrary, colorFormat, depthFormat);
	}

	bool Skybox::LoadEquirectangularCubemap(const std::string& filePath)
	{
		const std::string resolvedPath = Path::ResolvePath(filePath);
		Image image(resolvedPath);
		auto [sourceWidth, sourceHeight] = image.GetPixelSize();
		if (sourceWidth == 0 || sourceHeight == 0)
		{
			Logger::LogWarn("Skybox: Could not read texture '{}' (resolved: '{}')", filePath, resolvedPath);
			return false;
		}

		const uint32_t faceSize = std::max(1u, std::min(sourceWidth / 4u, sourceHeight / 2u));
		const size_t facePixelCount = static_cast<size_t>(faceSize) * static_cast<size_t>(faceSize);
		std::vector<std::uint16_t> cubemapData(facePixelCount * 6 * 4, 0);

		for (uint32_t face = 0; face < 6; ++face)
		{
			for (uint32_t row = 0; row < faceSize; ++row)
			{
				for (uint32_t col = 0; col < faceSize; ++col)
				{
					const float u = (static_cast<float>(col) + 0.5f) / static_cast<float>(faceSize);
					const float v = (static_cast<float>(row) + 0.5f) / static_cast<float>(faceSize);
					const float s = u * 2.0f - 1.0f;
					const float t = v * 2.0f - 1.0f;

					float dx = 0.0f;
					float dy = 0.0f;
					float dz = 0.0f;
					ComputeFaceDirection(face, s, t, dx, dy, dz);

					const float len = std::sqrt(dx * dx + dy * dy + dz * dz);
					if (len <= 0.0f)
						continue;

					dx /= len;
					dy /= len;
					dz /= len;

					const float envU = 0.5f + std::atan2(dz, dx) / (2.0f * PI);
					// Most lat-long HDR assets are authored in a convention that needs a vertical
					// flip when sampling into our cubemap orientation, otherwise sky and ground invert.
					const float envV = 1.0f - std::acos(std::clamp(dy, -1.0f, 1.0f)) / PI;
					auto sample = SampleEquirectangular(image, envU, envV);

					const size_t pixelIndex = (static_cast<size_t>(face) * facePixelCount
						+ static_cast<size_t>(row) * static_cast<size_t>(faceSize)
						+ static_cast<size_t>(col)) * 4;
					cubemapData[pixelIndex + 0] = glm::packHalf1x16(sample[0]);
					cubemapData[pixelIndex + 1] = glm::packHalf1x16(sample[1]);
					cubemapData[pixelIndex + 2] = glm::packHalf1x16(sample[2]);
					cubemapData[pixelIndex + 3] = glm::packHalf1x16(sample[3]);
				}
			}
		}

		VulkanImageCreateConfig config;
		config.width = faceSize;
		config.height = faceSize;
		config.format = vk::Format::eR16G16B16A16Sfloat;
		config.mipLevels = 1;
		config.arrayLayers = 6;
		config.flags = vk::ImageCreateFlagBits::eCubeCompatible;
		config.viewType = vk::ImageViewType::eCube;
		config.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
		config.aspectMask = vk::ImageAspectFlagBits::eColor;

		_pCubemapImage = VulkanContext::GetResourceManager()->CreateImageFromConfig(
			config, cubemapData.data(), cubemapData.size() * sizeof(std::uint16_t));
		if (_pCubemapImage == nullptr)
		{
			Logger::LogError("Skybox: Failed to create cubemap image from '{}'", resolvedPath);
			return false;
		}

		CreateSampler();
		Logger::LogInfo("Skybox: Loaded '{}' into {}x{} cubemap", resolvedPath, faceSize, faceSize);
		return _pSampler != nullptr;
	}

	void Skybox::GenerateGradientCubemap()
	{
		// Sky colors
		constexpr float skyTop[3]     = { 0.15f, 0.35f, 0.85f }; // Deep blue zenith
		constexpr float skyHorizon[3] = { 0.6f,  0.75f, 0.95f }; // Light blue horizon
		constexpr float ground[3]     = { 0.25f, 0.2f,  0.15f }; // Dark earthy brown

		// Generate 6 face images, each DEFAULT_FACE_SIZE x DEFAULT_FACE_SIZE x 4 channels (RGBA)
		const uint32_t facePixelCount = DEFAULT_FACE_SIZE * DEFAULT_FACE_SIZE * 4;
		std::vector<uint8_t> allFaceData(facePixelCount * 6);

		for (uint32_t face = 0; face < 6; face++)
		{
			uint8_t* faceData = allFaceData.data() + face * facePixelCount;

			for (uint32_t row = 0; row < DEFAULT_FACE_SIZE; row++)
			{
				for (uint32_t col = 0; col < DEFAULT_FACE_SIZE; col++)
				{
					// Compute normalized UV for this pixel [0, 1]
					float u = (static_cast<float>(col) + 0.5f) / static_cast<float>(DEFAULT_FACE_SIZE);
					float v = (static_cast<float>(row) + 0.5f) / static_cast<float>(DEFAULT_FACE_SIZE);

					// Map to [-1, 1]
					float s = u * 2.0f - 1.0f;
					float t = v * 2.0f - 1.0f;

					// Compute 3D direction based on cubemap face
					float dx = 0.0f;
					float dy = 0.0f;
					float dz = 0.0f;
					ComputeFaceDirection(face, s, t, dx, dy, dz);

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

					uint32_t idx = (row * DEFAULT_FACE_SIZE + col) * 4;
					faceData[idx + 0] = static_cast<uint8_t>(std::min(r * 255.0f, 255.0f));
					faceData[idx + 1] = static_cast<uint8_t>(std::min(g * 255.0f, 255.0f));
					faceData[idx + 2] = static_cast<uint8_t>(std::min(b * 255.0f, 255.0f));
					faceData[idx + 3] = 255;
				}
			}
		}

		// Create cubemap image via VulkanResourceManager
		VulkanImageCreateConfig config;
		config.width = DEFAULT_FACE_SIZE;
		config.height = DEFAULT_FACE_SIZE;
		config.format = vk::Format::eR8G8B8A8Srgb;
		config.mipLevels = 1;
		config.arrayLayers = 6;
		config.flags = vk::ImageCreateFlagBits::eCubeCompatible;
		config.viewType = vk::ImageViewType::eCube;
		config.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
		config.aspectMask = vk::ImageAspectFlagBits::eColor;

		_pCubemapImage = VulkanContext::GetResourceManager()->CreateImageFromConfig(
			config, allFaceData.data(), static_cast<uint32_t>(allFaceData.size()));

		CreateSampler();
	}

	void Skybox::CreateSampler()
	{
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
		if (_pPipeline == nullptr || _descriptorSet == nullptr)
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
