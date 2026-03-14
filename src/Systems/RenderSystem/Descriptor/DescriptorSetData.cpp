#include "Ailurus/Systems/RenderSystem/Descriptor/DescriptorSetData.h"
#include "Ailurus/Systems/RenderSystem/Descriptor/DescriptorSetSchema.h"
#include "VulkanContext/Descriptor/VulkanDescriptorAllocator.h"
#include "VulkanContext/Descriptor/VulkanDescriptorSetLayout.h"
#include "VulkanContext/Descriptor/VulkanDescriptorWriter.h"

namespace Ailurus
{
	auto DescriptorSetData::AllocateAndWrite(
		VulkanDescriptorAllocator* pAllocator,
		const DescriptorSetSchema* pSchema,
		VulkanCommandBuffer* pCmdBuffer) -> VulkanDescriptorSet
	{
		auto* pLayout = pSchema->GetDescriptorSetLayout();

		VulkanDescriptorAllocator::CacheKey key;
		key.layout = pLayout->GetDescriptorSetLayout();
		key.bindingHash = ComputeBindingHash();

		bool wasCacheHit = false;
		auto descriptorSet = pAllocator->AllocateDescriptorSet(pLayout, &key, &wasCacheHit);

		PrepareGpuData(pCmdBuffer);

		if (wasCacheHit)
			return descriptorSet;

		VulkanDescriptorWriter writer;
		WriteBindings(writer);
		writer.UpdateSet(descriptorSet);

		return descriptorSet;
	}

	void DescriptorSetData::PrepareGpuData(VulkanCommandBuffer* /*pCmdBuffer*/)
	{
		// No-op by default
	}
} // namespace Ailurus
