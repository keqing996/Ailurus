#pragma once

#include <memory>
#include <unordered_map>
#include <vulkan/vulkan.hpp>
#include "UniformSet.h"
#include "UniformAccess.h"
#include "VulkanContext/Descriptor/VulkanDescriptorSet.h"

namespace Ailurus
{
	class VulkanUniformBuffer;
	class VulkanCommandBuffer;
	class MaterialInstance;

	class UniformSetMemory
	{
	public:
		UniformSetMemory(const UniformSet* pTargetUniformSet);
		~UniformSetMemory();

	public:
		auto GetUniformValueMap() const -> const UniformValueMap&;
		auto SetUniformValue(uint32_t bindingId, const std::string& access, const UniformValue& value) -> void;
		auto SetUniformValue(const UniformAccess& entry, const UniformValue& value) -> void;
		auto UpdateToDescriptorSet(VulkanCommandBuffer* pCommandBuffer, VulkanDescriptorSet descriptorSet, 
			const MaterialInstance* pMaterialInstance = nullptr) const -> void;

	private:
		auto TransitionDataToGpu(VulkanCommandBuffer* pCommandBuffer) const -> void;

	private:
		// Target uniform set
		const UniformSet* _pTargetUniformSet;

		// Uniform access -> uniform value
		UniformValueMap _uniformValueMap;

		// Uniform buffer memory
		std::unique_ptr<VulkanUniformBuffer> _pUniformBuffer;
	};
} // namespace Ailurus