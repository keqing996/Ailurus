#pragma once

#include <vulkan/vulkan.hpp>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>
#include <Ailurus/Application/RenderSystem/Shader/Uniform/UniformLayout.h>

namespace Ailurus
{
	class VulkanShaderProgram: public NonCopyable, public NonMovable
	{
	public:
		VulkanShaderProgram();
		~VulkanShaderProgram();

	public:
		void SetUniformLayout(const std::vector<UniformLayout>& uniformLayoutVec);
		vk::DescriptorSetLayout GetDescriptorSetLayout() const;

	private:
		void DestroySetLayout();

	private:
		vk::DescriptorSetLayout _descriptorSetLayout;
	};
}