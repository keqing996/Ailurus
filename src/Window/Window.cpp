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
