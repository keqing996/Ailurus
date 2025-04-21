#include "Ailurus/Application/Shader/ShaderProgram.h"
#include "Ailurus/Application/Shader/Shader.h"
#include "Ailurus/Application/Shader/ShaderStage.h"
#include "Rhi/Shader/RhiShaderProgram.h"

namespace Ailurus
{
    ShaderProgram::ShaderProgram()
		: _pImpl(std::make_unique<RhiShaderProgram>())
		, _shaderStage()
	{
	}

	ShaderProgram::~ShaderProgram()
    {
    }

    void ShaderProgram::SetShader(const Shader* pShader)
	{
		_shaderStage[pShader->GetStage()] = pShader;
	}

	void ShaderProgram::SetUniformLayout(const UniformLayout& uniformLayout)
	{
	}

	const RhiShaderProgram* ShaderProgram::GetImpl() const
	{
    	return _pImpl.get();
	}
} // namespace Ailurus