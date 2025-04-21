#pragma once

#include "Ailurus/Application/Shader/ShaderStage.h"
#include "Ailurus/Utility/NonCopyable.h"
#include "Uniform/UniformLayout.h"

namespace Ailurus
{
	class RhiShaderProgram;

    class ShaderProgram : public NonCopyable
    {
    public:
        ShaderProgram();
        ~ShaderProgram() override;

    public:
        void SetShader(const Shader* pShader);
    	void SetUniformLayout(const UniformLayout& uniformLayout);
    	const RhiShaderProgram* GetImpl() const;


    private:
    	std::unique_ptr<RhiShaderProgram> _pImpl;
        StageShaderArray _shaderStage;
    };
}