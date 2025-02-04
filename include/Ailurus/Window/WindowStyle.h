#pragma once

#include "Ailurus/PlatformDefine.h"

#if AILURUS_FEAT_SUPPORT_WINDOW

namespace Ailurus
{
    struct WindowStyle
    {
        bool canResize = true;
        bool canMin = true;
        bool canMax = true;
        bool haveBorder = true;
    };
}

#endif