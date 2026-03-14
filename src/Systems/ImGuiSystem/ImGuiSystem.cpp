#include <cstdint>
#include <SDL3/SDL.h>
#include <backends/imgui_impl_sdl3.h>
#include "ImGuiVulkanBackEnd.h"
#include "Ailurus/Application.h"
#include "Ailurus/PlatformDefine.h"
#include "Ailurus/Systems/ImGuiSystem/ImGuiSystem.h"
#include "Ailurus/Systems/ImGuiSystem/Theme/MaterialTheme.h"
#include "Ailurus/Systems/ImGuiSystem/Font/JetBrainsMonoRegular.h"
#include "Ailurus/Systems/ImGuiSystem/Font/SansProBold.h"
#include "Ailurus/Systems/ImGuiSystem/Font/SansProRegular.h"
#include "VulkanContext/CommandBuffer/VulkanCommandBuffer.h"

namespace Ailurus
{
	namespace
	{
		void SyncMainViewportPlatformHandles()
		{
			SDL_Window* window = static_cast<SDL_Window*>(Application::GetSDLWindowPtr());
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			if (window == nullptr || viewport == nullptr)
				return;

			// ImGui_ImplVulkan_Shutdown() destroys platform windows during swapchain rebuilds,
			// which clears the main viewport's SDL registration even in single-window mode.
			viewport->PlatformHandle = reinterpret_cast<void*>(static_cast<intptr_t>(SDL_GetWindowID(window)));
			viewport->PlatformHandleRaw = nullptr;
#if AILURUS_PLATFORM_WINDOWS
			viewport->PlatformHandleRaw = SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
#elif AILURUS_PLATFORM_MAC
			viewport->PlatformHandleRaw = SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);
#endif
		}
	}

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
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		ImGui::StyleColorsLight();

		// Disable imgui ini file
		io.IniFilename = nullptr;
		io.LogFilename = nullptr;

		// Init frontend
		ImGui_ImplSDL3_InitForVulkan(static_cast<SDL_Window*>(Application::GetSDLWindowPtr()));

		// Init backend
		_pVkImpl->Init();
		SyncMainViewportPlatformHandles();

		LoadDefaultFonts();
		MaterialTheme::ApplyTheme(MaterialTheme::LightScheme());
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
		SyncMainViewportPlatformHandles();
		ImGui_ImplSDL3_ProcessEvent(sdlEvent);
	}

	void ImGuiSystem::NewFrame()
	{
		SyncMainViewportPlatformHandles();
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

	ImGuiID ImGuiSystem::BeginDockSpace(const char* name, ImGuiDockNodeFlags flags)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking
			| ImGuiWindowFlags_NoTitleBar
			| ImGuiWindowFlags_NoCollapse
			| ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoBringToFrontOnFocus
			| ImGuiWindowFlags_NoNavFocus;

		if ((flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0)
		{
			windowFlags |= ImGuiWindowFlags_NoBackground;
			ImGui::SetNextWindowBgAlpha(0.0f);
		}

		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin(name, nullptr, windowFlags);
		ImGui::PopStyleVar(3);

		const ImGuiID dockspaceId = ImGui::GetID(name);
		ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), flags);
		return dockspaceId;
	}

	void ImGuiSystem::EndDockSpace()
	{
		ImGui::End();
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

	ImFont* ImGuiSystem::GetFont(FontType type) const
	{
		switch (type)
		{
		case FontType::SemiBold:
			return _fontSemiBold != nullptr ? _fontSemiBold : _fontRegular;
		case FontType::Mono:
			return _fontMono != nullptr ? _fontMono : _fontRegular;
		case FontType::Icon:
			return _fontIcon != nullptr ? _fontIcon : _fontRegular;
		case FontType::Regular:
		default:
			return _fontRegular;
		}
	}

	void ImGuiSystem::LoadDefaultFonts()
	{
		ImGuiIO& io = ImGui::GetIO();
		io.Fonts->Clear();

		const ImWchar* glyphRanges = io.Fonts->GetGlyphRangesDefault();
		_fontRegular = CreateImGuiFont(SourceSansProRegular.data(), static_cast<int>(SourceSansProRegular.size()),
			static_cast<int>(DEFAULT_FONT_SIZE), false, glyphRanges);
		_fontSemiBold = CreateImGuiFont(SourceSansProBold.data(), static_cast<int>(SourceSansProBold.size()),
			static_cast<int>(DEFAULT_FONT_SIZE), false, glyphRanges);
		_fontMono = CreateImGuiFont(SourceJetBrainsMonoRegular.data(), static_cast<int>(SourceJetBrainsMonoRegular.size()),
			static_cast<int>(DEFAULT_FONT_SIZE - 1.0f), false, glyphRanges);
		_fontIcon = _fontRegular;
		io.FontDefault = _fontRegular;
	}
} // namespace Ailurus
