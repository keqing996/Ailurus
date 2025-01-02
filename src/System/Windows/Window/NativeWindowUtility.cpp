#include "NativeWindowUtility.h"

#if AILURUS_PLATFORM_WINDOWS

#include "Ailurus/Utility/ScopeGuard.h"

namespace Ailurus
{
    bool NativeWindowUtility::FixProcessDpi()
    {
        // Try SetProcessDpiAwareness first
        if (FixProcessDpiBySetProcessDpiAwareness())
            return true;

        // Fall back to SetProcessDPIAware
        if (FixProcessDpiBySetProcessDPIAware())
            return true;

        return false;
    }

    LRESULT NativeWindowUtility::DefaultWndProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (message == WM_CREATE)
        {
            LONG_PTR window = reinterpret_cast<LONG_PTR>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
            ::SetWindowLongPtrW(handle, GWLP_USERDATA, window);
        }

        Window* pWindow = handle ? reinterpret_cast<Window*>(::GetWindowLongPtrW(handle, GWLP_USERDATA)) : nullptr;
        if (pWindow && pWindow->IsWindowValid())
            return pWindow->WindowEventProcess(message, reinterpret_cast<void*>(wParam), reinterpret_cast<void*>(lParam));

        return ::DefWindowProcW(handle, message, wParam, lParam);
    }

    std::pair<int, int> NativeWindowUtility::CalculateAdjustWindowSize(int width, int height, DWORD dwStyle)
    {
        RECT rectangle = {0, 0, width, height};

        ::AdjustWindowRect(&rectangle, dwStyle, false);

        return { rectangle.right - rectangle.left, rectangle.bottom - rectangle.top };
    }

    bool NativeWindowUtility::FixProcessDpiBySetProcessDpiAwareness()
    {
        HINSTANCE pShcodeDll = ::LoadLibraryW(L"Shcore.dll");

        if (pShcodeDll == nullptr)
            return false;

        ScopeGuard pShcodeDllReleaseGuard = [&pShcodeDll]()
        {
            ::FreeLibrary(pShcodeDll);
        };

        void* pFuncSetProcessDpiAwareness = reinterpret_cast<void*>(GetProcAddress(pShcodeDll, "SetProcessDpiAwareness"));
        if (pFuncSetProcessDpiAwareness == nullptr)
            return false;

        enum ProcessDpiAwareness
        {
            ProcessDpiUnaware         = 0,
            ProcessSystemDpiAware     = 1,
            ProcessPerMonitorDpiAware = 2
        };

        using SetProcessDpiAwarenessFuncType = HRESULT (WINAPI*)(ProcessDpiAwareness);

        auto setProcessDpiAwarenessFunc = reinterpret_cast<SetProcessDpiAwarenessFuncType>(pFuncSetProcessDpiAwareness);
        if (setProcessDpiAwarenessFunc(ProcessPerMonitorDpiAware) == E_INVALIDARG)
            return false;

        return true;
    }

    bool NativeWindowUtility::FixProcessDpiBySetProcessDPIAware()
    {
        HINSTANCE pUser32Dll = ::LoadLibraryW(L"user32.dll");

        if (pUser32Dll == nullptr)
            return false;

        ScopeGuard pShcodeDllReleaseGuard = [&pUser32Dll]()
        {
            ::FreeLibrary(pUser32Dll);
        };

        void* pFuncSetProcessDPIAware = reinterpret_cast<void*>(GetProcAddress(pUser32Dll, "SetProcessDPIAware"));
        if (pFuncSetProcessDPIAware == nullptr)
            return false;

        using SetProcessDPIAwareFuncType = BOOL (WINAPI*)(void);

        auto setProcessDpiAwareFunc = reinterpret_cast<SetProcessDPIAwareFuncType>(pFuncSetProcessDPIAware);
        if (!setProcessDpiAwareFunc())
            return false;

        return true;
    }
}

#endif
