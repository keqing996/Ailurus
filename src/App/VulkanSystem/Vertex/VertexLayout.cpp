#include "VertexLayout.h"
#include "VulkanSystem/Helper/VulkanHelper.h"

namespace Ailurus
{
	VertexLayout::VertexLayout(const std::initializer_list<AttributeType>& attributes)
	{
		_attribute.insert(_attribute.end(), attributes.begin(), attributes.end());
		GetMeshVulkanAttributeDescription();
	}

	VertexLayout::VertexLayout(const std::vector<AttributeType>& attributes)
		: _attribute(attributes)
	{
		GetMeshVulkanAttributeDescription();
	}

	uint32_t VertexLayout::GetStride() const
	{
		uint32_t stride = 0;
		for (auto i = 0; i < _attribute.size(); i++)
			stride += VulkanHelper::SizeOf(_attribute[i]);
		return stride;
	}

	const std::vector<AttributeType>& VertexLayout::GetAttributes() const
	{
		return _attribute;
	}

	void VertexLayout::GetMeshVulkanAttributeDescription()
	{
		_vkAttributeDescriptions.clear();

		uint32_t offset = 0;
		for (auto i = 0; i < _attribute.size(); i++)
		{
			AttributeType attr = _attribute[i];

			vk::VertexInputAttributeDescription attributeDescriptions;
			attributeDescriptions.setBinding(0)
				.setLocation(i)
				.setFormat(VulkanHelper::GetFormat(attr))
				.setOffset(offset);

			_vkAttributeDescriptions.push_back(attributeDescriptions);

			offset += VulkanHelper::SizeOf(attr);
		}
	}

	const std::vector<vk::VertexInputAttributeDescription>& VertexLayout::GetVulkanAttributeDescription() const
	{
		return _vkAttributeDescriptions;
	}
} // namespace Ailurus
