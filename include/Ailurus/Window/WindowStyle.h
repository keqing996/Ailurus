#pragma once

#include "Ailurus/PlatformDefine.h"

#if AILURUS_FEAT_SUPPORT_WINDOW

namespace Ailurus
{
    struct WindowStyle
    {
        bool haveTitleBar = true;
        bool haveResize = true;
        bool haveClose = true;
    };
}

#endif