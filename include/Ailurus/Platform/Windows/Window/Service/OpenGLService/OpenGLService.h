#pragma once

#include "Ailurus/PlatformDefine.h"

#if AILURUS_PLATFORM_WINDOWS

#include "../Service.h"

namespace Ailurus
{
    class OpenGLService: public Service, public ServiceTypeGetter<ServiceType::OpenGL>
    {
    public:
        explicit OpenGLService(Window* pWindow);
        ~OpenGLService() override;

    public:
        void FinishLoop() override;
        void SetVSync(bool enable);

    private:
        bool _enableVSync = true;
        void* _hDeviceHandle = nullptr;
        void* _hGLContext = nullptr;
    };
}

#endif