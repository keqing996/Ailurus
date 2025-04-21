#pragma once

#include "Ailurus/Application/Shader/ShaderStage.h"
#include "Ailurus/Utility/NonCopyable.h"
#include "Uniform/UniformLayout.h"

namespace Ailurus
{
	class RHIShaderProgram;

    class ShaderProgram : public NonCopyable
    {
    public:
        ShaderProgram();
        ~ShaderProgram() override;

    public:
        void SetShader(const Shader* pShader);
    	void SetUniformLayout(const UniformLayout& uniformLayout);
    	const RHIShaderProgram* GetImpl() const;


    private:
    	std::unique_ptr<RHIShaderProgram> _pImpl;
        StageShaderArray _shaderStage;
    };
}