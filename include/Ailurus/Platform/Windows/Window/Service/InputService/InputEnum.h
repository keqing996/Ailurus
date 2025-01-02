#pragma once

#include <string>

namespace Ailurus
{
    enum class ButtonType: int
    {
        /* Mouse */
        MouseLeft = 0,
        MouseMiddle,
        MouseRight,
        MouseXButton1,
        MouseXButton2,
        /* Keyboard A-Z */
        KeyboardA, KeyboardB, KeyboardC, KeyboardD,
        KeyboardE, KeyboardF, KeyboardG, KeyboardH,
        KeyboardI, KeyboardJ, KeyboardK, KeyboardL,
        KeyboardM, KeyboardN, KeyboardO, KeyboardP,
        KeyboardQ, KeyboardR, KeyboardS, KeyboardT,
        KeyboardU, KeyboardV, KeyboardW, KeyboardX,
        KeyboardY, KeyboardZ,
        /* Keyboard Num */
        KeyboardNum0, KeyboardNum1, KeyboardNum2, KeyboardNum3,
        KeyboardNum4, KeyboardNum5, KeyboardNum6, KeyboardNum7,
        KeyboardNum8, KeyboardNum9,
        /* Keyboard F1-F24 */
        KeyboardF1,  KeyboardF2,  KeyboardF3,
        KeyboardF4,  KeyboardF5,  KeyboardF6,
        KeyboardF7,  KeyboardF8,  KeyboardF9,
        KeyboardF10, KeyboardF11, KeyboardF12,
        KeyboardF13, KeyboardF14, KeyboardF15,
        KeyboardF16, KeyboardF17, KeyboardF18,
        KeyboardF19, KeyboardF20, KeyboardF21,
        KeyboardF22, KeyboardF23, KeyboardF24,
        /* Keyboard Navigation */
        KeyboardArrowUp,
        KeyboardArrowDown,
        KeyboardArrowLeft,
        KeyboardArrowRight,
        /* Keyboard NumPad */
        KeyboardNumPad0, KeyboardNumPad1, KeyboardNumPad2,
        KeyboardNumPad3, KeyboardNumPad4, KeyboardNumPad5,
        KeyboardNumPad6, KeyboardNumPad7, KeyboardNumPad8,
        KeyboardNumPad9, KeyboardNumPadMinus,
        KeyboardNumPadAdd, KeyboardNumPadMultiply,
        KeyboardNumPadDivide, KeyboardNumPadDecimal,
        KeyboardNumPadLock, KeyboardNumPadEnter,
        /* Keyboard Control */
        KeyboardEscape,
        KeyboardTab,
        KeyboardCapital,
        KeyboardLeftShift,
        KeyboardLeftCtrl,
        KeyboardLeftWindows,
        KeyboardLeftAlt,
        KeyboardSpace,
        KeyboardRightAlt,
        KeyboardRightWindows,
        KeyboardRightCtrl,
        KeyboardRightShift,
        KeyboardEnter,
        KeyboardBackspace,
        /* Keyboard Edit */
        KeyboardInsert,
        KeyboardDelete,
        KeyboardPageUp,
        KeyboardPageDown,
        KeyboardHome,
        KeyboardEnd,
        /* Keyboard Sign */
        KeyboardComma,              /* < , */
        KeyboardPeriod,             /* > . */
        KeyboardSlash,              /* ? / */
        KeyboardSemicolon,          /* : ; */
        KeyboardApostrophe,         /* " ' */
        KeyboardLeftBracket,        /* { [ */
        KeyboardRightBracket,       /* } ] */
        KeyboardBackslash,          /* | \ */
        KeyboardGraveAccent,        /* ~ ` */
        KeyboardMinus,              /* _ - */
        KeyboardEqual,              /* + = */
        /* Keyboard Other */
        KeyboardApps,
        KeyboardPause,
        KeyboardPrintScreen,
        KeyboardScrollLock,
        KeyboardVolumeMute,
        KeyboardVolumeUp,
        KeyboardVolumeDown,
        KeyboardMediaPlayPause,
        KeyboardMediaStop,
        KeyboardMediaPrev,
        KeyboardMediaNext,
        /* Finish */
        Unknown,
        Count
    };

    class ButtonTypeUtility
    {
    public:
        ButtonTypeUtility() = delete;

    public:
        static const char* GetName(ButtonType type);
    };
}
