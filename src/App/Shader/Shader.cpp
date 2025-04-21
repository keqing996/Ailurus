#include "Ailurus/Application/Shader/Shader.h"
#include "Rhi/Shader/RHIShader.h"

namespace Ailurus
{
	Shader::Shader(ShaderStage stage, const std::string& name)
		: _stage(stage)
		, _name(name)
		, _pImpl(std::make_unique<RHIShader>(name))
	{
	}

	Shader::~Shader()
	{
	}

	ShaderStage Shader::GetStage() const
	{
		return _stage;
	}

	const std::string& Shader::GetName() const
	{
		return _name;
	}

	const RHIShader* Shader::GetImpl() const
	{
		return _pImpl.get();
	}
} // namespace Ailurus
