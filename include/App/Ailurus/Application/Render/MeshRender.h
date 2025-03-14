#pragma once

#include <memory>
#include "Render.h"
#include "InputAttribute.h"
#include "Ailurus/Application/Material/Material.h"

namespace Ailurus
{
    class MeshRenderImpl;

    class MeshRender: public Render
    {
    public:

    private:
        std::unique_ptr<MeshRenderImpl> _pImpl = nullptr;
    };
}