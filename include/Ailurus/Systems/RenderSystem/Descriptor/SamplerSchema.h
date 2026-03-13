#pragma once

#include <cstdint>
#include <vector>
#include <vulkan/vulkan.hpp>
#include "DescriptorSetSchema.h"

namespace Ailurus
{
	struct SamplerBindingDesc
	{
		uint32_t binding;
		vk::ShaderStageFlags stageFlags = vk::ShaderStageFlagBits::eFragment;
	};

	class SamplerSchema : public DescriptorSetSchema
	{
	public:
		explicit SamplerSchema(const std::vector<SamplerBindingDesc>& bindings);
		~SamplerSchema() override;
	};
} // namespace Ailurus
