#include <SDL3/SDL.h>
#include <backends/imgui_impl_sdl3.h>
#include "ImGuiVulkanBackEnd.h"
#include "Ailurus/Application/Application.h"
#include "Ailurus/Application/ImGuiSystem/ImGuiSystem.h"
#include "Ailurus/Application/ImGuiSystem/Theme/Spectrum.h"
#include "Ailurus/Application/ImGuiSystem/Font/SansProRegular.h"
#include "VulkanContext/CommandBuffer/VulkanCommandBuffer.h"

namespace Ailurus
{
	ImGuiSystem::ImGuiSystem()
	{
		IMGUI_CHECKVERSION();

		// Create backend
		_pVkImpl = std::make_unique<ImGuiVulkanBackEnd>();

		// Create the ImGui context
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

		// Light style
		ImGui::StyleColorsLight();

		// Disable imgui ini file
		io.IniFilename = nullptr;
		io.LogFilename = nullptr;

		// Init frontend
		ImGui_ImplSDL3_InitForVulkan(static_cast<SDL_Window*>(Application::GetSDLWindowPtr()));

		// Init backend
		_pVkImpl->Init();

		// Font
		io.Fonts->Clear();
		io.FontDefault = CreateImGuiFont(SourceSansProRegular.data(), SourceSansProRegular.size(),
			DEFAULT_FONT_SIZE, false);

		// Style
		Spectrum::LoadStyle(false);
	}

	ImGuiSystem::~ImGuiSystem()
	{
		// Shutdown backend
		_pVkImpl->Shutdown();

		// Shutdown frontend
		ImGui_ImplSDL3_Shutdown();

		// Shutdown ImGUI context
		ImGui::DestroyContext();

		// Clear backend
		_pVkImpl.reset();
	}

	void ImGuiSystem::HandleEvent(const void* evt)
	{
		const SDL_Event* sdlEvent = static_cast<const SDL_Event*>(evt);
		ImGui_ImplSDL3_ProcessEvent(sdlEvent);
	}

	void ImGuiSystem::NewFrame()
	{
		_pVkImpl->NewFrame();
		ImGui_ImplSDL3_NewFrame();

		// Manually update display size and scale for HiDPI displays like Retina
		ImGuiIO& io = ImGui::GetIO();
		int window_width, window_height;
		int display_width, display_height;
		SDL_GetWindowSize(static_cast<SDL_Window*>(Application::GetSDLWindowPtr()), &window_width, &window_height);
		SDL_GetWindowSizeInPixels(static_cast<SDL_Window*>(Application::GetSDLWindowPtr()), &display_width, &display_height);
		io.DisplaySize = ImVec2((float)window_width, (float)window_height);
		if (window_width > 0 && window_height > 0)
			io.DisplayFramebufferScale = ImVec2((float)display_width / window_width, (float)display_height / window_height);

		ImGui::NewFrame();
	}

	void ImGuiSystem::Render(VulkanCommandBuffer* pCommandBuffer)
	{
		ImGui::Render();

		_pVkImpl->Render(pCommandBuffer);
	}

	ImFont* ImGuiSystem::CreateImGuiFont(void* fontData, int fontDataSize, int fontSize, bool transferDataOwnership, const ImWchar* glyphRanges)
	{
		ImFontConfig tempConfig;
		if (!transferDataOwnership)
			tempConfig.FontDataOwnedByAtlas = false;

		if (fontSize <= 0)
			fontSize = DEFAULT_FONT_SIZE;

		fontSize *= Application::GetWindowScale();

		const auto pFonts = ImGui::GetIO().Fonts;
		return pFonts->AddFontFromMemoryTTF(
			fontData,
			fontDataSize,
			fontSize,
			&tempConfig,
			glyphRanges);
	}

	ImFont* ImGuiSystem::CreateImGuiFont(const std::string& ttfPath, int fontSize, const ImWchar* glyphRanges)
	{
		if (fontSize <= 0)
			fontSize = DEFAULT_FONT_SIZE;

		fontSize *= Application::GetWindowScale();

		const auto pFonts = ImGui::GetIO().Fonts;
		return pFonts->AddFontFromFileTTF(
			ttfPath.c_str(),
			fontSize,
			nullptr,
			glyphRanges);
	}
} // namespace Ailurus
