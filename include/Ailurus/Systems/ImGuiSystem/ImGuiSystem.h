#pragma once

#include <memory>
#include <string>
#include <imgui.h>

namespace Ailurus
{
    class ImGuiVulkanBackEnd;
    class VulkanCommandBuffer;

    enum class FontType
    {
        Regular,
        SemiBold,
        Mono,
        Icon,
    };

    class ImGuiSystem
    {
    public:
        ImGuiSystem();
        ~ImGuiSystem();

    public:
    	void HandleEvent(const void* evt);
    	void NewFrame();
    	void Render(VulkanCommandBuffer* pCommandBuffer);
        ImGuiID BeginDockSpace(const char* name = "MainDockSpace", ImGuiDockNodeFlags flags = ImGuiDockNodeFlags_None);
        void EndDockSpace();
    	ImFont* CreateImGuiFont(void* fontData, int fontDataSize, int fontSize = -1, bool transferDataOwnership = true, const ImWchar* glyphRanges = nullptr);
    	ImFont* CreateImGuiFont(const std::string& ttfPath, int fontSize = -1, const ImWchar* glyphRanges = nullptr);
        ImFont* GetFont(FontType type) const;
        constexpr float GetDefaultFontSize() const { return DEFAULT_FONT_SIZE; }

    public:
        static constexpr float DEFAULT_FONT_SIZE = 16.0f;

    private:
        void LoadDefaultFonts();

    private:
        std::unique_ptr<ImGuiVulkanBackEnd> _pVkImpl;
        ImFont* _fontRegular = nullptr;
        ImFont* _fontSemiBold = nullptr;
        ImFont* _fontMono = nullptr;
        ImFont* _fontIcon = nullptr;
    };
}