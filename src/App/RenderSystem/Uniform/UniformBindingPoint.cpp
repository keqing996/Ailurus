#include <Ailurus/Application/RenderSystem/Uniform/UniformBindingPoint.h>

namespace Ailurus
{
	UniformBindingPoint::UniformBindingPoint(uint32_t bindingPoint, std::unique_ptr<MaterialUniformVariable>&& pUniform)
		: _bindingPoint(bindingPoint)
		, _uniform(std::move(pUniform))
	{
		std::fill(_usingStages.begin(), _usingStages.end(), false);
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

	const MaterialUniformVariable* UniformBindingPoint::GetUniform() const
	{
		return _uniform.get();
	}

	MaterialUniformVariable* UniformBindingPoint::GetUniform()
	{
		return _uniform.get();
	}

} // namespace Ailurus