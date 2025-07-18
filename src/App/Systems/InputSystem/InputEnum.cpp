#include "Ailurus/Application/InputSystem/InputEnum.h"

namespace Ailurus
{
    const char* ButtonTypeUtility::GetName(ButtonType type)
    {
        switch (type)
        {
            case ButtonType::MouseLeft:             return "MouseLeft";
            case ButtonType::MouseMiddle:           return "MouseMiddle";
            case ButtonType::MouseRight:            return "MouseRight";
            case ButtonType::MouseXButton1:         return "MouseXButton1";
            case ButtonType::MouseXButton2:         return "MouseXButton2";
            case ButtonType::KeyboardA:             return "KeyboardA";
            case ButtonType::KeyboardB:             return "KeyboardB";
            case ButtonType::KeyboardC:             return "KeyboardC";
            case ButtonType::KeyboardD:             return "KeyboardD";
            case ButtonType::KeyboardE:             return "KeyboardE";
            case ButtonType::KeyboardF:             return "KeyboardF";
            case ButtonType::KeyboardG:             return "KeyboardG";
            case ButtonType::KeyboardH:             return "KeyboardH";
            case ButtonType::KeyboardI:             return "KeyboardI";
            case ButtonType::KeyboardJ:             return "KeyboardJ";
            case ButtonType::KeyboardK:             return "KeyboardK";
            case ButtonType::KeyboardL:             return "KeyboardL";
            case ButtonType::KeyboardM:             return "KeyboardM";
            case ButtonType::KeyboardN:             return "KeyboardN";
            case ButtonType::KeyboardO:             return "KeyboardO";
            case ButtonType::KeyboardP:             return "KeyboardP";
            case ButtonType::KeyboardQ:             return "KeyboardQ";
            case ButtonType::KeyboardR:             return "KeyboardR";
            case ButtonType::KeyboardS:             return "KeyboardS";
            case ButtonType::KeyboardT:             return "KeyboardT";
            case ButtonType::KeyboardU:             return "KeyboardU";
            case ButtonType::KeyboardV:             return "KeyboardV";
            case ButtonType::KeyboardW:             return "KeyboardW";
            case ButtonType::KeyboardX:             return "KeyboardX";
            case ButtonType::KeyboardY:             return "KeyboardY";
            case ButtonType::KeyboardZ:             return "KeyboardZ";
            case ButtonType::KeyboardNum0:          return "KeyboardNum0";
            case ButtonType::KeyboardNum1:          return "KeyboardNum1";
            case ButtonType::KeyboardNum2:          return "KeyboardNum2";
            case ButtonType::KeyboardNum3:          return "KeyboardNum3";
            case ButtonType::KeyboardNum4:          return "KeyboardNum4";
            case ButtonType::KeyboardNum5:          return "KeyboardNum5";
            case ButtonType::KeyboardNum6:          return "KeyboardNum6";
            case ButtonType::KeyboardNum7:          return "KeyboardNum7";
            case ButtonType::KeyboardNum8:          return "KeyboardNum8";
            case ButtonType::KeyboardNum9:          return "KeyboardNum9";
            case ButtonType::KeyboardF1:            return "KeyboardF1";
            case ButtonType::KeyboardF2:            return "KeyboardF2";
            case ButtonType::KeyboardF3:            return "KeyboardF3";
            case ButtonType::KeyboardF4:            return "KeyboardF4";
            case ButtonType::KeyboardF5:            return "KeyboardF5";
            case ButtonType::KeyboardF6:            return "KeyboardF6";
            case ButtonType::KeyboardF7:            return "KeyboardF7";
            case ButtonType::KeyboardF8:            return "KeyboardF8";
            case ButtonType::KeyboardF9:            return "KeyboardF9";
            case ButtonType::KeyboardF10:           return "KeyboardF10";
            case ButtonType::KeyboardF11:           return "KeyboardF11";
            case ButtonType::KeyboardF12:           return "KeyboardF12";
            case ButtonType::KeyboardF13:           return "KeyboardF13";
            case ButtonType::KeyboardF14:           return "KeyboardF14";
            case ButtonType::KeyboardF15:           return "KeyboardF15";
            case ButtonType::KeyboardF16:           return "KeyboardF16";
            case ButtonType::KeyboardF17:           return "KeyboardF17";
            case ButtonType::KeyboardF18:           return "KeyboardF18";
            case ButtonType::KeyboardF19:           return "KeyboardF19";
            case ButtonType::KeyboardF20:           return "KeyboardF20";
            case ButtonType::KeyboardF21:           return "KeyboardF21";
            case ButtonType::KeyboardF22:           return "KeyboardF22";
            case ButtonType::KeyboardF23:           return "KeyboardF23";
            case ButtonType::KeyboardF24:           return "KeyboardF24";
            case ButtonType::KeyboardArrowUp:       return "KeyboardArrowUp";
            case ButtonType::KeyboardArrowDown:     return "KeyboardArrowDown";
            case ButtonType::KeyboardArrowLeft:     return "KeyboardArrowLeft";
            case ButtonType::KeyboardArrowRight:    return "KeyboardArrowRight";
            case ButtonType::KeyboardNumPad0:       return "KeyboardNumPad0";
            case ButtonType::KeyboardNumPad1:       return "KeyboardNumPad1";
            case ButtonType::KeyboardNumPad2:       return "KeyboardNumPad2";
            case ButtonType::KeyboardNumPad3:       return "KeyboardNumPad3";
            case ButtonType::KeyboardNumPad4:       return "KeyboardNumPad4";
            case ButtonType::KeyboardNumPad5:       return "KeyboardNumPad5";
            case ButtonType::KeyboardNumPad6:       return "KeyboardNumPad6";
            case ButtonType::KeyboardNumPad7:       return "KeyboardNumPad7";
            case ButtonType::KeyboardNumPad8:       return "KeyboardNumPad8";
            case ButtonType::KeyboardNumPad9:       return "KeyboardNumPad9";
            case ButtonType::KeyboardNumPadMinus:   return "KeyboardNumPadMinus";
            case ButtonType::KeyboardNumPadAdd:     return "KeyboardNumPadAdd";
            case ButtonType::KeyboardNumPadMultiply:return "KeyboardNumPadMultiply";
            case ButtonType::KeyboardNumPadDivide:  return "KeyboardNumPadDivide";
            case ButtonType::KeyboardNumPadDecimal: return "KeyboardNumPadDecimal";
            case ButtonType::KeyboardNumPadLock:    return "KeyboardNumPadLock";
            case ButtonType::KeyboardNumPadEnter:   return "KeyboardNumPadEnter";
            case ButtonType::KeyboardEscape:        return "KeyboardEscape";
            case ButtonType::KeyboardTab:           return "KeyboardTab";
            case ButtonType::KeyboardCapital:       return "KeyboardCapital";
            case ButtonType::KeyboardLeftShift:     return "KeyboardLeftShift";
            case ButtonType::KeyboardLeftCtrl:      return "KeyboardLeftCtrl";
            case ButtonType::KeyboardLeftSuper:     return "KeyboardLeftSuper";
            case ButtonType::KeyboardLeftAlt:       return "KeyboardLeftAlt";
            case ButtonType::KeyboardSpace:         return "KeyboardSpace";
            case ButtonType::KeyboardRightAlt:      return "KeyboardRightAlt";
            case ButtonType::KeyboardRightSuper:    return "KeyboardRightSuper";
            case ButtonType::KeyboardRightCtrl:     return "KeyboardRightCtrl";
            case ButtonType::KeyboardRightShift:    return "KeyboardRightShift";
            case ButtonType::KeyboardEnter:         return "KeyboardEnter";
            case ButtonType::KeyboardBackspace:     return "KeyboardBackspace";
            case ButtonType::KeyboardInsert:        return "KeyboardInsert";
            case ButtonType::KeyboardDelete:        return "KeyboardDelete";
            case ButtonType::KeyboardPageUp:        return "KeyboardPageUp";
            case ButtonType::KeyboardPageDown:      return "KeyboardPageDown";
            case ButtonType::KeyboardHome:          return "KeyboardHome";
            case ButtonType::KeyboardEnd:           return "KeyboardEnd";
            case ButtonType::KeyboardComma:         return "KeyboardComma";
            case ButtonType::KeyboardPeriod:        return "KeyboardPeriod";
            case ButtonType::KeyboardSlash:         return "KeyboardSlash";
            case ButtonType::KeyboardSemicolon:     return "KeyboardSemicolon";
            case ButtonType::KeyboardApostrophe:    return "KeyboardApostrophe";
            case ButtonType::KeyboardLeftBracket:   return "KeyboardLeftBracket";
            case ButtonType::KeyboardRightBracket:  return "KeyboardRightBracket";
            case ButtonType::KeyboardBackslash:     return "KeyboardBackslash";
            case ButtonType::KeyboardGraveAccent:   return "KeyboardGraveAccent";
            case ButtonType::KeyboardMinus:         return "KeyboardMinus";
            case ButtonType::KeyboardEqual:         return "KeyboardEqual";
            case ButtonType::KeyboardApps:          return "KeyboardApps";
            case ButtonType::KeyboardPause:         return "KeyboardPause";
            case ButtonType::KeyboardPrintScreen:   return "KeyboardPrintScreen";
            case ButtonType::KeyboardScrollLock:    return "KeyboardScrollLock";
            case ButtonType::KeyboardVolumeMute:    return "KeyboardVolumeMute";
            case ButtonType::KeyboardVolumeUp:      return "KeyboardVolumeUp";
            case ButtonType::KeyboardVolumeDown:    return "KeyboardVolumeDown";
            case ButtonType::KeyboardMediaPlayPause:return "KeyboardMediaPlayPause";
            case ButtonType::KeyboardMediaStop:     return "KeyboardMediaStop";
            case ButtonType::KeyboardMediaPrev:     return "KeyboardMediaPrev";
            case ButtonType::KeyboardMediaNext:     return "KeyboardMediaNext";
            case ButtonType::Unknown:
            case ButtonType::Count:
            default:
                return "Unknown";
        }
    }
}

