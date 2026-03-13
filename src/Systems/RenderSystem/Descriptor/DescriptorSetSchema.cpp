#include "Ailurus/Systems/RenderSystem/Descriptor/DescriptorSetSchema.h"
#include "VulkanContext/Descriptor/VulkanDescriptorSetLayout.h"

namespace Ailurus
{
	DescriptorSetSchema::~DescriptorSetSchema() = default;

	auto DescriptorSetSchema::GetDescriptorSetLayout() const -> VulkanDescriptorSetLayout*
	{
		return _pDescriptorSetLayout.get();
	}
} // namespace Ailurus
