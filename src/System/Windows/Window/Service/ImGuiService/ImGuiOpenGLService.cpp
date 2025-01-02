#include "Ailurus/Platform/Windows/Window/Service/ImGuiService/ImGuiOpenGLService.h"

#if AILURUS_PLATFORM_WINDOWS

#include <glad/gl.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include "Ailurus/Platform/Windows/Window/Window.h"

namespace Ailurus
{
    ImGuiOpenGLService::ImGuiOpenGLService(Window* pWindow)
        : ImGuiService(pWindow)
    {
        ImGui_ImplOpenGL3_Init();
    }

    ImGuiOpenGLService::~ImGuiOpenGLService()
    {
        ImGui_ImplOpenGL3_Shutdown();
    }

    void ImGuiOpenGLService::BeforeTick()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGuiService::BeforeTick();
    }

    void ImGuiOpenGLService::AfterTick()
    {
        ImGuiService::AfterTick();

        auto [width, height] = _pWindow->GetSize();
        ::glViewport(0, 0, width, height);
        ::glClearColor(_clearColor[0], _clearColor[1], _clearColor[2], _clearColor[3]);
        ::glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void ImGuiOpenGLService::SetClearColor(float r, float g, float b, float a)
    {
        _clearColor[0] = r;
        _clearColor[1] = g;
        _clearColor[2] = b;
        _clearColor[3] = a;
    }
}

#endif
