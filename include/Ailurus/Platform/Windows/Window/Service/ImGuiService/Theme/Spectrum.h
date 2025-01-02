#pragma once

// Style from Adobe
// https://github.com/adobe/imgui/blob/master/imgui_spectrum.cpp
// all colors are from http://spectrum.corp.adobe.com/color.html

#include "Ailurus/PlatformDefine.h"

#if AILURUS_PLATFORM_WINDOWS

namespace Ailurus::Spectrum
{
    // a list of changes introduced to change the look of the widgets.
    // Collected here as const rather than being magic numbers spread
    // around imgui.cpp and imgui_widgets.cpp.
    const float CHECKBOX_BORDER_SIZE = 2.0f;
    const float CHECKBOX_ROUNDING = 2.0f;

    // Sets the ImGui style to Spectrum
    void LoadStyle(bool isDark = false);

    struct StaticColor
    {
        static unsigned int NONE;
        static unsigned int WHITE;
        static unsigned int BLACK;
        static unsigned int GRAY200;
        static unsigned int GRAY300;
        static unsigned int GRAY400;
        static unsigned int GRAY500;
        static unsigned int GRAY600;
        static unsigned int GRAY700;
        static unsigned int GRAY800;
        static unsigned int GRAY900;
        static unsigned int BLUE400;
        static unsigned int BLUE500;
        static unsigned int BLUE600;
        static unsigned int BLUE700;
        static unsigned int RED400;
        static unsigned int RED500;
        static unsigned int RED600;
        static unsigned int RED700;
        static unsigned int ORANGE400;
        static unsigned int ORANGE500;
        static unsigned int ORANGE600;
        static unsigned int ORANGE700;
        static unsigned int GREEN400;
        static unsigned int GREEN500;
        static unsigned int GREEN600;
        static unsigned int GREEN700;
    };

    struct LightColor
    {
        static unsigned int GRAY50;
        static unsigned int GRAY75;
        static unsigned int GRAY100;
        static unsigned int GRAY200;
        static unsigned int GRAY300;
        static unsigned int GRAY400;
        static unsigned int GRAY500;
        static unsigned int GRAY600;
        static unsigned int GRAY700;
        static unsigned int GRAY800;
        static unsigned int GRAY900;
        static unsigned int BLUE400;
        static unsigned int BLUE500;
        static unsigned int BLUE600;
        static unsigned int BLUE700;
        static unsigned int RED400;
        static unsigned int RED500;
        static unsigned int RED600;
        static unsigned int RED700;
        static unsigned int ORANGE400;
        static unsigned int ORANGE500;
        static unsigned int ORANGE600;
        static unsigned int ORANGE700;
        static unsigned int GREEN400;
        static unsigned int GREEN500;
        static unsigned int GREEN600;
        static unsigned int GREEN700;
        static unsigned int INDIGO400;
        static unsigned int INDIGO500;
        static unsigned int INDIGO600;
        static unsigned int INDIGO700;
        static unsigned int CELERY400;
        static unsigned int CELERY500;
        static unsigned int CELERY600;
        static unsigned int CELERY700;
        static unsigned int MAGENTA400;
        static unsigned int MAGENTA500;
        static unsigned int MAGENTA600;
        static unsigned int MAGENTA700;
        static unsigned int YELLOW400;
        static unsigned int YELLOW500;
        static unsigned int YELLOW600;
        static unsigned int YELLOW700;
        static unsigned int FUCHSIA400;
        static unsigned int FUCHSIA500;
        static unsigned int FUCHSIA600;
        static unsigned int FUCHSIA700;
        static unsigned int SEAFOAM400;
        static unsigned int SEAFOAM500;
        static unsigned int SEAFOAM600;
        static unsigned int SEAFOAM700;
        static unsigned int CHARTREUSE400;
        static unsigned int CHARTREUSE500;
        static unsigned int CHARTREUSE600;
        static unsigned int CHARTREUSE700;
        static unsigned int PURPLE400;
        static unsigned int PURPLE500;
        static unsigned int PURPLE600;
        static unsigned int PURPLE700;
    };

    struct DarkColor
    {
        static unsigned int GRAY50;
        static unsigned int GRAY75;
        static unsigned int GRAY100;
        static unsigned int GRAY200;
        static unsigned int GRAY300;
        static unsigned int GRAY400;
        static unsigned int GRAY500;
        static unsigned int GRAY600;
        static unsigned int GRAY700;
        static unsigned int GRAY800;
        static unsigned int GRAY900;
        static unsigned int BLUE400;
        static unsigned int BLUE500;
        static unsigned int BLUE600;
        static unsigned int BLUE700;
        static unsigned int RED400;
        static unsigned int RED500;
        static unsigned int RED600;
        static unsigned int RED700;
        static unsigned int ORANGE400;
        static unsigned int ORANGE500;
        static unsigned int ORANGE600;
        static unsigned int ORANGE700;
        static unsigned int GREEN400;
        static unsigned int GREEN500;
        static unsigned int GREEN600;
        static unsigned int GREEN700;
        static unsigned int INDIGO400;
        static unsigned int INDIGO500;
        static unsigned int INDIGO600;
        static unsigned int INDIGO700;
        static unsigned int CELERY400;
        static unsigned int CELERY500;
        static unsigned int CELERY600;
        static unsigned int CELERY700;
        static unsigned int MAGENTA400;
        static unsigned int MAGENTA500;
        static unsigned int MAGENTA600;
        static unsigned int MAGENTA700;
        static unsigned int YELLOW400;
        static unsigned int YELLOW500;
        static unsigned int YELLOW600;
        static unsigned int YELLOW700;
        static unsigned int FUCHSIA400;
        static unsigned int FUCHSIA500;
        static unsigned int FUCHSIA600;
        static unsigned int FUCHSIA700;
        static unsigned int SEAFOAM400;
        static unsigned int SEAFOAM500;
        static unsigned int SEAFOAM600;
        static unsigned int SEAFOAM700;
        static unsigned int CHARTREUSE400;
        static unsigned int CHARTREUSE500;
        static unsigned int CHARTREUSE600;
        static unsigned int CHARTREUSE700;
        static unsigned int PURPLE400;
        static unsigned int PURPLE500;
        static unsigned int PURPLE600;
        static unsigned int PURPLE700;
    };
}

#endif
