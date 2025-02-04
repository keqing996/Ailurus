
#include "Ailurus/Window/Service/Service.h"

#if AILURUS_FEAT_SUPPORT_WINDOW

#include <unordered_map>

namespace Ailurus
{
    static std::unordered_map<ServiceType, std::vector<ServiceType>> gServiceDependentMap = {
        { ServiceType::ImGuiOpenGL, { ServiceType::OpenGL } },
    };

    static std::unordered_map<ServiceType, std::vector<ServiceType>> gConflictMap = {
    };

    Service::Service(Window* pWindow)
        : _pWindow(pWindow)
    {
    }

    void Service::ProcessWinMessage(void* hWnd, uint32_t msg, void* wPara, void* lPara)
    {
    }

    void Service::BeforeTick()
    {
    }

    void Service::AfterTick()
    {
    }

    void Service::FinishLoop()
    {
    }

    void Service::OnWindowMoved(int x, int y)
    {
    }

    void Service::OnWindowPreDestroyed()
    {
    }

    void Service::OnWindowResized(int width, int height)
    {
    }

    void Service::OnWindowFocusChanged(bool isFocus)
    {
    }

    void Service::OnCursorEnteredOrLeave(bool isEnter)
    {
    }

    void Service::OnCursorVisibleChanged(bool isVisible)
    {
    }

    const std::vector<ServiceType>* Service::GetServiceDependent(ServiceType type)
    {
        auto itr = gServiceDependentMap.find(type);
        if (itr != gServiceDependentMap.end())
            return &itr->second;

        return nullptr;
    }

    const std::vector<ServiceType>* Service::GetServiceConflict(ServiceType type)
    {
        auto itr = gConflictMap.find(type);
        if (itr != gConflictMap.end())
            return &itr->second;

        return nullptr;
    }
}

#endif
