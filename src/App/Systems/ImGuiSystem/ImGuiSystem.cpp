#include <imgui.h>
#include <SDL3/SDL.h>
#include <backends/imgui_impl_sdl3.h>
#include "ImGuiVulkanBackEnd.h"
#include "Ailurus/Application/Application.h"
#include "Ailurus/Application/ImGuiSystem/ImGuiSystem.h"

namespace Ailurus
{
    ImGuiSystem::ImGuiSystem()
    {
        // Create backend
        _pVkImpl = std::make_unique<ImGuiVulkanBackEnd>();

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsLight();

        ImGui_ImplSDL3_InitForVulkan((SDL_Window*)Application::GetSDLWindowPtr());

        _pVkImpl->Init();
    }

    ImGuiSystem::~ImGuiSystem()
    {
        _pVkImpl->Shutdown();

        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();

        // Clear backend
        _pVkImpl.reset();
    }
}
