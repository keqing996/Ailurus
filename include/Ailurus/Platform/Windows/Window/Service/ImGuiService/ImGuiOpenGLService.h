#pragma once

#include "Ailurus/PlatformDefine.h"

#if PLATFORM_WINDOWS

#include <array>
#include "ImGuiService.h"

namespace Ailurus
{
    class ImGuiOpenGLService: public ImGuiService, public ServiceTypeGetter<ServiceType::ImGuiOpenGL>
    {
    public:
        explicit ImGuiOpenGLService(Window* pWindow);
        ~ImGuiOpenGLService() override;

    public:
        void BeforeTick() override;
        void AfterTick() override;

        void SetClearColor(float r, float g, float b, float a);

    private:
        std::array<float, 4> _clearColor = { 0.75f, 0.75f, 0.75f, 1.00f };
    };
}

#endif