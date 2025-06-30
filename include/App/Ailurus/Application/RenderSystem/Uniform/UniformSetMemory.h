#pragma once

#include <memory>
#include "UniformSet.h"
#include "UniformAccess.h"

namespace Ailurus
{
    class VulkanUniformBuffer;

    class UniformSetMemory 
    {
    public:
		UniformSetMemory(const UniformSet* pTargetUniformSet);
		~UniformSetMemory();

    public:
		auto GetUniformValueMap() const -> const UniformValueMap&;
		auto SetUniformValue(uint32_t bindingId, const std::string& access, const UniformValue& value) -> void;
		auto SetUniformValue(const UniformAccess& entry, const UniformValue& value) -> void;
		auto TransitionDataToGpu() const -> void;

	private:
        // Target uniform set
        const UniformSet* _pTargetUniformSet;

        // Uniform access -> uniform value
		UniformValueMap _uniformValueMap;

        // Uniform buffer memory
        std::unique_ptr<VulkanUniformBuffer> _pUniformBuffer;
    };
}