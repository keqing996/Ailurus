#include "Ailurus/Window/Window.h"

#if AILURUS_FEAT_SUPPORT_WINDOW

#include <SDL3/SDL_main.h>

namespace Ailurus
{
    Window::Window()
    {
    }

    Window::~Window()
    {
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
        if (style.canMin)
            flags |= SDL_WINDOW_MAXIMIZED;
        if (style.canMax)
            flags |= SDL_WINDOW_MINIMIZED;

        _pWindow = SDL_CreateWindow(title.c_str(), width, height, flags);
        if (_pWindow == nullptr)
            return false;

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

            ClearServices();
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
        SDL_Event e;
        while(_running)
        {
            while(SDL_PollEvent(&e)) {
                switch(e.type)
                {
                    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                        if (!_onWindowTryToClose || _onWindowTryToClose()) {
                            _running = false;
                        }
                    break;

                    case SDL_EVENT_WINDOW_MOVED:
                        if(_onWindowMoved)
                            _onWindowMoved(e.window.data1, e.window.data2);
                    break;

                    case SDL_EVENT_WINDOW_RESIZED:
                        if(_onWindowResize)
                            _onWindowResize(e.window.data1, e.window.data2);
                    break;

                    case SDL_EVENT_WINDOW_FOCUS_GAINED:
                        if(_onWindowFocusChanged) _onWindowFocusChanged(true);
                    break;

                    case SDL_EVENT_WINDOW_FOCUS_LOST:
                        if(_onWindowFocusChanged) _onWindowFocusChanged(false);
                    break;

                    // 处理鼠标进入/离开事件
                    case SDL_EVENT_MOUSE_ENTER:
                        _cursorInsideWindow = true;
                    if(_onWindowCursorEnteredOrLeaved) _onWindowCursorEnteredOrLeaved(true);
                    break;

                    case SDL_EVENT_MOUSE_LEAVE:
                        _cursorInsideWindow = false;
                    if(_onWindowCursorEnteredOrLeaved) _onWindowCursorEnteredOrLeaved(false);
                    break;
                }
            }

            if(loopFunction)
                loopFunction();
        }
    }

    std::pair<int, int> Window::GetSize() const
    {
        if (_pWindow != nullptr)
        {
            int w, h;
            SDL_GetWindowSize(static_cast<SDL_Window*>(_pWindow), &w, &h);
            return {w, h};
        }

        return {0, 0};
    }

    void Window::SetSize(int width, int height)
    {
        if (_pWindow != nullptr)
            SDL_SetWindowSize(static_cast<SDL_Window*>(_pWindow), width, height);
    }

    std::pair<int, int> Window::GetPosition() const
    {
        if (_pWindow == nullptr)
            return {0, 0};

        int x, y;
        SDL_GetWindowPosition(static_cast<SDL_Window*>(_pWindow), &x, &y);
        return {x, y};
    }

    void Window::SetPosition(int x, int y)
    {
        if (_pWindow == nullptr)
            return;

        SDL_SetWindowPosition(static_cast<SDL_Window*>(_pWindow), x, y);
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

    void Window::SetCallbackOnWindowMessagePreProcess(const std::function<bool(uint32_t, void*, void*, int*)>& callback)
    {
        _onWindowMessagePreProcess = callback;
    }

    void Window::SetCallbackOnWindowMoved(const std::function<void(int, int)>& callback)
    {
        _onWindowMoved = callback;
    }

    void Window::SetCallbackOnWindowResize(const std::function<void(int, int)>& callback)
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
}

#endif
