#include "Ailurus/Platform/Windows/Window/Service/ImGuiService/ImGuiService.h"

#if AILURUS_PLATFORM_WINDOWS

#include <imgui/imgui_internal.h>
#include <imgui/backends/imgui_impl_win32.h>
#include "Ailurus/Platform/Windows/Window/Window.h"
#include "Ailurus/Platform/Windows/WindowsDefine.h"
#include "Ailurus/Platform/Windows/Window/Service/ImGuiService/Theme/Spectrum.h"
#include "Ailurus/Platform/Windows/Window/Service/ImGuiService/Font/SansProRegular.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Ailurus
{
    ImGuiService::ImGuiService(Window* pWindow)
        : Service(pWindow)
    {
        IMGUI_CHECKVERSION();

        // ImGui context
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Light style
        ImGui::StyleColorsLight();

        // Disable imgui ini file
        io.IniFilename = nullptr;
        io.LogFilename = nullptr;

        // Init front end
        ImGui_ImplWin32_Init(pWindow->GetWindowHandle<HWND>());

        // Init theme
        float dpiScale =  GetDpiScale();

        // Font
        io.Fonts->Clear();
        io.FontDefault = CreateImGuiFont(
            SourceSansProRegular.data(),
            SourceSansProRegular.size(),
            GetDefaultFontSize(),
            false);

        // Style
        Spectrum::LoadStyle(false);

        // Dpi scale
        ImGui::GetStyle().ScaleAllSizes(dpiScale);
    }

    ImGuiService::~ImGuiService()
    {
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiService::ProcessWinMessage(void* hWnd, uint32_t msg, void* wPara, void* lPara)
    {
        ImGui_ImplWin32_WndProcHandler(static_cast<HWND>(hWnd), msg,
            reinterpret_cast<LPARAM>(wPara), reinterpret_cast<WPARAM>(lPara));
    }

    void ImGuiService::BeforeTick()
    {
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiService::AfterTick()
    {
        ImGui::Render();
    }

    void ImGuiService::OnCursorVisibleChanged(bool isVisible)
    {
        auto pCurrentContext = ImGui::GetCurrentContext();
        if (pCurrentContext != nullptr)
        {
            ImGuiIO& io = pCurrentContext->IO;
            io.ConfigFlags |= isVisible ? !ImGuiConfigFlags_NoMouseCursorChange : ImGuiConfigFlags_NoMouseCursorChange;
        }
    }

    float ImGuiService::GetDpiScale()
    {
        return ImGui_ImplWin32_GetDpiScaleForHwnd(_pWindow->GetWindowHandle<HWND>());
    }

    int ImGuiService::GetDefaultFontSize()
    {
        return 16 * GetDpiScale();
    }

    ImFont* ImGuiService::CreateImGuiFont(void* fontData, int fontDataSize, int fontSize, bool transferDataOwnership, const ImWchar* glyphRanges)
    {
        HWND hWnd = _pWindow->GetWindowHandle<HWND>();
        float dpiScale = ImGui_ImplWin32_GetDpiScaleForHwnd(hWnd);

        ImFontConfig tempConfig;
        if (!transferDataOwnership)
            tempConfig.FontDataOwnedByAtlas = false;

        if (fontSize <= 0)
            fontSize = GetDefaultFontSize();

        auto pFonts = ImGui::GetIO().Fonts;
        return pFonts->AddFontFromMemoryTTF(
                fontData,
                fontDataSize,
                dpiScale * fontSize,
                &tempConfig,
                glyphRanges);
    }

    ImFont* ImGuiService::CreateImGuiFont(const std::string& ttfPath, int fontSize, const ImWchar* glyphRanges)
    {
        HWND hWnd = _pWindow->GetWindowHandle<HWND>();
        float dpiScale = ImGui_ImplWin32_GetDpiScaleForHwnd(hWnd);

        if (fontSize <= 0)
            fontSize = GetDefaultFontSize();

        auto pFonts = ImGui::GetIO().Fonts;
        return pFonts->AddFontFromFileTTF(
                ttfPath.c_str(),
                dpiScale * fontSize,
                nullptr,
                glyphRanges);
    }

}

#endif
