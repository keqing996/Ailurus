#include "Ailurus/Systems/RenderSystem/Descriptor/SamplerSchema.h"
#include "VulkanContext/Descriptor/VulkanDescriptorSetLayout.h"

namespace Ailurus
{
	SamplerSchema::SamplerSchema(const std::vector<SamplerBindingDesc>& bindings)
	{
		std::vector<vk::DescriptorSetLayoutBinding> layoutBindings;
		layoutBindings.reserve(bindings.size());

		for (const auto& desc : bindings)
		{
			vk::DescriptorSetLayoutBinding binding;
			binding.setBinding(desc.binding)
				.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
				.setDescriptorCount(1)
				.setStageFlags(desc.stageFlags);
			layoutBindings.push_back(binding);
		}

		_pDescriptorSetLayout = std::make_unique<VulkanDescriptorSetLayout>(layoutBindings);
	}

	SamplerSchema::~SamplerSchema() = default;
} // namespace Ailurus
