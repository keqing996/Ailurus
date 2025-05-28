#include <../../../../include/App/Ailurus/Application/RenderSystem/Uniform/UniformVariable.h>

namespace Ailurus
{
	MaterialUniformVariable::MaterialUniformVariable(
		const std::string& uniformBlockName,
		const std::string& uniformValueName)
		: _uniformBlockName(uniformBlockName)
		, _uniformValueName(uniformValueName)
	{
	}

	auto MaterialUniformVariable::GetUniformBlockName() const -> const std::string&
	{
		return _uniformBlockName;
	}

	auto MaterialUniformVariable::GetUniformValueName() const -> const std::string&
	{
		return _uniformValueName;
	}
} // namespace Ailurus
