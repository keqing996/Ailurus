#pragma once

#include <memory>
#include <imgui.h>

namespace Ailurus
{
    class ImGuiVulkanBackEnd;
    class VulkanCommandBuffer;
    class RenderPass;

    class ImGuiSystem
    {
    public:
        ImGuiSystem();
        ~ImGuiSystem();

    public:
    	void NewFrame();
    	void Render(VulkanCommandBuffer* pCommandBuffer);
    	ImFont* CreateImGuiFont(void* fontData, int fontDataSize, int fontSize = -1, bool transferDataOwnership = true, const ImWchar* glyphRanges = nullptr);
    	ImFont* CreateImGuiFont(const std::string& ttfPath, int fontSize = -1, const ImWchar* glyphRanges = nullptr);

    public:
    	static constexpr float DEFAULT_FONT_SIZE = 16;

    private:
        std::unique_ptr<ImGuiVulkanBackEnd> _pVkImpl;
    };
}