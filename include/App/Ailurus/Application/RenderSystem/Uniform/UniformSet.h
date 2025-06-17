#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include "Ailurus/Utility/EnumReflection.h"
#include "UniformBindingPoint.h"
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"

namespace Ailurus
{
	class VulkanUniformBuffer;
	class VulkanDescriptorSetLayout;

	REFLECTION_ENUM(UniformSetUsage,
		General,
		MaterialCustom);

	class UniformSet : public NonCopyable, public NonMovable
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
		void InitDescriptorSetLayout();

		// Getter
		auto GetAllBindingPoints() const -> const BindingPointMap&;
		auto GetBindingPoint(uint32_t bindingPoint) const -> const UniformBindingPoint*;
		auto GetBindingPointOffsetInUniformBuffer(uint32_t bindingPoint) const -> uint32_t;
		auto GetSetId() const -> uint32_t;
		auto GetUniformBufferSize() const -> uint32_t;
		auto GetDescriptorSetLayout() const -> VulkanDescriptorSetLayout*;

	private:
		auto GetBindingPoint(uint32_t bindingPoint) -> UniformBindingPoint*;

	private:
		// Set id in shader
		uint32_t _setId;

		// Binding point
		BindingPointMap _bindingPoints;

		// Uniform buffer offset map
		uint32_t _uniformBufferSize = 0;
		std::unordered_map<uint32_t, uint32_t> _bindingPointOffsetInUniformBufferMap;

		// Descriptor set layout
		std::unique_ptr<VulkanDescriptorSetLayout> _pDescriptorSetLayout;
	};
} // namespace Ailurus