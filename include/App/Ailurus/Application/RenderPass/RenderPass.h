#pragma once

#include <memory>
#include "Ailurus/Application/RenderPass/RenderPassType.h"
#include "Ailurus/Utility/NonCopyable.h"

namespace Ailurus
{
    class RHIRenderPass;

    class RenderPass: public NonCopyable
    {
    public:
        explicit RenderPass(RenderPassType passType);
        ~RenderPass() override;

    public:
        virtual RenderPassType GetRenderPassType() = 0;

    private:
        std::unique_ptr<RHIRenderPass> _pRHI = nullptr;
    };
}