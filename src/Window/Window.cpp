#include "Ailurus/Window/Window.h"

#if AILURUS_FEAT_SUPPORT_WINDOW

#include <unordered_set>
#include <SDL3/SDL.h>

namespace Ailurus
{
    class NativeWindowUtility
    {
        static bool IsSameWindow(const Window& window, const SDL_WindowEvent& sdlWindowEvent)
        {
            return SDL_GetWindowID(static_cast<SDL_Window*>(window._pWindow)) == sdlWindowEvent.windowID;
        }

    public:
        static void HandleEvent(Window& window, const SDL_Event& event, bool* quitLoop)
        {
            switch (event.type)
            {
                case SDL_EVENT_QUIT:
                {
                    if (window._ignoreNextQuit)
                        window._ignoreNextQuit = false;
                    else
                        *quitLoop = true;
                    break;
                }
                case SDL_EVENT_WINDOW_DESTROYED:
                {
                    if (IsSameWindow(window, event.window))
                        *quitLoop = true;

                    break;
                }
                case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                {
                    if (IsSameWindow(window, event.window))
                    {
                        if (window._onWindowTryToClose != nullptr && !window._onWindowTryToClose())
                            window._ignoreNextQuit = true;
                    }

                    break;
                }
                case SDL_EVENT_WINDOW_MOVED:
                {
                    if (IsSameWindow(window, event.window))
                    {
                        if (window._onWindowMoved)
                            window._onWindowMoved(event.window.data1, event.window.data2);
                    }

                    break;
                }
                case SDL_EVENT_WINDOW_RESIZED:
                {
                    if (IsSameWindow(window, event.window))
                    {
                        if (window._onWindowResize)
                            window._onWindowResize(event.window.data1, event.window.data2);
                    }

                    break;
                }
                case SDL_EVENT_WINDOW_FOCUS_GAINED:
                {
                    if (IsSameWindow(window, event.window))
                    {
                        if (window._onWindowFocusChanged)
                            window._onWindowFocusChanged(true);
                    }

                    break;
                }
                case SDL_EVENT_WINDOW_FOCUS_LOST:
                {
                    if (IsSameWindow(window, event.window))
                    {
                        if (window._onWindowFocusChanged)
                            window._onWindowFocusChanged(false);
                    }

                    break;
                }
                case SDL_EVENT_WINDOW_MOUSE_ENTER:
                {
                    if (IsSameWindow(window, event.window))
                    {
                        if (window._onWindowCursorEnteredOrLeaved)
                            window._onWindowCursorEnteredOrLeaved(true);
                    }

                    break;
                }
                case SDL_EVENT_WINDOW_MOUSE_LEAVE:
                {
                    if (IsSameWindow(window, event.window))
                    {
                        if (window._onWindowCursorEnteredOrLeaved)
                            window._onWindowCursorEnteredOrLeaved(false);
                    }
                    break;
                }
                default:
                    break;
            }
        }
    };

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
            while(true)
            {
                SDL_Event event;
                if (!SDL_PollEvent(&event))
                    break;

                bool closeWindow = false;
                NativeWindowUtility::HandleEvent(*this, event, &closeWindow);

                if (closeWindow)
                    shouldBreakLoop = true;
            }

            if (shouldBreakLoop)
                break;

            if (loopFunction != nullptr)
                loopFunction();
        }

        Destroy();
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
/*
    Service* Window::GetServiceInternal(ServiceType type)
    {
        auto itr = _serviceMap.find(type);
        if (itr == _serviceMap.end())
            return nullptr;

        return itr->second;
    }

    bool Window::AddServiceInternal(ServiceType type)
    {
        auto itr = _serviceMap.find(type);
        if (itr != _serviceMap.end())
            return true;

        auto pDependentService = Service::GetServiceDependent(type);
        if (pDependentService != nullptr)
        {
            for (auto dependentServiceType: *pDependentService)
            {
                if (!AddServiceInternal(dependentServiceType))
                    return false;
            }
        }

        Service* service = nullptr;

        switch (type)
        {
            case ServiceType::Input:
                service = new InputService(this);
                break;
            case ServiceType::OpenGL:
                service = new OpenGLService(this);
                break;
            case ServiceType::ImGuiOpenGL:
                service = new ImGuiOpenGLService(this);
                break;
        }

        if (service != nullptr)
        {
            _serviceMap[type] = service;
            _servicesInCreationOrder.push_back(service);
        }

        return service != nullptr;
    }

    bool Window::CanServiceBeAdded(ServiceType type)
    {
        std::unordered_set<ServiceType> tempSet;

        std::function<void(ServiceType)> CollectConflict = [&](ServiceType type)
        {
            auto pConflictServices = Service::GetServiceConflict(type);
            if (pConflictServices != nullptr)
            {
                for (auto conflictServiceType: *pConflictServices)
                {
                    tempSet.insert(conflictServiceType);
                    CollectConflict(conflictServiceType);
                }
            }
        };

        CollectConflict(type);

        for (auto conflictType: tempSet)
        {
            if (GetServiceInternal(conflictType) != nullptr)
                return false;
        }

        return true;
    }

    const std::vector<Service*>& Window::GetServices()
    {
        return _servicesInCreationOrder;
    }

    void Window::ClearService()
    {
        auto itr = _servicesInCreationOrder.rbegin();
        for (; itr != _servicesInCreationOrder.rend(); ++itr)
            delete *itr;

        _servicesInCreationOrder.clear();
        _serviceMap.clear();
    }
    */
}

#endif
