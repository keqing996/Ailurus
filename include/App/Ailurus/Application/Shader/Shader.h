#pragma once

#include <string>
#include <memory>
#include "Ailurus/Utility/NonCopyable.h"

namespace Ailurus 
{
    class Shader: public NonCopyable
    {
    public:
        Shader();
        ~Shader() override;

        class RHIShader* GetImpl() const;

    private:
        std::string _name;
        std::unique_ptr<class RHIShader> _pImpl = nullptr;
    };
}