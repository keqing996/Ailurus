#include <cstdint>
#include <unordered_map>
#include "Ailurus/Platform/Windows/Window/Utility/WindowsInclude.h"
#include "Ailurus/Platform/Windows/Window/Service/InputService/InputService.h"

namespace Ailurus
{
    InputService::InputService(Window* pWindow)
        : Service(pWindow)
    {
        _mousePos = std::make_pair(-1, -1);
        _mouseWheel = 0;
    }

    void InputService::ProcessWinMessage(void* hWnd, uint32_t msg, void* wpara, void* lpara)
    {
        const WPARAM wParam = reinterpret_cast<WPARAM>(wpara);
        const LPARAM lParam = reinterpret_cast<LPARAM>(lpara);
        switch (msg)
        {
            case WM_KEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            {
                const bool pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
                auto button = WinVirtualKeyToButtonType(wpara, lpara);

                InputEvent inputEvent{};
                inputEvent.eventType = InputEventType::Button;
                inputEvent.data.button.button = button;
                inputEvent.data.button.isPress = pressed;

                _eventQueue.emplace_back(inputEvent);
                
                break;
            }
            case WM_MOUSEMOVE:
            // case WM_NCMOUSEMOVE: // For non-client area, y pos will be negative, so we do not deal this case for now.
            {
                POINT currentPos = { static_cast<LONG>(LOWORD(lParam)), static_cast<LONG>(HIWORD(lParam)) };
                if (msg == WM_NCMOUSEMOVE && ::ScreenToClient(static_cast<HWND>(hWnd), &currentPos) == FALSE)
                    break;

                InputEvent inputEvent{};
                inputEvent.eventType = InputEventType::MouseMove;
                inputEvent.data.mouseMove.position = std::make_pair(currentPos.x, currentPos.y);
                _eventQueue.emplace_back(inputEvent);
                break;
            }
            case WM_MOUSELEAVE:
            // case WM_NCMOUSELEAVE:
            {
                InputEvent inputEvent{};
                inputEvent.eventType = InputEventType::MouseMove;
                inputEvent.data.mouseMove.position = std::make_pair(-1, -1);
                _eventQueue.emplace_back(inputEvent);
                break;
            }
            case WM_LBUTTONDOWN:
            case WM_RBUTTONDOWN:
            case WM_MBUTTONDOWN:
            case WM_XBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_RBUTTONUP:
            case WM_MBUTTONUP:
            case WM_XBUTTONUP:
            {
                bool pressed = (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN
                    || msg == WM_MBUTTONDOWN || msg == WM_XBUTTONDOWN);

                ButtonType button = ButtonType::Unknown;
                if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP)
                    button = ButtonType::MouseLeft;
                else if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP)
                    button = ButtonType::MouseRight;
                else if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONUP)
                    button = ButtonType::MouseMiddle;
                else if (msg == WM_XBUTTONDOWN || msg == WM_XBUTTONUP)
                    button = GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? ButtonType::MouseXButton1 : ButtonType::MouseXButton2;

                if (button != ButtonType::Unknown)
                {
                    InputEvent inputEvent{};
                    inputEvent.eventType = InputEventType::Button;
                    inputEvent.data.button.button = button;
                    inputEvent.data.button.isPress = pressed;

                    _eventQueue.emplace_back(inputEvent);
                }
                break;
            }
            case WM_MOUSEWHEEL:
            {
                const float delta = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / static_cast<float>(WHEEL_DELTA);

                InputEvent inputEvent{};
                inputEvent.eventType = InputEventType::MouseWheel;
                inputEvent.data.mouseWheel.delta = delta;
                _eventQueue.emplace_back(inputEvent);

                break;
            }
        }
    }

    void InputService::BeforeTick()
    {
        ProcessEventQueue();
    }

    void InputService::ProcessEventQueue()
    {
        // Record last frame mouse position.
        auto mousePosLastFrame = _mousePos;
        // Mouse wheel reset every frame.
        _mouseWheel = 0;

        for (const auto& [eventType, eventData]: _eventQueue)
        {
            if (eventType == InputEventType::Button)
            {
                auto [buttonType, isPressed] = eventData.button;
                if (isPressed)
                {
                    if (_pressedButton.find(buttonType) == _pressedButton.end())
                    {
                        _pressedButton.insert(buttonType);
                        if (_onButtonPressed != nullptr)
                            _onButtonPressed(buttonType);
                    }
                    else
                    {
                        if (_enableAutoRepeat && _onButtonPressed != nullptr)
                            _onButtonPressed(buttonType);
                    }
                }
                else
                {
                    if (auto itr = _pressedButton.find(buttonType); itr != _pressedButton.end())
                    {
                        _pressedButton.erase(itr);
                        if (_onButtonReleased != nullptr)
                            _onButtonReleased(buttonType);
                    }
                }
            }
            else if (eventType == InputEventType::MouseMove)
            {
                _mousePos = eventData.mouseMove.position;
            }
            else if (eventType == InputEventType::MouseWheel)
            {
                _mouseWheel += eventData.mouseWheel.delta;
            }
        }

        _eventQueue.clear();

        if (_onMouseMove != nullptr && mousePosLastFrame != _mousePos)
            _onMouseMove(mousePosLastFrame, _mousePos);

        if (_onMouseWheel != nullptr && _mouseWheel != 0)
            _onMouseWheel(_mouseWheel);
    }

    bool InputService::IsButtonPressed(ButtonType key) const
    {
        return _pressedButton.find(key) != _pressedButton.end();
    }

    std::pair<int, int> InputService::GetMousePosition() const
    {
        return _mousePos;
    }

    float InputService::GetMouseWheel() const
    {
        return _mouseWheel;
    }

    bool InputService::GetIsAutoRepeat() const
    {
        return _enableAutoRepeat;
    }

    void InputService::SetIsAutoRepeat(bool autoRepeat)
    {
        _enableAutoRepeat = autoRepeat;
    }

    void InputService::SetCallbackOnMouseMove(const std::function<void(std::pair<int, int>, std::pair<int, int>)>& fun)
    {
        _onMouseMove = fun;
    }

    void InputService::SetCallbackOnMouseWheel(const std::function<void(float)>& fun)
    {
        _onMouseWheel = fun;
    }

    void InputService::SetCallbackOnButtonPressed(const std::function<void(ButtonType)>& fun)
    {
        _onButtonPressed = fun;
    }

    void InputService::SetCallbackOnButtonReleased(const std::function<void(ButtonType)>& fun)
    {
        _onButtonReleased = fun;
    }

    ButtonType InputService::WinVirtualKeyToButtonType(void* wpara, void* lpara)
    {
        wpara = MapControlKeyLeftAndRight(wpara, lpara);

        WPARAM wParam = reinterpret_cast<WPARAM>(wpara);
        LPARAM lParam = reinterpret_cast<LPARAM>(lpara);
        if ((wParam == VK_RETURN) && (HIWORD(lParam) & KF_EXTENDED))
            return ButtonType::KeyboardNumPadEnter;

        switch (wParam)
        {
            /* Keyboard A-Z */
            case 'A':       return ButtonType::KeyboardA;
            case 'B':       return ButtonType::KeyboardB;
            case 'C':       return ButtonType::KeyboardC;
            case 'D':       return ButtonType::KeyboardD;
            case 'E':       return ButtonType::KeyboardE;
            case 'F':       return ButtonType::KeyboardF;
            case 'G':       return ButtonType::KeyboardG;
            case 'H':       return ButtonType::KeyboardH;
            case 'I':       return ButtonType::KeyboardI;
            case 'J':       return ButtonType::KeyboardJ;
            case 'K':       return ButtonType::KeyboardK;
            case 'L':       return ButtonType::KeyboardL;
            case 'M':       return ButtonType::KeyboardM;
            case 'N':       return ButtonType::KeyboardN;
            case 'O':       return ButtonType::KeyboardO;
            case 'P':       return ButtonType::KeyboardP;
            case 'Q':       return ButtonType::KeyboardQ;
            case 'R':       return ButtonType::KeyboardR;
            case 'S':       return ButtonType::KeyboardS;
            case 'T':       return ButtonType::KeyboardT;
            case 'U':       return ButtonType::KeyboardU;
            case 'V':       return ButtonType::KeyboardV;
            case 'W':       return ButtonType::KeyboardW;
            case 'X':       return ButtonType::KeyboardX;
            case 'Y':       return ButtonType::KeyboardY;
            case 'Z':       return ButtonType::KeyboardZ;
            /* Keyboard Num */
            case '0':       return ButtonType::KeyboardNum0;
            case '1':       return ButtonType::KeyboardNum1;
            case '2':       return ButtonType::KeyboardNum2;
            case '3':       return ButtonType::KeyboardNum3;
            case '4':       return ButtonType::KeyboardNum4;
            case '5':       return ButtonType::KeyboardNum5;
            case '6':       return ButtonType::KeyboardNum6;
            case '7':       return ButtonType::KeyboardNum7;
            case '8':       return ButtonType::KeyboardNum8;
            case '9':       return ButtonType::KeyboardNum9;
            /* Keyboard F1-F24 */
            case VK_F1:     return ButtonType::KeyboardF1;
            case VK_F2:     return ButtonType::KeyboardF2;
            case VK_F3:     return ButtonType::KeyboardF3;
            case VK_F4:     return ButtonType::KeyboardF4;
            case VK_F5:     return ButtonType::KeyboardF5;
            case VK_F6:     return ButtonType::KeyboardF6;
            case VK_F7:     return ButtonType::KeyboardF7;
            case VK_F8:     return ButtonType::KeyboardF8;
            case VK_F9:     return ButtonType::KeyboardF9;
            case VK_F10:    return ButtonType::KeyboardF10;
            case VK_F11:    return ButtonType::KeyboardF11;
            case VK_F12:    return ButtonType::KeyboardF12;
            case VK_F13:    return ButtonType::KeyboardF13;
            case VK_F14:    return ButtonType::KeyboardF14;
            case VK_F15:    return ButtonType::KeyboardF15;
            case VK_F16:    return ButtonType::KeyboardF16;
            case VK_F17:    return ButtonType::KeyboardF17;
            case VK_F18:    return ButtonType::KeyboardF18;
            case VK_F19:    return ButtonType::KeyboardF19;
            case VK_F20:    return ButtonType::KeyboardF20;
            case VK_F21:    return ButtonType::KeyboardF21;
            case VK_F22:    return ButtonType::KeyboardF22;
            case VK_F23:    return ButtonType::KeyboardF23;
            case VK_F24:    return ButtonType::KeyboardF24;
            /* Keyboard Navigation */
            case VK_LEFT:       return ButtonType::KeyboardArrowLeft;
            case VK_RIGHT:      return ButtonType::KeyboardArrowRight;
            case VK_UP:         return ButtonType::KeyboardArrowUp;
            case VK_DOWN:       return ButtonType::KeyboardArrowDown;
            /* Keyboard NumPad */
            case VK_NUMPAD0:    return ButtonType::KeyboardNumPad0;
            case VK_NUMPAD1:    return ButtonType::KeyboardNumPad1;
            case VK_NUMPAD2:    return ButtonType::KeyboardNumPad2;
            case VK_NUMPAD3:    return ButtonType::KeyboardNumPad3;
            case VK_NUMPAD4:    return ButtonType::KeyboardNumPad4;
            case VK_NUMPAD5:    return ButtonType::KeyboardNumPad5;
            case VK_NUMPAD6:    return ButtonType::KeyboardNumPad6;
            case VK_NUMPAD7:    return ButtonType::KeyboardNumPad7;
            case VK_NUMPAD8:    return ButtonType::KeyboardNumPad8;
            case VK_NUMPAD9:    return ButtonType::KeyboardNumPad9;
            case VK_ADD:        return ButtonType::KeyboardNumPadAdd;
            case VK_DECIMAL:    return ButtonType::KeyboardNumPadDecimal;
            case VK_DIVIDE:     return ButtonType::KeyboardNumPadDivide;
            case VK_MULTIPLY:   return ButtonType::KeyboardNumPadMultiply;
            case VK_SUBTRACT:   return ButtonType::KeyboardNumPadMinus;
            case VK_NUMLOCK:    return ButtonType::KeyboardNumPadLock;
            /* Keyboard Control */
            case VK_ESCAPE:     return ButtonType::KeyboardEscape;
            case VK_TAB:        return ButtonType::KeyboardTab;
            case VK_CAPITAL:    return ButtonType::KeyboardCapital;
            case VK_LSHIFT:     return ButtonType::KeyboardLeftShift;
            case VK_LCONTROL:   return ButtonType::KeyboardLeftCtrl;
            case VK_LWIN:       return ButtonType::KeyboardLeftWindows;
            case VK_LMENU:      return ButtonType::KeyboardLeftAlt;
            case VK_SPACE:      return ButtonType::KeyboardSpace;
            case VK_RMENU:      return ButtonType::KeyboardRightAlt;
            case VK_RWIN:       return ButtonType::KeyboardRightWindows;
            case VK_RCONTROL:   return ButtonType::KeyboardRightCtrl;
            case VK_RSHIFT:     return ButtonType::KeyboardRightShift;
            case VK_BACK:       return ButtonType::KeyboardBackspace;
            case VK_RETURN:
            {
                // Enter key and numpad enter key are same virtual key "VK_RETURN",
                // so we should use "KF_EXTENDED" to distinct them.
                if (HIWORD(lParam) & KF_EXTENDED)
                    return ButtonType::KeyboardNumPadEnter;
                else
                    return ButtonType::KeyboardEnter;
            }
            /* Keyboard Edit */
            case VK_INSERT:     return ButtonType::KeyboardInsert;
            case VK_DELETE:     return ButtonType::KeyboardDelete;
            case VK_PRIOR:      return ButtonType::KeyboardPageUp;
            case VK_NEXT:       return ButtonType::KeyboardPageDown;
            case VK_HOME:       return ButtonType::KeyboardHome;
            case VK_END:        return ButtonType::KeyboardEnd;
            /* Keyboard Sign */
            case VK_OEM_COMMA:  return ButtonType::KeyboardComma;
            case VK_OEM_PERIOD: return ButtonType::KeyboardPeriod;
            case VK_OEM_2:      return ButtonType::KeyboardSlash;
            case VK_OEM_1:      return ButtonType::KeyboardSemicolon;
            case VK_OEM_7:      return ButtonType::KeyboardApostrophe;
            case VK_OEM_4:      return ButtonType::KeyboardLeftBracket;
            case VK_OEM_6:      return ButtonType::KeyboardRightBracket;
            case VK_OEM_5:      return ButtonType::KeyboardBackslash;
            case VK_OEM_3:      return ButtonType::KeyboardGraveAccent;
            case VK_OEM_MINUS:  return ButtonType::KeyboardMinus;
            case VK_OEM_PLUS:   return ButtonType::KeyboardEqual;
            /* Keyboard Other */
            case VK_APPS:               return ButtonType::KeyboardApps;
            case VK_PAUSE:              return ButtonType::KeyboardPause;
            case VK_SNAPSHOT:           return ButtonType::KeyboardPrintScreen;
            case VK_SCROLL:             return ButtonType::KeyboardScrollLock;
            case VK_VOLUME_MUTE:        return ButtonType::KeyboardVolumeMute;
            case VK_VOLUME_UP:          return ButtonType::KeyboardVolumeUp;
            case VK_VOLUME_DOWN:        return ButtonType::KeyboardVolumeDown;
            case VK_MEDIA_PLAY_PAUSE:   return ButtonType::KeyboardMediaPlayPause;
            case VK_MEDIA_STOP:         return ButtonType::KeyboardMediaStop;
            case VK_MEDIA_PREV_TRACK:   return ButtonType::KeyboardMediaPrev;
            case VK_MEDIA_NEXT_TRACK:   return ButtonType::KeyboardMediaNext;
            /* Finish */
            default:        return ButtonType::Unknown;
        }
    }

    // https://stackoverflow.com/questions/15966642/how-do-you-tell-lshift-apart-from-rshift-in-wm-keydown-events
    void* InputService::MapControlKeyLeftAndRight(void* wpara, void* lpara)
    {
        WPARAM wParam = reinterpret_cast<WPARAM>(wpara);
        LPARAM lParam = reinterpret_cast<LPARAM>(lpara);

        UINT scancode = (lParam & 0x00ff0000) >> 16;
        int extended  = (lParam & 0x01000000) != 0;

        WPARAM newWParam = wParam;
        switch (wParam)
        {
            case VK_SHIFT:
                newWParam = ::MapVirtualKeyW(scancode, MAPVK_VSC_TO_VK_EX);
                break;
            case VK_CONTROL:
                newWParam = extended ? VK_RCONTROL : VK_LCONTROL;
                break;
            case VK_MENU:
                newWParam = extended ? VK_RMENU : VK_LMENU;
                break;
            default:
                break;
        }

        return reinterpret_cast<void*>(newWParam);
    }
}
