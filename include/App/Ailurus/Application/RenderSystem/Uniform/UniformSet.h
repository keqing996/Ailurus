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

	REFLECTION_ENUM(UniformSetUsage,
		General,
		MaterialCustom);

	class UniformSet : public NonCopyable, public NonMovable
	{
	public:
		explicit UniformSet(uint32_t setId);
		explicit UniformSet(UniformSetUsage usage);
		~UniformSet();

	public:
		void AddBindingPoint(std::unique_ptr<UniformBindingPoint>&& pBindingPoint);
		void InitUniformBuffer();

		const UniformBindingPoint* GetBindingPoint(uint32_t bindingPoint) const;
		uint32_t GetSetId() const;

		void SetUniformValue(uint32_t bindingId, const std::string& access, const UniformValue& value);

	private:
		UniformBindingPoint* GetBindingPoint(uint32_t bindingPoint);

	private:
		uint32_t _setId;
		std::unordered_map<uint32_t, std::unique_ptr<UniformBindingPoint>> _bindingPoints;
		std::unique_ptr<VulkanUniformBuffer> _pUniformBuffer;
		std::unordered_map<uint32_t, uint32_t> _bindingPointOffsetInUniformBufferMap;
	};
} // namespace Ailurus