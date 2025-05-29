#pragma once

#include <cstdint>
#include <unordered_map>
#include "Ailurus/Utility/EnumReflection.h"
#include "UniformBindingPoint.h"

namespace Ailurus
{
	REFLECTION_ENUM(UniformSetUsage,
		General,
		MaterialCustom);

	class UniformSet
	{
	public:
		explicit UniformSet(uint32_t setId);
		~UniformSet();

	public:
		void AddBindingPoint(std::unique_ptr<UniformBindingPoint>&& pBindingPoint);
		const UniformBindingPoint* GetBindingPoint(uint32_t bindingPoint) const;
		uint32_t GetSetId() const;

	private:
		uint32_t _setId;
		std::unordered_map<uint32_t, std::unique_ptr<UniformBindingPoint>> _bindingPoints;
	};
}