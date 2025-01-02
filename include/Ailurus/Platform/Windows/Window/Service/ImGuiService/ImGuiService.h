#pragma once

#include "Ailurus/PlatformDefine.h"

#if PLATFORM_WINDOWS

#include <imgui/imgui.h>
#include <string>
#include "../Service.h"

namespace Ailurus
{
    class ImGuiService: public Service
    {
    public:
        explicit ImGuiService(Window* pWindow);
        ~ImGuiService() override;

    public:
        void ProcessWinMessage(void* hWnd, uint32_t msg, void* wPara, void* lPara) override;
        void BeforeTick() override;
        void AfterTick() override;

        void OnCursorVisibleChanged(bool isVisible) override;

    public:
        // Dpi
        float GetDpiScale();
        // Font
        int GetDefaultFontSize();
        ImFont* CreateImGuiFont(void* fontData, int fontDataSize, int fontSize, bool transferDataOwnership = true);
        ImFont* CreateImGuiFont(const std::string& ttfPath, int fontSize);
    };
}

#endif