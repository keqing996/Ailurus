#pragma once

#include <cstddef>
#include "VulkanContext/Descriptor/VulkanDescriptorSet.h"

namespace Ailurus
{
	class VulkanDescriptorAllocator;
	class VulkanDescriptorWriter;
	class VulkanCommandBuffer;
	class DescriptorSetSchema;

	class DescriptorSetData
	{
	public:
		virtual ~DescriptorSetData() = default;

		/// @brief Allocate (with caching) and write descriptor set in one call.
		/// Template method: calls PrepareGpuData, ComputeBindingHash, WriteBindings.
		auto AllocateAndWrite(
			VulkanDescriptorAllocator* pAllocator,
			const DescriptorSetSchema* pSchema,
			VulkanCommandBuffer* pCmdBuffer = nullptr) -> VulkanDescriptorSet;

	protected:
		/// @brief Prepare GPU data before writing (e.g. upload UBO). No-op by default.
		virtual void PrepareGpuData(VulkanCommandBuffer* pCmdBuffer);

		/// @brief Write bindings to the descriptor writer. Pure virtual.
		virtual void WriteBindings(VulkanDescriptorWriter& writer) const = 0;

		/// @brief Compute a hash representing the current binding resources (for caching).
		virtual auto ComputeBindingHash() const -> size_t = 0;
	};
} // namespace Ailurus
