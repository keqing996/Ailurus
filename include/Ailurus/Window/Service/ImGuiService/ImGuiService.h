#pragma once

#include "Ailurus/PlatformDefine.h"

#if AILURUS_FEAT_SUPPORT_WINDOW

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
        ImFont* CreateImGuiFont(void* fontData, int fontDataSize, int fontSize = -1, bool transferDataOwnership = true, const ImWchar* glyphRanges = nullptr);
        ImFont* CreateImGuiFont(const std::string& ttfPath, int fontSize = -1, const ImWchar* glyphRanges = nullptr);
    };
}

#endif