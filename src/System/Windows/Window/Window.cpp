
#include <unordered_set>
#include <iostream>
#include <vector>

#include "Ailurus/Platform/Windows/Window/Utility/WindowsInclude.h"
#include "Ailurus/Platform/Windows/Window/Utility/String.h"
#include "Ailurus/Platform/Windows/Window/Window.h"
#include "NativeWindowUtility.h"
#include "Ailurus/Platform/Windows/Window/Service/InputService/InputService.h"
#include "Ailurus/Platform/Windows/Window/Service/OpenGLService/OpenGLService.h"
#include "Ailurus/Platform/Windows/Window/Service/ImGuiService/ImGuiOpenGLService.h"

#undef CreateService

// Serval paths of window destroyed.
// 1. Window receive WM_CLOSE, pressed close button for example.
//    OnWindowClose -> DestroyWindow -> WM_DESTROY -> OnWindowDestroy
// 2. Call Window::Clear
//    Window::Clear -> DestroyWindow -> WM_DESTROY -> OnWindowDestroy

namespace Ailurus
{
    static int gGlobalWindowsCount = 0;
    static const wchar_t* gWindowRegisterName = L"NativeWindow";

    Window::Window()
    {
        NativeWindowUtility::FixProcessDpi();
    }

    Window::~Window()
    {
        Destroy();
    }

    bool Window::Create(int width, int height, const std::string& title, WindowStyle style)
    {
        // Register window
        if (gGlobalWindowsCount == 0)
            RegisterWindowClass();

        // Get create style
        DWORD win32Style = WS_VISIBLE;
        if (style.type == WindowStyle::Type::Popup)
            win32Style |= WS_POPUP;
        else
        {
            if (style.config.overlapped.haveTitleBar)
                win32Style |= WS_CAPTION | WS_MINIMIZEBOX;
            if (style.config.overlapped.haveResize)
                win32Style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
            if (style.config.overlapped.haveClose)
                win32Style |= WS_SYSMENU;
        }

        // Calculate create position: left & top
        const HDC screenDC = ::GetDC(nullptr);
        const int left = (::GetDeviceCaps(screenDC, HORZRES) - width) / 2;
        const int top = (::GetDeviceCaps(screenDC, VERTRES) - height) / 2;
        ::ReleaseDC(nullptr, screenDC);

        // Adjust create size
        auto [adjustWidth, adjustHeight] = NativeWindowUtility::CalculateAdjustWindowSize(width, height, win32Style);

        // Create window
        const auto titleInWideStr = Utility::StringToWideString(title);
        const wchar_t* titleWide = titleInWideStr.c_str();
        auto hWindow = ::CreateWindowW(
                gWindowRegisterName,
                titleWide,
                win32Style,
                left,
                top,
                adjustWidth,
                adjustHeight,
                nullptr,
                nullptr,
                ::GetModuleHandleW(nullptr),
                this);

        if (!hWindow)
            return false;

        // Global counting
        gGlobalWindowsCount++;

        // Create window state
        _hWindow = hWindow;
        lastWidth = width;
        lastHeight = height;

        // Cursor
        hCursor = ::LoadCursor(nullptr, IDC_ARROW);
        cursorVisible = true;
        SetWindowVisible(true);
        cursorLimitedInWindow = false;
        SetCursorLimitedInWindow(false);
        cursorInsideWindow = CalculateMouseInsideWindow();

        // Set track mouse leave
        SetTrackMouseLeave(true);

        // Set size again after window creation to avoid some bug.
        SetSize(width, height);

        // Callback
        if (_onWindowCreated != nullptr)
            _onWindowCreated();

        return true;
    }

    void Window::Destroy()
    {
        if (_hWindow == nullptr)
            return;

        // Destroy window
        ::DestroyWindow(GetWindowHandle<HWND>());
    }

    bool Window::IsWindowValid() const
    {
        return _hWindow != nullptr;
    }

    void Window::RegisterWindowClass()
    {
        WNDCLASSW windowClass {};
        windowClass.style         = 0;
        windowClass.lpfnWndProc   = &NativeWindowUtility::DefaultWndProc;
        windowClass.cbClsExtra    = 0;
        windowClass.cbWndExtra    = 0;
        windowClass.hInstance     = ::GetModuleHandleW(nullptr);
        windowClass.hIcon         = nullptr;
        windowClass.hCursor       = nullptr;
        windowClass.hbrBackground = nullptr;
        windowClass.lpszMenuName  = nullptr;
        windowClass.lpszClassName = gWindowRegisterName;
        ::RegisterClassW(&windowClass);
    }

    void Window::UnRegisterWindowClass()
    {
        ::UnregisterClassW(gWindowRegisterName, GetModuleHandleW(nullptr));
    }

    void Window::SetSize(int width, int height)
    {
        if (_hWindow == nullptr)
            return;

        const HWND hWnd = GetWindowHandle<HWND>();
        const DWORD dwStyle = static_cast<DWORD>(::GetWindowLongPtrW(hWnd, GWL_STYLE));
        auto [adjustWidth, adjustHeight] = NativeWindowUtility::CalculateAdjustWindowSize(width, height, dwStyle);
        ::SetWindowPos(hWnd, nullptr, 0, 0, adjustWidth, adjustHeight, SWP_NOMOVE | SWP_NOZORDER);
    }

    void* Window::GetWindowHandle() const
    {
        return _hWindow;
    }

    void Window::SetIcon(unsigned int width, unsigned int height, const std::byte* pixels)
    {
        if (_hWindow == nullptr)
            return;

        if (hIcon != nullptr)
            ::DestroyIcon(static_cast<HICON>(hIcon));

        hIcon = nullptr;

        // Change RGBA to BGRA
        std::vector<std::byte> iconPixels(width * height * 4);
        for (std::size_t i = 0; i < iconPixels.size() / 4; ++i)
        {
            iconPixels[i * 4 + 0] = pixels[i * 4 + 2];
            iconPixels[i * 4 + 1] = pixels[i * 4 + 1];
            iconPixels[i * 4 + 2] = pixels[i * 4 + 0];
            iconPixels[i * 4 + 3] = pixels[i * 4 + 3];
        }

        hIcon = ::CreateIcon(
                GetModuleHandleW(nullptr),
                static_cast<int>(width),
                static_cast<int>(height),
                1,
                32,
                nullptr,
                reinterpret_cast<unsigned char*>(iconPixels.data()));

        if (hIcon != nullptr)
        {
            ::SendMessageW(
                    GetWindowHandle<HWND>(),
                    WM_SETICON,
                    ICON_BIG,
                    reinterpret_cast<LPARAM>(hIcon));

            ::SendMessageW(
                    GetWindowHandle<HWND>(),
                    WM_SETICON,
                    ICON_SMALL,
                    reinterpret_cast<LPARAM>(hIcon));
        }
    }

    void Window::SetIcon(int iconResId)
    {
        if (_hWindow == nullptr)
            return;

        if (hIcon != nullptr)
            ::DestroyIcon(static_cast<HICON>(hIcon));

        hIcon = nullptr;

        hIcon = ::LoadIconW(GetModuleHandleW(nullptr), MAKEINTRESOURCE(iconResId));
        if (hIcon != nullptr)
        {
            ::SendMessageW(
                    GetWindowHandle<HWND>(),
                    WM_SETICON,
                    ICON_BIG,
                    reinterpret_cast<LPARAM>(hIcon));

            ::SendMessageW(
                    GetWindowHandle<HWND>(),
                    WM_SETICON,
                    ICON_SMALL,
                    reinterpret_cast<LPARAM>(hIcon));
        }
    }

    void Window::SetWindowVisible(bool show)
    {
        if (_hWindow == nullptr)
            return;

        ::ShowWindow(GetWindowHandle<HWND>(), show ? SW_SHOW : SW_HIDE);
    }

    void Window::SetCursorVisible(bool show)
    {
        if (_hWindow == nullptr)
            return;

        cursorVisible = show;
        ::SetCursor(show ? static_cast<HCURSOR>(hCursor) : nullptr);

        for (auto pService: _servicesInCreationOrder)
            pService->OnCursorVisibleChanged(show);

        if (_onWindowCursorVisibleChanged != nullptr)
            _onWindowCursorVisibleChanged(show);
    }

    void Window::SetCursorLimitedInWindow(bool capture)
    {
        if (_hWindow == nullptr)
            return;

        cursorLimitedInWindow = capture;
        SetCursorLimitedInWindowInternal(cursorLimitedInWindow);
    }

    bool Window::IsCursorInsideWindow() const
    {
        if (_hWindow == nullptr)
            return false;

        return cursorInsideWindow;
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

    void Window::SetTrackMouseLeave(bool enable)
    {
        if (_hWindow == nullptr)
            return;

        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = enable ? TME_LEAVE : TME_CANCEL;
        tme.hwndTrack = GetWindowHandle<HWND>();
        ::TrackMouseEvent(&tme);
    }

    bool Window::IsCursorVisible() const
    {
        if (_hWindow == nullptr)
            return false;

        return cursorVisible;
    }

    bool Window::IsCursorLimitedInWindow() const
    {
        if (_hWindow == nullptr)
            return false;

        return cursorLimitedInWindow;
    }

    void Window::OnWindowClose()
    {
        if (_hWindow == nullptr)
            return;

        bool doClose = true;
        if (_onWindowTryToClose != nullptr)
            doClose = _onWindowTryToClose();

        if (doClose)
        {
            if (_onWindowClosed != nullptr)
                _onWindowClosed();

            Destroy();
        }
    }

    void Window::OnWindowPreDestroy()
    {
        if (_hWindow == nullptr)
            return;

        for (auto pService: _servicesInCreationOrder)
            pService->OnWindowPreDestroyed();

        if (_onWindowPreDestroyed != nullptr)
            _onWindowPreDestroyed();

        SetCursorVisible(true);
        ::ReleaseCapture();

        // Release cursor tracking
        SetTrackMouseLeave(false);

        // Icon
        if (hIcon != nullptr)
            ::DestroyIcon(static_cast<HICON>(hIcon));

        // Clear service
        ClearService();
    }

    void Window::OnWindowPostDestroy()
    {
        // Reset window handle
        _hWindow = nullptr;

        // Global counting
        gGlobalWindowsCount--;

        if (gGlobalWindowsCount == 0)
            UnRegisterWindowClass();

        if (_onWindowPostDestroyed != nullptr)
            _onWindowPostDestroyed();
    }

    void Window::SetTitle(const std::string& title)
    {
        if (_hWindow == nullptr)
            return;

        const auto titleInWideStr = Utility::StringToWideString(title);
        const wchar_t* titleWide = titleInWideStr.c_str();
        ::SetWindowTextW(GetWindowHandle<HWND>(), titleWide);
    }

    std::pair<int, int> Window::GetSize()
    {
        if (_hWindow == nullptr)
            return { 0, 0 };

        RECT rect;
        ::GetClientRect(GetWindowHandle<HWND>(), &rect);
        return { static_cast<int>(rect.right - rect.left), static_cast<int>(rect.bottom - rect.top) };
    }

    std::pair<int, int> Window::GetPosition()
    {
        if (_hWindow == nullptr)
            return { 0, 0 };

        RECT rect;
        ::GetWindowRect(GetWindowHandle<HWND>(), &rect);

        return { static_cast<int>(rect.left), static_cast<int>(rect.top) };
    }

    void Window::SetPosition(int x, int y)
    {
        if (_hWindow == nullptr)
            return;

        ::SetWindowPos(
                GetWindowHandle<HWND>(),
                nullptr,
                x,
                y,
                0,
                0,
                SWP_NOSIZE | SWP_NOZORDER);

        // Adjust cursor position
        if(cursorLimitedInWindow)
            SetCursorLimitedInWindow(true);
    }

    void Window::Loop(const std::function<void()>& loopFunction)
    {
        while (true)
        {
            // Fetch new event
            MSG message;
            while (::PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
            {
                ::TranslateMessage(&message);
                ::DispatchMessageW(&message);
            }

            if (!IsWindowValid())
                return;

            auto services = GetServices();
            for (auto pService: services)
                pService->BeforeTick();

            if (loopFunction != nullptr)
                loopFunction();

            for (auto pService: services)
                pService->AfterTick();

            for (auto pService: services)
                pService->FinishLoop();
        }
    }

    void Window::SetCursorLimitedInWindowInternal(bool doCapture)
    {
        if (doCapture)
        {
            if (_hWindow == nullptr)
                return;

            HWND hWindow = GetWindowHandle<HWND>();

            RECT rect;
            ::GetClientRect(hWindow, &rect);
            ::MapWindowPoints(hWindow, nullptr, reinterpret_cast<LPPOINT>(&rect), 2);
            ::ClipCursor(&rect);
        }
        else
        {
            ::ClipCursor(nullptr);
        }
    }

    bool Window::CalculateMouseInsideWindow() const
    {
        if (_hWindow == nullptr)
            return false;

        POINT cursorPos;
        if (!::GetCursorPos(&cursorPos))
            return false;

        const HWND hWnd = GetWindowHandle<HWND>();
        POINT clientPoint = cursorPos;
        ::ScreenToClient(hWnd, &clientPoint);

        RECT clientRect;
        ::GetClientRect(hWnd, &clientRect);

        return (clientPoint.x >= 0 && clientPoint.x < clientRect.right &&
            clientPoint.y >= 0 && clientPoint.y < clientRect.bottom);
    }

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

    int Window::WindowEventProcess(uint32_t message, void* wpara, void* lpara)
    {
        if (_hWindow == nullptr)
            return 0;

        int ret = 0;
        if (_onWindowMessagePreProcess != nullptr)
        {
            // ReSharper disable once CppTooWideScope
            const bool blockProcess = _onWindowMessagePreProcess(message, wpara, lpara, &ret);
            if (blockProcess)
                return ret;
        }

        // WM_CLOSE in DefWindowProcW will cause WM_DESTROY sent.
        if (message == WM_CLOSE)
        {
            OnWindowClose();
            return 0;
        }

        // Hack the menu system command, so that pressing ALT or F10 doesn't steal the focus
        if ((message == WM_SYSCOMMAND) && (reinterpret_cast<WPARAM>(wpara) == SC_KEYMENU))
            return 0;

        // Process service messages
        for (auto pService: GetServices())
        {
            if (pService != nullptr)
                pService->ProcessWinMessage(GetWindowHandle(), message, wpara, lpara);
        }

        // Process window messages
        WindowEventProcessInternal(message, wpara, lpara);

        if (message == WM_DESTROY)
            OnWindowPreDestroy();

        ret = ::DefWindowProcW(GetWindowHandle<HWND>()
            , message
            , reinterpret_cast<WPARAM>(wpara)
            , reinterpret_cast<LPARAM>(lpara));

        if (message == WM_DESTROY)
            OnWindowPostDestroy();

        return ret;
    }

    void Window::WindowEventProcessInternal(uint32_t message, void* wpara, void* lpara)
    {
        if (_hWindow == nullptr)
            return;

        WPARAM wParam = reinterpret_cast<WPARAM>(wpara);
        LPARAM lParam = reinterpret_cast<LPARAM>(lpara);

        switch (message)
        {
            case WM_SETCURSOR:
            {
                // lower world of lParam is hit test result
                if (LOWORD(lParam) == HTCLIENT)
                    ::SetCursor(cursorVisible ? static_cast<HCURSOR>(hCursor) : nullptr);

                break;
            }
            case WM_MOVE:
            {
                int x = LOWORD(lParam); // left up x
                int y = HIWORD(lParam); // left up y

                for (auto pService: _servicesInCreationOrder)
                    pService->OnWindowMoved(x, y);

                if (_onWindowMoved != nullptr)
                    _onWindowMoved(x, y);

                break;
            }
            case WM_SIZE:
            {
                auto [newWidth, newHeight] = GetSize();
                if (wParam != SIZE_MINIMIZED && (lastWidth != newWidth || lastHeight != newHeight))
                {
                    for (auto pService: _servicesInCreationOrder)
                        pService->OnWindowResized(newWidth, newHeight);

                    if (_onWindowResize != nullptr)
                        _onWindowResize(newWidth, newHeight);

                    lastWidth = newWidth;
                    lastHeight = newHeight;
                }
                break;
            }
            case WM_SETFOCUS:
            {
                SetCursorLimitedInWindowInternal(cursorLimitedInWindow);

                for (auto pService: _servicesInCreationOrder)
                    pService->OnWindowFocusChanged(true);

                if (_onWindowFocusChanged != nullptr)
                    _onWindowFocusChanged(true);

                break;
            }
            case WM_KILLFOCUS:
            {
                SetCursorLimitedInWindowInternal(false);

                for (auto pService: _servicesInCreationOrder)
                    pService->OnWindowFocusChanged(false);

                if (_onWindowFocusChanged != nullptr)
                    _onWindowFocusChanged(false);

                break;
            }
            case WM_MOUSEMOVE:
            {
                const HWND hWnd = GetWindowHandle<HWND>();

                // Capture the mouse in case the user wants to drag it outside
                if ((wParam & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2)) == 0)
                {
                    if (::GetCapture() == hWnd)
                        ::ReleaseCapture();
                }
                else if (::GetCapture() != hWnd)
                {
                    ::SetCapture(hWnd);
                }

                // Mouse inside window
                if (!cursorInsideWindow)
                {
                    cursorInsideWindow = true;

                    SetTrackMouseLeave(true);

                    for (auto pService: _servicesInCreationOrder)
                        pService->OnCursorEnteredOrLeave(true);

                    if (_onWindowCursorEnteredOrLeaved != nullptr)
                        _onWindowCursorEnteredOrLeaved(true);
                }
                break;
            }
            // WM_MOUSELEAVE will be received only if ::TrackMouseEvent set.
            // Once WM_MOUSELEAVE received, ::TrackMouseEvent will be canceled.
            // So ::TrackMouseEvent should be set again when mouse enter.
            case WM_MOUSELEAVE:
            {
                // Mouse outside window
                if (cursorInsideWindow)
                {
                    cursorInsideWindow = false;

                    for (auto pService: _servicesInCreationOrder)
                        pService->OnCursorEnteredOrLeave(false);

                    if (_onWindowCursorEnteredOrLeaved != nullptr)
                        _onWindowCursorEnteredOrLeaved(false);
                }
                break;
            }
        }

    }

}
