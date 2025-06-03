#pragma once

#include <cstdint>
#include <memory>
#include "UniformVariable.h"
#include "Ailurus/Application/RenderSystem/Shader/ShaderStage.h"
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"

namespace Ailurus
{
	class UniformBindingPoint: public NonCopyable, public NonMovable
	{
	public:
		UniformBindingPoint(uint32_t bindingPoint, std::unique_ptr<UniformVariable>&& pUniform);
		~UniformBindingPoint();

	public:
		void AddUsingStage(ShaderStage stage);
		const std::vector<ShaderStage>& GetUsingStages() const;
		uint32_t GetBindingPoint() const;
		const UniformVariable* GetUniform() const;
		UniformVariable* GetUniform();

	private:
		uint32_t _bindingPoint = 0;
		std::vector<ShaderStage> _usingStages;
		std::unique_ptr<UniformVariable> _pUniformVariable;
	};
} // namespace Ailurus