#include "Ailurus/Application/RenderSystem/Shader/ShaderProgram.h"
#include "Ailurus/Application/RenderSystem/Shader/Shader.h"
#include "Ailurus/Application/RenderSystem/Shader/ShaderStage.h"
#include "RenderSystem/Shader/VulkanShaderProgram.h"

namespace Ailurus
{
    ShaderProgram::ShaderProgram()
		: _pImpl(std::make_unique<VulkanShaderProgram>())
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

	const VulkanShaderProgram* ShaderProgram::GetImpl() const
	{
    	return _pImpl.get();
	}
} // namespace Ailurus