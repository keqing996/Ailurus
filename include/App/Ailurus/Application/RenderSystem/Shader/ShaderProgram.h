#pragma once

#include <memory>
#include "Ailurus/Application/RenderSystem/Shader/ShaderStage.h"
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"
#include "Uniform/UniformLayout.h"

namespace Ailurus
{
	class VulkanShaderProgram;

    class ShaderProgram : public NonCopyable, public NonMovable
    {
    public:
        ShaderProgram();
        ~ShaderProgram();

    public:
        void SetShader(const Shader* pShader);
    	void SetUniformLayout(const UniformLayout& uniformLayout);
    	const VulkanShaderProgram* GetImpl() const;


    private:
    	std::unique_ptr<VulkanShaderProgram> _pImpl;
        StageShaderArray _shaderStage;
    };
}