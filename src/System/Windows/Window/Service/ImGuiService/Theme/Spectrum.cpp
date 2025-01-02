#include "Ailurus/Platform/Windows/Window/Service/ImGuiService/Theme/Spectrum.h"

#if PLATFORM_WINDOWS

#include <imgui/imgui.h>

namespace Ailurus::Spectrum
{
    namespace Utility
    {
        static unsigned int Color(const unsigned int c)
        {
            // add alpha.
            // also swap red and blue channel for some reason.
            // todo: figure out why, and fix it.
            const short a = 0xFF;
            const short r = (c >> 16) & 0xFF;
            const short g = (c >> 8) & 0xFF;
            const short b = (c >> 0) & 0xFF;
            return (a << 24)
                   | (r << 0)
                   | (g << 8)
                   | (b << 16);
        }
    }

    unsigned int StaticColor::NONE = 0x00000000; // transparent
    unsigned int StaticColor::WHITE = Utility::Color(0xFFFFFF);
    unsigned int StaticColor::BLACK = Utility::Color(0x000000);
    unsigned int StaticColor::GRAY200 = Utility::Color(0xF4F4F4);
    unsigned int StaticColor::GRAY300 = Utility::Color(0xEAEAEA);
    unsigned int StaticColor::GRAY400 = Utility::Color(0xD3D3D3);
    unsigned int StaticColor::GRAY500 = Utility::Color(0xBCBCBC);
    unsigned int StaticColor::GRAY600 = Utility::Color(0x959595);
    unsigned int StaticColor::GRAY700 = Utility::Color(0x767676);
    unsigned int StaticColor::GRAY800 = Utility::Color(0x505050);
    unsigned int StaticColor::GRAY900 = Utility::Color(0x323232);
    unsigned int StaticColor::BLUE400 = Utility::Color(0x378EF0);
    unsigned int StaticColor::BLUE500 = Utility::Color(0x2680EB);
    unsigned int StaticColor::BLUE600 = Utility::Color(0x1473E6);
    unsigned int StaticColor::BLUE700 = Utility::Color(0x0D66D0);
    unsigned int StaticColor::RED400 = Utility::Color(0xEC5B62);
    unsigned int StaticColor::RED500 = Utility::Color(0xE34850);
    unsigned int StaticColor::RED600 = Utility::Color(0xD7373F);
    unsigned int StaticColor::RED700 = Utility::Color(0xC9252D);
    unsigned int StaticColor::ORANGE400 = Utility::Color(0xF29423);
    unsigned int StaticColor::ORANGE500 = Utility::Color(0xE68619);
    unsigned int StaticColor::ORANGE600 = Utility::Color(0xDA7B11);
    unsigned int StaticColor::ORANGE700 = Utility::Color(0xCB6F10);
    unsigned int StaticColor::GREEN400 = Utility::Color(0x33AB84);
    unsigned int StaticColor::GREEN500 = Utility::Color(0x2D9D78);
    unsigned int StaticColor::GREEN600 = Utility::Color(0x268E6C);
    unsigned int StaticColor::GREEN700 = Utility::Color(0x12805C);

    unsigned int LightColor::GRAY50 = Utility::Color(0xFFFFFF);
    unsigned int LightColor::GRAY75 = Utility::Color(0xFAFAFA);
    unsigned int LightColor::GRAY100 = Utility::Color(0xF5F5F5);
    unsigned int LightColor::GRAY200 = Utility::Color(0xEAEAEA);
    unsigned int LightColor::GRAY300 = Utility::Color(0xE1E1E1);
    unsigned int LightColor::GRAY400 = Utility::Color(0xCACACA);
    unsigned int LightColor::GRAY500 = Utility::Color(0xB3B3B3);
    unsigned int LightColor::GRAY600 = Utility::Color(0x8E8E8E);
    unsigned int LightColor::GRAY700 = Utility::Color(0x707070);
    unsigned int LightColor::GRAY800 = Utility::Color(0x4B4B4B);
    unsigned int LightColor::GRAY900 = Utility::Color(0x2C2C2C);
    unsigned int LightColor::BLUE400 = Utility::Color(0x2680EB);
    unsigned int LightColor::BLUE500 = Utility::Color(0x1473E6);
    unsigned int LightColor::BLUE600 = Utility::Color(0x0D66D0);
    unsigned int LightColor::BLUE700 = Utility::Color(0x095ABA);
    unsigned int LightColor::RED400 = Utility::Color(0xE34850);
    unsigned int LightColor::RED500 = Utility::Color(0xD7373F);
    unsigned int LightColor::RED600 = Utility::Color(0xC9252D);
    unsigned int LightColor::RED700 = Utility::Color(0xBB121A);
    unsigned int LightColor::ORANGE400 = Utility::Color(0xE68619);
    unsigned int LightColor::ORANGE500 = Utility::Color(0xDA7B11);
    unsigned int LightColor::ORANGE600 = Utility::Color(0xCB6F10);
    unsigned int LightColor::ORANGE700 = Utility::Color(0xBD640D);
    unsigned int LightColor::GREEN400 = Utility::Color(0x2D9D78);
    unsigned int LightColor::GREEN500 = Utility::Color(0x268E6C);
    unsigned int LightColor::GREEN600 = Utility::Color(0x12805C);
    unsigned int LightColor::GREEN700 = Utility::Color(0x107154);
    unsigned int LightColor::INDIGO400 = Utility::Color(0x6767EC);
    unsigned int LightColor::INDIGO500 = Utility::Color(0x5C5CE0);
    unsigned int LightColor::INDIGO600 = Utility::Color(0x5151D3);
    unsigned int LightColor::INDIGO700 = Utility::Color(0x4646C6);
    unsigned int LightColor::CELERY400 = Utility::Color(0x44B556);
    unsigned int LightColor::CELERY500 = Utility::Color(0x3DA74E);
    unsigned int LightColor::CELERY600 = Utility::Color(0x379947);
    unsigned int LightColor::CELERY700 = Utility::Color(0x318B40);
    unsigned int LightColor::MAGENTA400 = Utility::Color(0xD83790);
    unsigned int LightColor::MAGENTA500 = Utility::Color(0xCE2783);
    unsigned int LightColor::MAGENTA600 = Utility::Color(0xBC1C74);
    unsigned int LightColor::MAGENTA700 = Utility::Color(0xAE0E66);
    unsigned int LightColor::YELLOW400 = Utility::Color(0xDFBF00);
    unsigned int LightColor::YELLOW500 = Utility::Color(0xD2B200);
    unsigned int LightColor::YELLOW600 = Utility::Color(0xC4A600);
    unsigned int LightColor::YELLOW700 = Utility::Color(0xB79900);
    unsigned int LightColor::FUCHSIA400 = Utility::Color(0xC038CC);
    unsigned int LightColor::FUCHSIA500 = Utility::Color(0xB130BD);
    unsigned int LightColor::FUCHSIA600 = Utility::Color(0xA228AD);
    unsigned int LightColor::FUCHSIA700 = Utility::Color(0x93219E);
    unsigned int LightColor::SEAFOAM400 = Utility::Color(0x1B959A);
    unsigned int LightColor::SEAFOAM500 = Utility::Color(0x16878C);
    unsigned int LightColor::SEAFOAM600 = Utility::Color(0x0F797D);
    unsigned int LightColor::SEAFOAM700 = Utility::Color(0x096C6F);
    unsigned int LightColor::CHARTREUSE400 = Utility::Color(0x85D044);
    unsigned int LightColor::CHARTREUSE500 = Utility::Color(0x7CC33F);
    unsigned int LightColor::CHARTREUSE600 = Utility::Color(0x73B53A);
    unsigned int LightColor::CHARTREUSE700 = Utility::Color(0x6AA834);
    unsigned int LightColor::PURPLE400 = Utility::Color(0x9256D9);
    unsigned int LightColor::PURPLE500 = Utility::Color(0x864CCC);
    unsigned int LightColor::PURPLE600 = Utility::Color(0x7A42BF);
    unsigned int LightColor::PURPLE700 = Utility::Color(0x6F38B1);

    unsigned int DarkColor::GRAY50 = Utility::Color(0x252525);
    unsigned int DarkColor::GRAY75 = Utility::Color(0x2F2F2F);
    unsigned int DarkColor::GRAY100 = Utility::Color(0x323232);
    unsigned int DarkColor::GRAY200 = Utility::Color(0x393939);
    unsigned int DarkColor::GRAY300 = Utility::Color(0x3E3E3E);
    unsigned int DarkColor::GRAY400 = Utility::Color(0x4D4D4D);
    unsigned int DarkColor::GRAY500 = Utility::Color(0x5C5C5C);
    unsigned int DarkColor::GRAY600 = Utility::Color(0x7B7B7B);
    unsigned int DarkColor::GRAY700 = Utility::Color(0x999999);
    unsigned int DarkColor::GRAY800 = Utility::Color(0xCDCDCD);
    unsigned int DarkColor::GRAY900 = Utility::Color(0xFFFFFF);
    unsigned int DarkColor::BLUE400 = Utility::Color(0x2680EB);
    unsigned int DarkColor::BLUE500 = Utility::Color(0x378EF0);
    unsigned int DarkColor::BLUE600 = Utility::Color(0x4B9CF5);
    unsigned int DarkColor::BLUE700 = Utility::Color(0x5AA9FA);
    unsigned int DarkColor::RED400 = Utility::Color(0xE34850);
    unsigned int DarkColor::RED500 = Utility::Color(0xEC5B62);
    unsigned int DarkColor::RED600 = Utility::Color(0xF76D74);
    unsigned int DarkColor::RED700 = Utility::Color(0xFF7B82);
    unsigned int DarkColor::ORANGE400 = Utility::Color(0xE68619);
    unsigned int DarkColor::ORANGE500 = Utility::Color(0xF29423);
    unsigned int DarkColor::ORANGE600 = Utility::Color(0xF9A43F);
    unsigned int DarkColor::ORANGE700 = Utility::Color(0xFFB55B);
    unsigned int DarkColor::GREEN400 = Utility::Color(0x2D9D78);
    unsigned int DarkColor::GREEN500 = Utility::Color(0x33AB84);
    unsigned int DarkColor::GREEN600 = Utility::Color(0x39B990);
    unsigned int DarkColor::GREEN700 = Utility::Color(0x3FC89C);
    unsigned int DarkColor::INDIGO400 = Utility::Color(0x6767EC);
    unsigned int DarkColor::INDIGO500 = Utility::Color(0x7575F1);
    unsigned int DarkColor::INDIGO600 = Utility::Color(0x8282F6);
    unsigned int DarkColor::INDIGO700 = Utility::Color(0x9090FA);
    unsigned int DarkColor::CELERY400 = Utility::Color(0x44B556);
    unsigned int DarkColor::CELERY500 = Utility::Color(0x4BC35F);
    unsigned int DarkColor::CELERY600 = Utility::Color(0x51D267);
    unsigned int DarkColor::CELERY700 = Utility::Color(0x58E06F);
    unsigned int DarkColor::MAGENTA400 = Utility::Color(0xD83790);
    unsigned int DarkColor::MAGENTA500 = Utility::Color(0xE2499D);
    unsigned int DarkColor::MAGENTA600 = Utility::Color(0xEC5AAA);
    unsigned int DarkColor::MAGENTA700 = Utility::Color(0xF56BB7);
    unsigned int DarkColor::YELLOW400 = Utility::Color(0xDFBF00);
    unsigned int DarkColor::YELLOW500 = Utility::Color(0xEDCC00);
    unsigned int DarkColor::YELLOW600 = Utility::Color(0xFAD900);
    unsigned int DarkColor::YELLOW700 = Utility::Color(0xFFE22E);
    unsigned int DarkColor::FUCHSIA400 = Utility::Color(0xC038CC);
    unsigned int DarkColor::FUCHSIA500 = Utility::Color(0xCF3EDC);
    unsigned int DarkColor::FUCHSIA600 = Utility::Color(0xD951E5);
    unsigned int DarkColor::FUCHSIA700 = Utility::Color(0xE366EF);
    unsigned int DarkColor::SEAFOAM400 = Utility::Color(0x1B959A);
    unsigned int DarkColor::SEAFOAM500 = Utility::Color(0x20A3A8);
    unsigned int DarkColor::SEAFOAM600 = Utility::Color(0x23B2B8);
    unsigned int DarkColor::SEAFOAM700 = Utility::Color(0x26C0C7);
    unsigned int DarkColor::CHARTREUSE400 = Utility::Color(0x85D044);
    unsigned int DarkColor::CHARTREUSE500 = Utility::Color(0x8EDE49);
    unsigned int DarkColor::CHARTREUSE600 = Utility::Color(0x9BEC54);
    unsigned int DarkColor::CHARTREUSE700 = Utility::Color(0xA3F858);
    unsigned int DarkColor::PURPLE400 = Utility::Color(0x9256D9);
    unsigned int DarkColor::PURPLE500 = Utility::Color(0x9D64E1);
    unsigned int DarkColor::PURPLE600 = Utility::Color(0xA873E9);
    unsigned int DarkColor::PURPLE700 = Utility::Color(0xB483F0);

    void LoadStyle(bool isDark)
    {
        ImGuiStyle* style = &ImGui::GetStyle();
        style->GrabRounding = 4.0f;

        ImVec4* colors = style->Colors;

        if (!isDark)
        {
            colors[ImGuiCol_Text] = ImGui::ColorConvertU32ToFloat4(LightColor::GRAY800); // text on hovered controls is gray900
            colors[ImGuiCol_TextDisabled] = ImGui::ColorConvertU32ToFloat4(LightColor::GRAY500);
            colors[ImGuiCol_WindowBg] = ImGui::ColorConvertU32ToFloat4(LightColor::GRAY100);
            colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            colors[ImGuiCol_PopupBg] = ImGui::ColorConvertU32ToFloat4(LightColor::GRAY50); // not sure about this. Note: applies to tooltips too.
            colors[ImGuiCol_Border] = ImGui::ColorConvertU32ToFloat4(LightColor::GRAY300);
            colors[ImGuiCol_BorderShadow] = ImGui::ColorConvertU32ToFloat4(StaticColor::NONE); // We don't want shadows. Ever.
            colors[ImGuiCol_FrameBg] = ImGui::ColorConvertU32ToFloat4(LightColor::GRAY75); // this isnt right, spectrum does not do this, but it's a good fallback
            colors[ImGuiCol_FrameBgHovered] = ImGui::ColorConvertU32ToFloat4(LightColor::GRAY50);
            colors[ImGuiCol_FrameBgActive] = ImGui::ColorConvertU32ToFloat4(LightColor::GRAY200);
            colors[ImGuiCol_TitleBg] = ImGui::ColorConvertU32ToFloat4(LightColor::GRAY300); // those titlebar values are totally made up, spectrum does not have this.
            colors[ImGuiCol_TitleBgActive] = ImGui::ColorConvertU32ToFloat4(LightColor::GRAY200);
            colors[ImGuiCol_TitleBgCollapsed] = ImGui::ColorConvertU32ToFloat4(LightColor::GRAY400);
            colors[ImGuiCol_MenuBarBg] = ImGui::ColorConvertU32ToFloat4(LightColor::GRAY100);
            colors[ImGuiCol_ScrollbarBg] = ImGui::ColorConvertU32ToFloat4(LightColor::GRAY100); // same as regular background
            colors[ImGuiCol_ScrollbarGrab] = ImGui::ColorConvertU32ToFloat4(LightColor::GRAY400);
            colors[ImGuiCol_ScrollbarGrabHovered] = ImGui::ColorConvertU32ToFloat4(LightColor::GRAY600);
            colors[ImGuiCol_ScrollbarGrabActive] = ImGui::ColorConvertU32ToFloat4(LightColor::GRAY700);
            colors[ImGuiCol_CheckMark] = ImGui::ColorConvertU32ToFloat4(LightColor::BLUE500);
            colors[ImGuiCol_SliderGrab] = ImGui::ColorConvertU32ToFloat4(LightColor::GRAY700);
            colors[ImGuiCol_SliderGrabActive] = ImGui::ColorConvertU32ToFloat4(LightColor::GRAY800);
            colors[ImGuiCol_Button] = ImGui::ColorConvertU32ToFloat4(LightColor::GRAY200); // match default button to Spectrum's 'Action Button'.
            colors[ImGuiCol_ButtonHovered] = ImGui::ColorConvertU32ToFloat4(LightColor::GRAY300);
            colors[ImGuiCol_ButtonActive] = ImGui::ColorConvertU32ToFloat4(LightColor::GRAY400);
            colors[ImGuiCol_Header] = ImGui::ColorConvertU32ToFloat4(LightColor::GRAY400);
            colors[ImGuiCol_HeaderHovered] = ImGui::ColorConvertU32ToFloat4(LightColor::GRAY500);
            colors[ImGuiCol_HeaderActive] = ImGui::ColorConvertU32ToFloat4(LightColor::GRAY600);
            colors[ImGuiCol_Separator] = ImGui::ColorConvertU32ToFloat4(LightColor::GRAY400);
            colors[ImGuiCol_SeparatorHovered] = ImGui::ColorConvertU32ToFloat4(LightColor::GRAY600);
            colors[ImGuiCol_SeparatorActive] = ImGui::ColorConvertU32ToFloat4(LightColor::GRAY700);
            colors[ImGuiCol_ResizeGrip] = ImGui::ColorConvertU32ToFloat4(LightColor::GRAY400);
            colors[ImGuiCol_ResizeGripHovered] = ImGui::ColorConvertU32ToFloat4(LightColor::GRAY600);
            colors[ImGuiCol_ResizeGripActive] = ImGui::ColorConvertU32ToFloat4(LightColor::GRAY700);
            colors[ImGuiCol_PlotLines] = ImGui::ColorConvertU32ToFloat4(LightColor::BLUE400);
            colors[ImGuiCol_PlotLinesHovered] = ImGui::ColorConvertU32ToFloat4(LightColor::BLUE600);
            colors[ImGuiCol_PlotHistogram] = ImGui::ColorConvertU32ToFloat4(LightColor::BLUE400);
            colors[ImGuiCol_PlotHistogramHovered] = ImGui::ColorConvertU32ToFloat4(LightColor::BLUE600);
            colors[ImGuiCol_TextSelectedBg] = ImGui::ColorConvertU32ToFloat4((LightColor::BLUE400 & 0x00FFFFFF) | 0x33000000);
            colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
            colors[ImGuiCol_NavHighlight] = ImGui::ColorConvertU32ToFloat4((LightColor::GRAY900 & 0x00FFFFFF) | 0x0A000000);
            colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
            colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
            colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
        }
        else
        {
            colors[ImGuiCol_Text] = ImGui::ColorConvertU32ToFloat4(DarkColor::GRAY800); // text on hovered controls is gray900
            colors[ImGuiCol_TextDisabled] = ImGui::ColorConvertU32ToFloat4(DarkColor::GRAY500);
            colors[ImGuiCol_WindowBg] = ImGui::ColorConvertU32ToFloat4(DarkColor::GRAY100);
            colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            colors[ImGuiCol_PopupBg] = ImGui::ColorConvertU32ToFloat4(DarkColor::GRAY50); // not sure about this. Note: applies to tooltips too.
            colors[ImGuiCol_Border] = ImGui::ColorConvertU32ToFloat4(DarkColor::GRAY300);
            colors[ImGuiCol_BorderShadow] = ImGui::ColorConvertU32ToFloat4(StaticColor::NONE); // We don't want shadows. Ever.
            colors[ImGuiCol_FrameBg] = ImGui::ColorConvertU32ToFloat4(DarkColor::GRAY75); // this isnt right, spectrum does not do this, but it's a good fallback
            colors[ImGuiCol_FrameBgHovered] = ImGui::ColorConvertU32ToFloat4(DarkColor::GRAY50);
            colors[ImGuiCol_FrameBgActive] = ImGui::ColorConvertU32ToFloat4(DarkColor::GRAY200);
            colors[ImGuiCol_TitleBg] = ImGui::ColorConvertU32ToFloat4(DarkColor::GRAY300); // those titlebar values are totally made up, spectrum does not have this.
            colors[ImGuiCol_TitleBgActive] = ImGui::ColorConvertU32ToFloat4(DarkColor::GRAY200);
            colors[ImGuiCol_TitleBgCollapsed] = ImGui::ColorConvertU32ToFloat4(DarkColor::GRAY400);
            colors[ImGuiCol_MenuBarBg] = ImGui::ColorConvertU32ToFloat4(DarkColor::GRAY100);
            colors[ImGuiCol_ScrollbarBg] = ImGui::ColorConvertU32ToFloat4(DarkColor::GRAY100); // same as regular background
            colors[ImGuiCol_ScrollbarGrab] = ImGui::ColorConvertU32ToFloat4(DarkColor::GRAY400);
            colors[ImGuiCol_ScrollbarGrabHovered] = ImGui::ColorConvertU32ToFloat4(DarkColor::GRAY600);
            colors[ImGuiCol_ScrollbarGrabActive] = ImGui::ColorConvertU32ToFloat4(DarkColor::GRAY700);
            colors[ImGuiCol_CheckMark] = ImGui::ColorConvertU32ToFloat4(DarkColor::BLUE500);
            colors[ImGuiCol_SliderGrab] = ImGui::ColorConvertU32ToFloat4(DarkColor::GRAY700);
            colors[ImGuiCol_SliderGrabActive] = ImGui::ColorConvertU32ToFloat4(DarkColor::GRAY800);
            colors[ImGuiCol_Button] = ImGui::ColorConvertU32ToFloat4(DarkColor::GRAY75); // match default button to Spectrum's 'Action Button'.
            colors[ImGuiCol_ButtonHovered] = ImGui::ColorConvertU32ToFloat4(DarkColor::GRAY50);
            colors[ImGuiCol_ButtonActive] = ImGui::ColorConvertU32ToFloat4(DarkColor::GRAY200);
            colors[ImGuiCol_Header] = ImGui::ColorConvertU32ToFloat4(DarkColor::GRAY400);
            colors[ImGuiCol_HeaderHovered] = ImGui::ColorConvertU32ToFloat4(DarkColor::GRAY500);
            colors[ImGuiCol_HeaderActive] = ImGui::ColorConvertU32ToFloat4(DarkColor::GRAY600);
            colors[ImGuiCol_Separator] = ImGui::ColorConvertU32ToFloat4(DarkColor::GRAY400);
            colors[ImGuiCol_SeparatorHovered] = ImGui::ColorConvertU32ToFloat4(DarkColor::GRAY600);
            colors[ImGuiCol_SeparatorActive] = ImGui::ColorConvertU32ToFloat4(DarkColor::GRAY700);
            colors[ImGuiCol_ResizeGrip] = ImGui::ColorConvertU32ToFloat4(DarkColor::GRAY400);
            colors[ImGuiCol_ResizeGripHovered] = ImGui::ColorConvertU32ToFloat4(DarkColor::GRAY600);
            colors[ImGuiCol_ResizeGripActive] = ImGui::ColorConvertU32ToFloat4(DarkColor::GRAY700);
            colors[ImGuiCol_PlotLines] = ImGui::ColorConvertU32ToFloat4(DarkColor::BLUE400);
            colors[ImGuiCol_PlotLinesHovered] = ImGui::ColorConvertU32ToFloat4(DarkColor::BLUE600);
            colors[ImGuiCol_PlotHistogram] = ImGui::ColorConvertU32ToFloat4(DarkColor::BLUE400);
            colors[ImGuiCol_PlotHistogramHovered] = ImGui::ColorConvertU32ToFloat4(DarkColor::BLUE600);
            colors[ImGuiCol_TextSelectedBg] = ImGui::ColorConvertU32ToFloat4((DarkColor::BLUE400 & 0x00FFFFFF) | 0x33000000);
            colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
            colors[ImGuiCol_NavHighlight] = ImGui::ColorConvertU32ToFloat4((DarkColor::GRAY900 & 0x00FFFFFF) | 0x0A000000);
            colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
            colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
            colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
        }
    }

}

#endif
