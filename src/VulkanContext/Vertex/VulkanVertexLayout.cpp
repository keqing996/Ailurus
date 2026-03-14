#include "VulkanVertexLayout.h"
#include "VulkanContext/Helper/VulkanHelper.h"

namespace Ailurus
{
	namespace
	{
		uint32_t GetAttributeLocation(AttributeType attribute)
		{
			switch (attribute)
			{
				case AttributeType::Position:
					return 0;
				case AttributeType::Normal:
					return 1;
				case AttributeType::TexCoord:
					return 2;
				case AttributeType::Tangent:
					return 3;
				case AttributeType::Bitangent:
					return 4;
				case AttributeType::Color:
					return 5;
			}

			return 0;
		}
	}

	VulkanVertexLayout::VulkanVertexLayout(const std::initializer_list<AttributeType>& attributes)
	{
		_attribute.insert(_attribute.end(), attributes.begin(), attributes.end());
		GetMeshVulkanAttributeDescription();
	}

	VulkanVertexLayout::VulkanVertexLayout(const std::vector<AttributeType>& attributes)
		: _attribute(attributes)
	{
		GetMeshVulkanAttributeDescription();
	}

	uint32_t VulkanVertexLayout::GetStride() const
	{
		return VulkanHelper::CalculateVertexLayoutStride(_attribute);
	}

	const std::vector<AttributeType>& VulkanVertexLayout::GetAttributes() const
	{
		return _attribute;
	}

	void VulkanVertexLayout::GetMeshVulkanAttributeDescription()
	{
		_vkAttributeDescriptions.clear();

		uint32_t offset = 0;
		for (auto i = 0; i < _attribute.size(); i++)
		{
			AttributeType attr = _attribute[i];

			vk::VertexInputAttributeDescription attributeDescriptions;
			attributeDescriptions.setBinding(0)
				.setLocation(GetAttributeLocation(attr))
				.setFormat(VulkanHelper::GetFormat(attr))
				.setOffset(offset);

			_vkAttributeDescriptions.push_back(attributeDescriptions);

			offset += VulkanHelper::SizeOf(attr);
		}
	}

	const std::vector<vk::VertexInputAttributeDescription>& VulkanVertexLayout::GetVulkanAttributeDescription() const
	{
		return _vkAttributeDescriptions;
	}
} // namespace Ailurus
