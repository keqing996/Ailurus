#include <Ailurus/Application/RenderSystem/Uniform/UniformBindingPoint.h>

namespace Ailurus
{
	UniformBindingPoint::UniformBindingPoint(uint32_t bindingPoint, std::unique_ptr<UniformVariable>&& pUniform)
		: _bindingPoint(bindingPoint)
		, _pUniformVariable(std::move(pUniform))
	{
	}

	UniformBindingPoint::~UniformBindingPoint() = default;

	void UniformBindingPoint::AddUsingStage(ShaderStage stage)
	{
		_usingStages.push_back(stage);
	}

	const std::vector<ShaderStage>& UniformBindingPoint::GetUsingStages() const
	{
		return _usingStages;
	}

	uint32_t UniformBindingPoint::GetBindingPoint() const
	{
		return _bindingPoint;
	}

	const UniformVariable* UniformBindingPoint::GetUniform() const
	{
		return _pUniformVariable.get();
	}

	UniformVariable* UniformBindingPoint::GetUniform()
	{
		return _pUniformVariable.get();
	}

} // namespace Ailurus