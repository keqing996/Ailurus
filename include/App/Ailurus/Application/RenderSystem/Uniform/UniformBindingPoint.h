#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include "UniformVariable.h"
#include "Ailurus/Application/RenderSystem/Shader/ShaderStage.h"
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"

namespace Ailurus
{
	class UniformBindingPoint: public NonCopyable, public NonMovable
	{
	public:
		UniformBindingPoint(uint32_t bindingPoint, const std::vector<ShaderStage>& shaderStage, 
			const std::string& name, std::unique_ptr<UniformVariable>&& pUniform);
		~UniformBindingPoint();

	public:
		const std::vector<ShaderStage>& GetUsingStages() const;
		uint32_t GetBindingPoint() const;
		const UniformVariable* GetUniform() const;
		UniformVariable* GetUniform();
		uint32_t GetTotalSize() const;
		std::optional<uint32_t> GetAccessOffset(const std::string& accessName) const;

	private:
		uint32_t _bindingPoint = 0;
		std::vector<ShaderStage> _usingStages;
		std::string _bindingPointName;
		std::unique_ptr<UniformVariable> _pUniformVariable;

		uint32_t _totalSize = 0;
		std::unordered_map<std::string, uint32_t> _accessNameToBufferOffsetMap;
	};
} // namespace Ailurus