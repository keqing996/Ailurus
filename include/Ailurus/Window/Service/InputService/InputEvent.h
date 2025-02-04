
#pragma once

#include "Ailurus/PlatformDefine.h"

#if AILURUS_FEAT_SUPPORT_WINDOW

#include <utility>
#include "InputEnum.h"

namespace Ailurus
{
    enum class InputEventType
    {
        MouseMove,
        MouseWheel,
        Button,
    };

    struct MouseMoveEventData
    {
        std::pair<int, int> position;
    };

    struct MouseWheelEventData
    {
        float delta;
    };

    struct ButtonData
    {
        ButtonType button;
        bool isPress;
    };

    union InputEventData
    {
        MouseMoveEventData mouseMove;
        MouseWheelEventData mouseWheel;
        ButtonData button;
    };

    struct InputEvent
    {
        InputEventType eventType;
        InputEventData data;
    };

}

#endif