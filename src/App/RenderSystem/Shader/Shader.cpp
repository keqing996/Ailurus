#include "Ailurus/Application/Render/Shader/Shader.h"
#include "Render/Shader/RhiShader.h"

namespace Ailurus
{
	Shader::Shader(ShaderStage stage, const std::string& name)
		: _stage(stage)
		, _name(name)
		, _pImpl(std::make_unique<RhiShader>(name))
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

	const RhiShader* Shader::GetImpl() const
	{
		return _pImpl.get();
	}
} // namespace Ailurus
