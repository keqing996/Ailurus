#pragma once

#include <vulkan/vulkan.hpp>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Application/Shader/Uniform/UniformLayout.h>

namespace Ailurus
{
	class RHIShaderProgram: public NonCopyable
	{
	public:
		RHIShaderProgram();
		~RHIShaderProgram() override;

	public:
		void SetUniformLayout(const std::vector<UniformLayout>& uniformLayoutVec);
		vk::DescriptorSetLayout GetDescriptorSetLayout() const;

	private:
		void DestroySetLayout();

	private:
		vk::DescriptorSetLayout _descriptorSetLayout;
	};
}