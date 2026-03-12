#include "VulkanPipelineEntry.h"

namespace Ailurus
{
	VulkanPipelineEntry::VulkanPipelineEntry(RenderPassType renderPassType,  uint32_t materialId, uint64_t layoutId)
		: renderPass(renderPassType)
        , materialAssetId(materialId)
        , vertexLayoutId(layoutId)
	{
	}

	std::size_t VulkanPipelineEntryHash::operator()(const VulkanPipelineEntry& entry) const
	{
		return std::hash<int>()(static_cast<int>(entry.renderPass)) 
            ^ std::hash<uint32_t>()(entry.materialAssetId)
            ^ std::hash<uint64_t>()(entry.vertexLayoutId);
	}

	bool VulkanPipelineEntryEqual::operator()(const VulkanPipelineEntry& lhs, const VulkanPipelineEntry& rhs) const
	{
		return lhs.renderPass == rhs.renderPass 
            && lhs.materialAssetId == rhs.materialAssetId
            && lhs.vertexLayoutId == rhs.vertexLayoutId ;
	}
} // namespace Ailurus