#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>
#include "Ailurus/Utility/EnumReflection.h"
#include "UniformBindingPoint.h"
#include "Ailurus/Systems/RenderSystem/Descriptor/DescriptorSetSchema.h"

namespace Ailurus
{
	class VulkanUniformBuffer;
	class VulkanDescriptorSetLayout;
	struct TextureBindingInfo;

	REFLECTION_ENUM(UniformSetUsage,
		General,
		MaterialCustom);

	class UniformSet : public DescriptorSetSchema
	{
		using BindingPointMap = std::unordered_map<uint32_t, std::unique_ptr<UniformBindingPoint>>;
	public:
		explicit UniformSet(uint32_t setId);
		explicit UniformSet(UniformSetUsage usage);
		~UniformSet();

	public:
		// When reading from json
		void AddBindingPoint(std::unique_ptr<UniformBindingPoint>&& pBindingPoint);
		void InitUniformBufferInfo();
		void InitDescriptorSetLayout(); // For uniform sets without textures
		void InitDescriptorSetLayout(const std::vector<TextureBindingInfo>& textureBindings); // For uniform sets with textures

		// Getter
		auto GetAllBindingPoints() const -> const BindingPointMap&;
		auto GetBindingPoint(uint32_t bindingPoint) const -> const UniformBindingPoint*;
		auto GetBindingPointOffsetInUniformBuffer(uint32_t bindingPoint) const -> uint32_t;
		auto GetSetId() const -> uint32_t;
		auto GetUniformBufferSize() const -> uint32_t;

	private:
		// Set id in shader
		uint32_t _setId;

		// Binding point
		BindingPointMap _bindingPoints;

		// Uniform buffer offset map
		uint32_t _uniformBufferSize = 0;
		std::unordered_map<uint32_t, uint32_t> _bindingPointOffsetInUniformBufferMap;
	};
} // namespace Ailurus