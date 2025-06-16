#pragma once

#include <vector>
#include <initializer_list>
#include <vulkan/vulkan.hpp>
#include "VertexAttributeType.h"

namespace Ailurus
{
	class VertexLayout
	{
	public:
		explicit VertexLayout(const std::initializer_list<AttributeType>& attributes);
		explicit VertexLayout(const std::vector<AttributeType>& attributes);

	public:
		auto GetStride() const -> uint32_t;
		auto GetAttributes() const -> const std::vector<AttributeType>&;
		auto GetVulkanAttributeDescription() const -> const std::vector<vk::VertexInputAttributeDescription>&;

	private:
		void GetMeshVulkanAttributeDescription();

	private:
		std::vector<AttributeType> _attribute;
		std::vector<vk::VertexInputAttributeDescription> _vkAttributeDescriptions;
	};

} // namespace Ailurus
