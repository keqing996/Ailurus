#pragma once

#include <vulkan/vulkan.hpp>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Application/Render/Shader/Uniform/UniformLayout.h>

namespace Ailurus
{
	class RhiShaderProgram: public NonCopyable
	{
	public:
		RhiShaderProgram();
		~RhiShaderProgram() override;

	public:
		void SetUniformLayout(const std::vector<UniformLayout>& uniformLayoutVec);
		vk::DescriptorSetLayout GetDescriptorSetLayout() const;

	private:
		void DestroySetLayout();

	private:
		vk::DescriptorSetLayout _descriptorSetLayout;
	};
}