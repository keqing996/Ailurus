#include "Ailurus/Window/Window.h"

#if AILURUS_FEAT_SUPPORT_WINDOW

#include <unordered_set>
#include <SDL3/SDL.h>

namespace Ailurus
{
    static ButtonType GetButtonFromSDL(const SDL_MouseButtonEvent& buttonEvent)
    {
        switch (buttonEvent.button)
        {
            case SDL_BUTTON_LEFT:
                return ButtonType::MouseLeft;
            case SDL_BUTTON_RIGHT:
                return ButtonType::MouseRight;
            case SDL_BUTTON_MIDDLE:
                return ButtonType::MouseMiddle;
            case SDL_BUTTON_X1:
                return ButtonType::MouseXButton1;
            case SDL_BUTTON_X2:
                return ButtonType::MouseXButton2;
            default:
                return ButtonType::Unknown;;
        }
    }

    static ButtonType GetButtonFromSDL(const SDL_KeyboardEvent& keyEvent)
    {
        switch (keyEvent.scancode)
        {
            case SDL_SCANCODE_KP_0:         return ButtonType::KeyboardNumPad0;
            case SDL_SCANCODE_KP_1:         return ButtonType::KeyboardNumPad1;
            case SDL_SCANCODE_KP_2:         return ButtonType::KeyboardNumPad2;
            case SDL_SCANCODE_KP_3:         return ButtonType::KeyboardNumPad3;
            case SDL_SCANCODE_KP_4:         return ButtonType::KeyboardNumPad4;
            case SDL_SCANCODE_KP_5:         return ButtonType::KeyboardNumPad5;
            case SDL_SCANCODE_KP_6:         return ButtonType::KeyboardNumPad6;
            case SDL_SCANCODE_KP_7:         return ButtonType::KeyboardNumPad7;
            case SDL_SCANCODE_KP_8:         return ButtonType::KeyboardNumPad8;
            case SDL_SCANCODE_KP_9:         return ButtonType::KeyboardNumPad9;
            case SDL_SCANCODE_KP_PLUS:      return ButtonType::KeyboardNumPadAdd;
            case SDL_SCANCODE_KP_MINUS:     return ButtonType::KeyboardNumPadMinus;
            case SDL_SCANCODE_KP_MULTIPLY:  return ButtonType::KeyboardNumPadMultiply;
            case SDL_SCANCODE_KP_DIVIDE:    return ButtonType::KeyboardNumPadDivide;
            case SDL_SCANCODE_KP_PERIOD:    return ButtonType::KeyboardNumPadDecimal;
            case SDL_SCANCODE_KP_ENTER:     return ButtonType::KeyboardNumPadEnter;
            default: break;
        }

        switch (keyEvent.key)
        {
            /* Keyboard A-Z */
            case SDLK_A:    return ButtonType::KeyboardA;
            case SDLK_B:    return ButtonType::KeyboardB;
            case SDLK_C:    return ButtonType::KeyboardC;
            case SDLK_D:    return ButtonType::KeyboardD;
            case SDLK_E:    return ButtonType::KeyboardE;
            case SDLK_F:    return ButtonType::KeyboardF;
            case SDLK_G:    return ButtonType::KeyboardG;
            case SDLK_H:    return ButtonType::KeyboardH;
            case SDLK_I:    return ButtonType::KeyboardI;
            case SDLK_J:    return ButtonType::KeyboardJ;
            case SDLK_K:    return ButtonType::KeyboardK;
            case SDLK_L:    return ButtonType::KeyboardL;
            case SDLK_M:    return ButtonType::KeyboardM;
            case SDLK_N:    return ButtonType::KeyboardN;
            case SDLK_O:    return ButtonType::KeyboardO;
            case SDLK_P:    return ButtonType::KeyboardP;
            case SDLK_Q:    return ButtonType::KeyboardQ;
            case SDLK_R:    return ButtonType::KeyboardR;
            case SDLK_S:    return ButtonType::KeyboardS;
            case SDLK_T:    return ButtonType::KeyboardT;
            case SDLK_U:    return ButtonType::KeyboardU;
            case SDLK_V:    return ButtonType::KeyboardV;
            case SDLK_W:    return ButtonType::KeyboardW;
            case SDLK_X:    return ButtonType::KeyboardX;
            case SDLK_Y:    return ButtonType::KeyboardY;
            case SDLK_Z:    return ButtonType::KeyboardZ;
            /* Keyboard Num */
            case SDLK_0:    return ButtonType::KeyboardNum0;
            case SDLK_1:    return ButtonType::KeyboardNum1;
            case SDLK_2:    return ButtonType::KeyboardNum2;
            case SDLK_3:    return ButtonType::KeyboardNum3;
            case SDLK_4:    return ButtonType::KeyboardNum4;
            case SDLK_5:    return ButtonType::KeyboardNum5;
            case SDLK_6:    return ButtonType::KeyboardNum6;
            case SDLK_7:    return ButtonType::KeyboardNum7;
            case SDLK_8:    return ButtonType::KeyboardNum8;
            case SDLK_9:    return ButtonType::KeyboardNum9;
            /* Keyboard F1-F24 */
            case SDLK_F1:   return ButtonType::KeyboardF1;
            case SDLK_F2:   return ButtonType::KeyboardF2;
            case SDLK_F3:   return ButtonType::KeyboardF3;
            case SDLK_F4:   return ButtonType::KeyboardF4;
            case SDLK_F5:   return ButtonType::KeyboardF5;
            case SDLK_F6:   return ButtonType::KeyboardF6;
            case SDLK_F7:   return ButtonType::KeyboardF7;
            case SDLK_F8:   return ButtonType::KeyboardF8;
            case SDLK_F9:   return ButtonType::KeyboardF9;
            case SDLK_F10:  return ButtonType::KeyboardF10;
            case SDLK_F11:  return ButtonType::KeyboardF11;
            case SDLK_F12:  return ButtonType::KeyboardF12;
            case SDLK_F13:  return ButtonType::KeyboardF13;
            case SDLK_F14:  return ButtonType::KeyboardF14;
            case SDLK_F15:  return ButtonType::KeyboardF15;
            case SDLK_F16:  return ButtonType::KeyboardF16;
            case SDLK_F17:  return ButtonType::KeyboardF17;
            case SDLK_F18:  return ButtonType::KeyboardF18;
            case SDLK_F19:  return ButtonType::KeyboardF19;
            case SDLK_F20:  return ButtonType::KeyboardF20;
            case SDLK_F21:  return ButtonType::KeyboardF21;
            case SDLK_F22:  return ButtonType::KeyboardF22;
            case SDLK_F23:  return ButtonType::KeyboardF23;
            case SDLK_F24:  return ButtonType::KeyboardF24;
            /* Keyboard Navigation */
            case SDLK_LEFT: return ButtonType::KeyboardArrowLeft;
            case SDLK_RIGHT:return ButtonType::KeyboardArrowRight;
            case SDLK_UP:   return ButtonType::KeyboardArrowUp;
            case SDLK_DOWN: return ButtonType::KeyboardArrowDown;
            /* Keyboard NumPad -> Except NumLock, other by scancode */
            case SDLK_NUMLOCKCLEAR: return ButtonType::KeyboardNumPadLock;
            /* Keyboard Control */
            case SDLK_ESCAPE:       return ButtonType::KeyboardEscape;
            case SDLK_TAB:          return ButtonType::KeyboardTab;
            case SDLK_CAPSLOCK:     return ButtonType::KeyboardCapital;
            case SDLK_LSHIFT:       return ButtonType::KeyboardLeftShift;
            case SDLK_LCTRL:        return ButtonType::KeyboardLeftCtrl;
            case SDLK_LGUI:         return ButtonType::KeyboardLeftSuper;
            case SDLK_LALT:         return ButtonType::KeyboardLeftAlt;
            case SDLK_SPACE:        return ButtonType::KeyboardSpace;
            case SDLK_RALT:         return ButtonType::KeyboardRightAlt;
            case SDLK_RGUI:         return ButtonType::KeyboardRightSuper;
            case SDLK_RCTRL:        return ButtonType::KeyboardRightCtrl;
            case SDLK_RSHIFT:       return ButtonType::KeyboardRightShift;
            case SDLK_BACKSPACE:    return ButtonType::KeyboardBackspace;
            case SDLK_RETURN:       return ButtonType::KeyboardEnter;
            /* Keyboard Edit */
            case SDLK_INSERT:       return ButtonType::KeyboardInsert;
            case SDLK_DELETE:       return ButtonType::KeyboardDelete;
            case SDLK_PAGEUP:       return ButtonType::KeyboardPageUp;
            case SDLK_PAGEDOWN:     return ButtonType::KeyboardPageDown;
            case SDLK_HOME:         return ButtonType::KeyboardHome;
            case SDLK_END:          return ButtonType::KeyboardEnd;
            /* Keyboard Sign */
            case SDLK_COMMA:        return ButtonType::KeyboardComma;
            case SDLK_PERIOD:       return ButtonType::KeyboardPeriod;
            case SDLK_SLASH:        return ButtonType::KeyboardSlash;
            case SDLK_SEMICOLON:    return ButtonType::KeyboardSemicolon;
            case SDLK_APOSTROPHE:   return ButtonType::KeyboardApostrophe;
            case SDLK_LEFTBRACKET:  return ButtonType::KeyboardLeftBracket;
            case SDLK_RIGHTBRACKET: return ButtonType::KeyboardRightBracket;
            case SDLK_BACKSLASH:    return ButtonType::KeyboardBackslash;
            case SDLK_GRAVE:        return ButtonType::KeyboardGraveAccent;
            case SDLK_MINUS:        return ButtonType::KeyboardMinus;
            case SDLK_EQUALS:       return ButtonType::KeyboardEqual;
            /* Keyboard Other */
            case SDLK_APPLICATION:          return ButtonType::KeyboardApps;
            case SDLK_PAUSE:                return ButtonType::KeyboardPause;
            case SDLK_PRINTSCREEN:          return ButtonType::KeyboardPrintScreen;
            case SDLK_SCROLLLOCK:           return ButtonType::KeyboardScrollLock;
            case SDLK_MUTE:                 return ButtonType::KeyboardVolumeMute;
            case SDLK_VOLUMEUP:             return ButtonType::KeyboardVolumeUp;
            case SDLK_VOLUMEDOWN:           return ButtonType::KeyboardVolumeDown;
            case SDLK_MEDIA_PLAY_PAUSE:     return ButtonType::KeyboardMediaPlayPause;
            case SDLK_MEDIA_STOP:           return ButtonType::KeyboardMediaStop;
            case SDLK_MEDIA_PREVIOUS_TRACK: return ButtonType::KeyboardMediaPrev;
            case SDLK_MEDIA_NEXT_TRACK:     return ButtonType::KeyboardMediaNext;
            /* Finish */
            default:        return ButtonType::Unknown;
        }
    }

    Window::Window() = default;

    Window::~Window()
    {
        Destroy();
    }

    bool Window::Create(int width, int height, const std::string& title, WindowStyle style)
    {
        if (!SDL_Init(SDL_INIT_VIDEO))
            return false;

        SDL_WindowFlags flags = 0;
        if (!style.haveBorder)
            flags |= SDL_WINDOW_BORDERLESS;
        if (style.canResize)
            flags |= SDL_WINDOW_RESIZABLE;

        _pWindow = SDL_CreateWindow(title.c_str(), width, height, flags);
        if (_pWindow == nullptr)
            return false;

        float x, y;
        SDL_GetMouseState(&x, &y);
        _mousePos = Vector2f(x, y);

        if (_onWindowCreated != nullptr)
            _onWindowCreated();

        return true;
    }

    void Window::Destroy()
    {
        if (_pWindow)
        {
            if (_onWindowPreDestroyed)
                _onWindowPreDestroyed();

            //ClearServices();
            SDL_DestroyWindow(static_cast<SDL_Window*>(_pWindow));
            _pWindow = nullptr;

            if (_onWindowPostDestroyed)
                _onWindowPostDestroyed();
        }
    }

    bool Window::IsWindowValid() const
    {
        return _pWindow != nullptr;
    }

    void Window::Loop(const std::function<void()>& loopFunction)
    {
        while(true)
        {
            bool shouldBreakLoop = false;
            EventLoop(&shouldBreakLoop);

            if (shouldBreakLoop)
                break;

            if (loopFunction != nullptr)
                loopFunction();
        }

        Destroy();
    }

    Vector2i Window::GetSize() const
    {
        if (_pWindow != nullptr)
        {
            int w, h;
            SDL_GetWindowSize(static_cast<SDL_Window*>(_pWindow), &w, &h);
            return Vector2i(w, h);
        }

        return Vector2i(0, 0);
    }

    void Window::SetSize(int width, int height)
    {
        if (_pWindow != nullptr)
            SDL_SetWindowSize(static_cast<SDL_Window*>(_pWindow), width, height);
    }

    void Window::SetSize(const Vector2i& size)
    {
        SetSize(size.x(), size.y());
    }

    Vector2i Window::GetPosition() const
    {
        if (_pWindow == nullptr)
            return Vector2i(0, 0);

        int x, y;
        SDL_GetWindowPosition(static_cast<SDL_Window*>(_pWindow), &x, &y);
        return Vector2i(x, y);
    }

    void Window::SetPosition(int x, int y)
    {
        if (_pWindow == nullptr)
            return;

        SDL_SetWindowPosition(static_cast<SDL_Window*>(_pWindow), x, y);
    }

    void Window::SetPosition(const Vector2i& pos)
    {
        SetPosition(pos.x(), pos.y());
    }

    void Window::SetIcon(unsigned int width, unsigned int height, const std::byte* pixels)
    {
        if (_pWindow == nullptr)
            return;

        auto surface = SDL_CreateSurfaceFrom(width, height, SDL_PIXELFORMAT_RGBA8888,
            const_cast<std::byte*>(pixels), width * 4);
        SDL_SetWindowIcon(static_cast<SDL_Window*>(_pWindow), surface);
    }

    void Window::SetTitle(const std::string& title)
    {
        if (_pWindow != nullptr)
            SDL_SetWindowTitle(static_cast<SDL_Window*>(_pWindow), title.c_str());
    }

    void Window::SetWindowVisible(bool show)
    {
        if (_pWindow == nullptr)
            return;

        if (show)
            SDL_ShowWindow(static_cast<SDL_Window*>(_pWindow));
        else
            SDL_HideWindow(static_cast<SDL_Window*>(_pWindow));
    }

    bool Window::IsCursorVisible() const
    {
        return SDL_CursorVisible();
    }

    void Window::SetCursorVisible(bool show)
    {
        if (show)
            SDL_ShowCursor();
        else
            SDL_HideCursor();
    }

    bool Window::IsCursorLimitedInWindow() const
    {
        if (_pWindow == nullptr)
            return false;

        return SDL_GetWindowMouseGrab(static_cast<SDL_Window*>(_pWindow));
    }

    void Window::SetCursorLimitedInWindow(bool capture)
    {
        if (_pWindow == nullptr)
            return;

        SDL_SetWindowMouseGrab(static_cast<SDL_Window*>(_pWindow), capture);
    }

    bool Window::IsCursorInsideWindow() const
    {
        if (_pWindow == nullptr)
            return false;

        int windowX, windowY;
        SDL_GetWindowPosition(static_cast<SDL_Window*>(_pWindow), &windowX, &windowY);

        int windowWidth, windowHeight;
        SDL_GetWindowSize(static_cast<SDL_Window*>(_pWindow), &windowWidth, &windowHeight);

        float mouseX, mouseY;
        SDL_GetGlobalMouseState(&mouseX, &mouseY);

        return (mouseX >= windowX &&
                mouseX <= windowX + windowWidth &&
                mouseY >= windowY &&
                mouseY <= windowY + windowHeight);
    }

    void Window::SetCallbackOnWindowCreated(const std::function<void()>& callback)
    {
        _onWindowCreated = callback;
    }

    void Window::SetCallbackOnWindowTryToClose(const std::function<bool()>& callback)
    {
        _onWindowTryToClose = callback;
    }

    void Window::SetCallbackOnWindowClosed(const std::function<void()>& callback)
    {
        _onWindowClosed = callback;
    }

    void Window::SetCallbackOnWindowPreDestroyed(const std::function<void()>& callback)
    {
        _onWindowPreDestroyed = callback;
    }

    void Window::SetCallbackOnWindowPostDestroyed(const std::function<void()>& callback)
    {
        _onWindowPostDestroyed = callback;
    }

    void Window::SetCallbackOnWindowMoved(const std::function<void(Vector2i)>& callback)
    {
        _onWindowMoved = callback;
    }

    void Window::SetCallbackOnWindowResize(const std::function<void(Vector2i)>& callback)
    {
        _onWindowResize = callback;
    }

    void Window::SetCallbackOnWindowFocusChanged(const std::function<void(bool)>& callback)
    {
        _onWindowFocusChanged = callback;
    }

    void Window::SetCallbackOnWindowCursorEnteredOrLeaved(const std::function<void(bool)>& callback)
    {
        _onWindowCursorEnteredOrLeaved = callback;
    }

    void Window::SetCallbackOnWindowCursorVisibleChanged(const std::function<void(bool)>& callback)
    {
        _onWindowCursorVisibleChanged = callback;
    }

    bool Window::IsButtonPressed(ButtonType key) const
    {
        return _pressedButton.find(key) != _pressedButton.end();
    }

    Vector2f Window::GetMousePosition() const
    {
        return _mousePos;
    }

    Vector2f Window::GetMouseWheel() const
    {
        return _mouseWheel;
    }

    bool Window::GetIsAutoRepeat() const
    {
        return _enableAutoRepeat;
    }

    void Window::SetIsAutoRepeat(bool autoRepeat)
    {
        _enableAutoRepeat = autoRepeat;
    }

    void Window::SetCallbackOnMouseMove(const std::function<void(Vector2f, Vector2f)>& fun)
    {
        _onMouseMove = fun;
    }

    void Window::SetCallbackOnMouseWheel(const std::function<void(Vector2f)>& fun)
    {
        _onMouseWheel = fun;
    }

    void Window::SetCallbackOnButtonPressed(const std::function<void(ButtonType)>& fun)
    {
        _onButtonPressed = fun;
    }

    void Window::SetCallbackOnButtonReleased(const std::function<void(ButtonType)>& fun)
    {
        _onButtonReleased = fun;
    }

    void Window::EventLoop(bool* quitLoop)
    {
        // Record
        Vector2f recordMousePos = _mousePos;

        // Clear
        _mouseWheel = Vector2f(0, 0);

        while(true)
        {
            SDL_Event event;
            if (!SDL_PollEvent(&event))
                break;

            bool closeWindow = false;
            HandleEvent(&event, &closeWindow);

            if (closeWindow)
                *quitLoop = true;
        }

        // Emit callback
        if (_mousePos != recordMousePos && _onMouseMove != nullptr)
            _onMouseMove(recordMousePos, _mousePos);

        if (_mouseWheel != Vector2f(0, 0))
            _onMouseWheel(_mouseWheel);
    }

    void Window::HandleEvent(const void* pEvent, bool* quitLoop)
    {
        const SDL_Event* pSDLEvent = static_cast<const SDL_Event*>(pEvent);
        const SDL_WindowID windowId = SDL_GetWindowID(static_cast<SDL_Window*>(_pWindow));

        switch (pSDLEvent->type)
        {
            case SDL_EVENT_QUIT:
            {
                if (_ignoreNextQuit)
                    _ignoreNextQuit = false;
                else
                    *quitLoop = true;
                break;
            }
            case SDL_EVENT_WINDOW_DESTROYED:
            {
                if (windowId == pSDLEvent->window.windowID)
                    *quitLoop = true;

                break;
            }
            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            {
                if (windowId == pSDLEvent->window.windowID
                    && _onWindowTryToClose != nullptr && !_onWindowTryToClose())
                {
                        _ignoreNextQuit = true;
                }

                break;
            }
            case SDL_EVENT_WINDOW_MOVED:
            {
                if (windowId == pSDLEvent->window.windowID && _onWindowMoved != nullptr)
                    _onWindowMoved(Vector2i(pSDLEvent->window.data1, pSDLEvent->window.data2));

                break;
            }
            case SDL_EVENT_WINDOW_RESIZED:
            {
                if (windowId == pSDLEvent->window.windowID && _onWindowResize != nullptr)
                    _onWindowResize(Vector2i(pSDLEvent->window.data1, pSDLEvent->window.data2));

                break;
            }
            case SDL_EVENT_WINDOW_FOCUS_GAINED:
            {
                if (windowId == pSDLEvent->window.windowID && _onWindowFocusChanged != nullptr)
                    _onWindowFocusChanged(true);

                break;
            }
            case SDL_EVENT_WINDOW_FOCUS_LOST:
            {
                if (windowId == pSDLEvent->window.windowID && _onWindowFocusChanged != nullptr)
                    _onWindowFocusChanged(false);

                break;
            }
            case SDL_EVENT_WINDOW_MOUSE_ENTER:
            {
                if (windowId == pSDLEvent->window.windowID && _onWindowCursorEnteredOrLeaved != nullptr)
                    _onWindowCursorEnteredOrLeaved(true);

                break;
            }
            case SDL_EVENT_WINDOW_MOUSE_LEAVE:
            {
                if (windowId == pSDLEvent->window.windowID && _onWindowCursorEnteredOrLeaved != nullptr)
                    _onWindowCursorEnteredOrLeaved(false);

                break;
            }
            case SDL_EVENT_MOUSE_MOTION:
            {
                if (windowId == pSDLEvent->motion.windowID)
                    _mousePos = Vector2f(pSDLEvent->motion.x, pSDLEvent->motion.y);

                break;
            }
            case SDL_EVENT_MOUSE_WHEEL:
            {
                if (windowId == pSDLEvent->motion.windowID)
                {
                    _mouseWheel += Vector2f(
                        -pSDLEvent->wheel.mouse_x,
                        pSDLEvent->wheel.mouse_y);
                }
            }
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                if (windowId == pSDLEvent->button.windowID)
                {
                    auto buttonType = GetButtonFromSDL(pSDLEvent->button);
                    OnEventButtonPressed(buttonType);
                }

                break;
            }
            case SDL_EVENT_MOUSE_BUTTON_UP:
            {
                if (windowId == pSDLEvent->button.windowID)
                {
                    auto buttonType = GetButtonFromSDL(pSDLEvent->button);
                    OnEventButtonReleased(buttonType);
                }
                break;
            }
            case SDL_EVENT_KEY_DOWN:
            {
                if (windowId == pSDLEvent->key.windowID)
                {
                    auto buttonType = GetButtonFromSDL(pSDLEvent->key);
                    OnEventButtonPressed(buttonType);
                }
                break;
            }
            case SDL_EVENT_KEY_UP:
            {
                if (windowId == pSDLEvent->key.windowID)
                {
                    auto buttonType = GetButtonFromSDL(pSDLEvent->key);
                    OnEventButtonReleased(buttonType);
                }
                break;
            }
            default:
                break;
        }
    }

    void Window::OnEventButtonPressed(ButtonType button)
    {
        if (_pressedButton.find(button) == _pressedButton.end())
        {
            _pressedButton.insert(button);
            if (_onButtonPressed != nullptr)
                _onButtonPressed(button);
        }
        else
        {
            if (_enableAutoRepeat && _onButtonPressed != nullptr)
                _onButtonPressed(button);
        }
    }

    void Window::OnEventButtonReleased(ButtonType button)
    {
        if (auto itr = _pressedButton.find(button); itr != _pressedButton.end())
        {
            _pressedButton.erase(itr);
            if (_onButtonReleased != nullptr)
                _onButtonReleased(button);
        }
    }
}

#endif
