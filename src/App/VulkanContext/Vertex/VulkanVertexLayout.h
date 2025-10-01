#pragma once

#include "VulkanContext/VulkanPch.h"
#include <vector>
#include <initializer_list>
#include <Ailurus/Application/RenderSystem/Vertex/VertexAttributeType.h>

namespace Ailurus
{
	class VulkanVertexLayout
	{
	public:
		auto GetStride() const -> uint32_t;
		auto GetAttributes() const -> const std::vector<AttributeType>&;
		auto GetVulkanAttributeDescription() const -> const std::vector<vk::VertexInputAttributeDescription>&;

	private:
		friend class VulkanVertexLayoutManager;
		explicit VulkanVertexLayout(const std::initializer_list<AttributeType>& attributes);
		explicit VulkanVertexLayout(const std::vector<AttributeType>& attributes);

	private:
		void GetMeshVulkanAttributeDescription();

	private:
		std::vector<AttributeType> _attribute;
		std::vector<vk::VertexInputAttributeDescription> _vkAttributeDescriptions;
	};

} // namespace Ailurus
