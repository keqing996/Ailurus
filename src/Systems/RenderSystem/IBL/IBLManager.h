#pragma once

#include <memory>
#include <vulkan/vulkan.hpp>

namespace Ailurus
{
	class RenderTarget;
	class VulkanSampler;
	class VulkanPipeline;
	class VulkanImage;
	class VulkanCommandBuffer;
	class ShaderLibrary;
	class Shader;
	class VulkanDescriptorSetLayout;

	class IBLManager
	{
	public:
		IBLManager();
		~IBLManager();

		// Precompute IBL maps from the skybox cubemap
		void Precompute(vk::ImageView envCubemapView, vk::Sampler envSampler,
						ShaderLibrary* pShaderLibrary);

		vk::ImageView GetIrradianceMapView() const;
		vk::ImageView GetPrefilteredMapView() const;
		vk::ImageView GetBRDFLUTView() const;
		VulkanSampler* GetIrradianceSampler() const;
		VulkanSampler* GetPrefilteredSampler() const;
		VulkanSampler* GetBRDFLUTSampler() const;

		bool IsReady() const { return _ready; }
		void Shutdown();

	private:
		void ComputeIrradianceMap(vk::ImageView envCubemapView, vk::Sampler envSampler,
								   ShaderLibrary* pShaderLibrary);
		void ComputePrefilteredMap(vk::ImageView envCubemapView, vk::Sampler envSampler,
									ShaderLibrary* pShaderLibrary);
		void ComputeBRDFLUT(ShaderLibrary* pShaderLibrary);

		// Helper to render a fullscreen triangle pass
		void RenderFullscreenPass(VulkanCommandBuffer* pCmdBuffer,
								   vk::ImageView colorView, uint32_t width, uint32_t height,
								   VulkanPipeline* pPipeline, vk::DescriptorSet descriptorSet,
								   const void* pushConstantData, uint32_t pushConstantSize);

	private:
		static constexpr uint32_t IRRADIANCE_SIZE = 32;
		static constexpr uint32_t PREFILTER_SIZE = 128;
		static constexpr uint32_t PREFILTER_MIP_LEVELS = 5;
		static constexpr uint32_t BRDF_LUT_SIZE = 512;

		// Irradiance cubemap
		std::unique_ptr<RenderTarget> _irradianceMap;
		VulkanSampler* _irradianceSampler = nullptr;

		// Pre-filtered env cubemap
		std::unique_ptr<RenderTarget> _prefilteredMap;
		VulkanSampler* _prefilteredSampler = nullptr;

		// BRDF LUT
		std::unique_ptr<RenderTarget> _brdfLUT;
		VulkanSampler* _brdfLUTSampler = nullptr;

		// Descriptor resources for precompute pipelines
		vk::DescriptorSetLayout _envMapDescriptorSetLayout;
		vk::DescriptorPool _envMapDescriptorPool;
		vk::DescriptorSet _envMapDescriptorSet;

		// Empty descriptor set layout for BRDF LUT pipeline
		vk::DescriptorSetLayout _emptyDescriptorSetLayout;
		vk::DescriptorPool _emptyDescriptorPool;
		vk::DescriptorSet _emptyDescriptorSet;

		// Pipelines
		std::unique_ptr<VulkanPipeline> _irradiancePipeline;
		std::unique_ptr<VulkanPipeline> _prefilterPipeline;
		std::unique_ptr<VulkanPipeline> _brdfLUTPipeline;

		bool _ready = false;
	};
} // namespace Ailurus
