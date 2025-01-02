#pragma once

#include "Ailurus/PlatformDefine.h"

#if PLATFORM_WINDOWS

#include <vector>
#include <cstdint>

namespace Ailurus
{
    class Window;

    enum class ServiceType: int
    {
        Input,
        OpenGL,
        ImGuiOpenGL,
    };

    template<ServiceType Type>
    class ServiceTypeGetter
    {
    public:
        static ServiceType ServiceType()
        {
            return Type;
        }
    };

    class Service
    {
    public:
        virtual ~Service() = default;

    public:
        virtual void ProcessWinMessage(void* hWnd, uint32_t msg, void* wPara, void* lPara);
        virtual void BeforeTick();
        virtual void AfterTick();
        virtual void FinishLoop();

        virtual void OnWindowMoved(int x, int y);
        virtual void OnWindowPreDestroyed();
        virtual void OnWindowResized(int width, int height);
        virtual void OnWindowFocusChanged(bool isFocus);
        virtual void OnCursorEnteredOrLeave(bool isEnter);
        virtual void OnCursorVisibleChanged(bool isVisible);

    public:
        static const std::vector<ServiceType>* GetServiceDependent(ServiceType type);
        static const std::vector<ServiceType>* GetServiceConflict(ServiceType type);

    protected:
        explicit Service(Window* pWindow);

        Window* _pWindow;
    };

}

#endif
