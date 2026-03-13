#include "Ailurus/Systems/RenderSystem/Descriptor/SamplerSetData.h"
#include "VulkanContext/Descriptor/VulkanDescriptorWriter.h"

namespace Ailurus
{
	void SamplerSetData::SetImage(uint32_t binding, vk::ImageView imageView, vk::Sampler sampler)
	{
		for (auto& b : _bindings)
		{
			if (b.binding == binding)
			{
				b.imageView = imageView;
				b.sampler = sampler;
				return;
			}
		}
		_bindings.push_back({ binding, imageView, sampler });
	}

	void SamplerSetData::Clear()
	{
		_bindings.clear();
	}

	void SamplerSetData::WriteBindings(VulkanDescriptorWriter& writer) const
	{
		for (const auto& b : _bindings)
			writer.WriteImage(b.binding, b.imageView, b.sampler);
	}

	auto SamplerSetData::ComputeBindingHash() const -> size_t
	{
		size_t hash = 0;
		for (const auto& b : _bindings)
			hash ^= reinterpret_cast<size_t>(static_cast<VkImageView>(b.imageView));
		return hash;
	}
} // namespace Ailurus
