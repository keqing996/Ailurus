#pragma once

#include "Ailurus/PlatformDefine.h"

#if AILURUS_PLATFORM_WINDOWS

namespace Ailurus
{
    class WindowStyle
    {
    public:
        enum class Type
        {
            Popup,
            Overlapped
        };

        struct PopupStyle
        {
            // nothing
        };

        struct OverlappedStyle
        {
            bool haveTitleBar = true;
            bool haveResize = true;
            bool haveClose = true;
        };

        union Config
        {
            PopupStyle popup;
            OverlappedStyle overlapped;
        };

    public:
        static WindowStyle DefaultStyle()
        {
            WindowStyle style{};
            style.type = Type::Overlapped;
            style.config.overlapped = OverlappedStyle();
            return style;
        }

    public:
        Type type;
        Config config;
    };
}

#endif