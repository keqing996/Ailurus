#pragma once

#include <cstdint>
#include <vector>
#include <vulkan/vulkan.hpp>
#include "DescriptorSetData.h"

namespace Ailurus
{
	class SamplerSetData : public DescriptorSetData
	{
	public:
		SamplerSetData() = default;
		~SamplerSetData() override = default;

		void SetImage(uint32_t binding, vk::ImageView imageView, vk::Sampler sampler);
		void Clear();

	protected:
		void WriteBindings(VulkanDescriptorWriter& writer) const override;
		auto ComputeBindingHash() const -> size_t override;

	private:
		struct ImageBinding
		{
			uint32_t binding;
			vk::ImageView imageView;
			vk::Sampler sampler;
		};

		std::vector<ImageBinding> _bindings;
	};
} // namespace Ailurus
