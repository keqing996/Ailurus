#include "Ailurus/Application/Render/Shader/ShaderProgram.h"
#include "Ailurus/Application/Render/Shader/Shader.h"
#include "Ailurus/Application/Render/Shader/ShaderStage.h"
#include "Render/Shader/RhiShaderProgram.h"

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