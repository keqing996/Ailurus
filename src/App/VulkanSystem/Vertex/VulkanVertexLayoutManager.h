#pragma once

#include <unordered_map>
#include <memory>
#include <vulkan/vulkan.hpp>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>
#include "VulkanVertexLayout.h"

namespace Ailurus
{
	class VulkanVertexLayoutManager : public NonCopyable, public NonMovable
	{
	public:
		uint64_t CreateLayout(const std::vector<AttributeType>& attributes);
        VulkanVertexLayout* GetLayout(uint64_t layoutId) const;

	private:
		static uint64_t CompressedAttributes(const std::vector<AttributeType>& attributes);

	private:
		std::unordered_map<uint64_t, std::unique_ptr<VulkanVertexLayout>> _vertexLayoutsMap;
	};
} // namespace Ailurus