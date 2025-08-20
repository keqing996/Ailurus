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

		// Dpi scale
		// ImGui::GetStyle().ScaleAllSizes(dpiScale);
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

	void ImGuiSystem::NewFrame()
	{
		_pVkImpl->NewFrame();
		ImGui_ImplSDL3_NewFrame();
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

		const auto pFonts = ImGui::GetIO().Fonts;
		return pFonts->AddFontFromFileTTF(
			ttfPath.c_str(),
			fontSize,
			nullptr,
			glyphRanges);
	}
} // namespace Ailurus
