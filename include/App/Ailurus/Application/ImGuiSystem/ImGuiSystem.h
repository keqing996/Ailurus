#pragma once

#include <memory>
#include <functional>
#include "Ailurus/PlatformDefine.h"

namespace Ailurus
{
    class ImGuiVulkanBackEnd;

    class ImGuiSystem
    {
    public:
        ImGuiSystem();
        ~ImGuiSystem();

    public:
    	void NewFrame();
    	void EndFrame();
    	ImFont* CreateImGuiFont(void* fontData, int fontDataSize, int fontSize = -1, bool transferDataOwnership = true, const ImWchar* glyphRanges = nullptr);
    	ImFont* CreateImGuiFont(const std::string& ttfPath, int fontSize = -1, const ImWchar* glyphRanges = nullptr);

    public:
    	static constexpr float DEFAULT_FONT_SIZE = 16;

    private:
        std::unique_ptr<ImGuiVulkanBackEnd> _pVkImpl;
    };
}