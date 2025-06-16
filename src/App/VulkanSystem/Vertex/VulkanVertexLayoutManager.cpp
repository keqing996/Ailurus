#include "VulkanVertexLayoutManager.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
	uint64_t VulkanVertexLayoutManager::CreateLayout(const std::vector<AttributeType>& attributes)
	{
        uint64_t layoutId = CompressedAttributes(attributes);

		const auto it = _vertexLayoutsMap.find(layoutId);
		if (it == _vertexLayoutsMap.end())
			_vertexLayoutsMap[layoutId] = std::unique_ptr<VulkanVertexLayout>(new VulkanVertexLayout(attributes));
		
        return layoutId;
	}

    VulkanVertexLayout* VulkanVertexLayoutManager::GetLayout(uint64_t layoutId) const
    {
        const auto it = _vertexLayoutsMap.find(layoutId);
        if (it != _vertexLayoutsMap.end())
            return it->second.get();
        
        return nullptr;
    }

	uint64_t VulkanVertexLayoutManager::CompressedAttributes(const std::vector<AttributeType>& attributes)
	{
		if (attributes.size() > 16)
		{
			Logger::LogError("Layout too large to encode into uint64_t (max 16 items).");
			return 0;
		}

		uint64_t encoded = 0;

		// Encoding each attribute (4 bits per attribute)
		for (auto i = 0; i < attributes.size(); i++)
		    encoded |= (static_cast<uint64_t>(attributes[i]) & 0xF) << (4 * i);

		return encoded;
	}
} // namespace Ailurus