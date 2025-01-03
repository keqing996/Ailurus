#pragma once

#include "Ailurus/PlatformDefine.h"

#if AILURUS_PLATFORM_WINDOWS

#include "Ailurus/Platform/Windows/Window/Window.h"
#include "Ailurus/Platform/Windows/WindowsDefine.h"

namespace Ailurus
{
    class NativeWindowUtility
    {
    public:
        NativeWindowUtility() = delete;

    public:
        static bool FixProcessDpi();

        static LRESULT CALLBACK DefaultWndProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);

        static std::pair<int, int> CalculateAdjustWindowSize(int width, int height, DWORD dwStyle);

    private:
        static bool FixProcessDpiBySetProcessDpiAwareness();

        static bool FixProcessDpiBySetProcessDPIAware();
    };
}

#endif
