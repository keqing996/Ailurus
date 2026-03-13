#pragma once

#include <memory>
#include <vulkan/vulkan.hpp>
#include <Ailurus/Math/Matrix4x4.hpp>
#include <VulkanContext/Pipeline/VulkanPipeline.h>

namespace Ailurus
{
	class VulkanImage;
	class VulkanSampler;
	class VulkanCommandBuffer;
	class VulkanDescriptorSetLayout;
	class ShaderLibrary;

	class Skybox
	{
	public:
		~Skybox();

		void Init(ShaderLibrary* pShaderLibrary, vk::Format colorFormat, vk::Format depthFormat);
		void Render(VulkanCommandBuffer* pCmdBuffer, const Matrix4x4f& inverseVP);
		void RebuildPipeline(vk::Format colorFormat, vk::Format depthFormat);
		void Shutdown();

		vk::ImageView GetCubemapImageView() const;
		VulkanSampler* GetCubemapSampler() const;

	private:
		void GenerateGradientCubemap();
		void CreateDescriptorResources();
		void CreatePipeline(ShaderLibrary* pShaderLibrary, vk::Format colorFormat, vk::Format depthFormat);

	private:
		static constexpr uint32_t FACE_SIZE = 64;

		// Cubemap image (owned by VulkanResourceManager)
		VulkanImage* _pCubemapImage = nullptr;
		VulkanSampler* _pSampler = nullptr;

		// Pipeline (owned by Skybox)
		std::unique_ptr<VulkanPipeline> _pPipeline;

		// Descriptor resources (owned by Skybox)
		std::unique_ptr<VulkanDescriptorSetLayout> _descriptorSetLayout;
		vk::DescriptorPool _descriptorPool;
		vk::DescriptorSet _descriptorSet;

		ShaderLibrary* _pShaderLibrary = nullptr;
	};
} // namespace Ailurus
