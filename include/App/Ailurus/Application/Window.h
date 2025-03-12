#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <memory>

#include "Input/Input.h"
#include "ImGui/ImGui.h"
#include "Ailurus/Math/Vector.hpp"
#include "Ailurus/Utility/NonCopyable.h"
#include "Render/Render.h"

namespace Ailurus
{
    class Application final: NonCopyable
    {
    public:
        struct Style
        {
            bool canResize = true;
            bool haveBorder = true;
        };

    public:
        Application() = delete;

    public:
        /// Create window instance.
        /// @param width Window width.
        /// @param height Window height.
        /// @param title Window title in UTF8.
        /// @param style Window style.
        /// @return Create success.
        static bool Create(int width, int height, const std::string& title, Style style);

        /// Destroy window instance.
        static void Destroy();

        /// Check is window instance created.
        static bool IsWindowValid();

        /// Window main loop.
        /// @param loopFunction called every frame.
        static void Loop(const std::function<void()>& loopFunction);

        /// Get native window's size(client area, without borders, caption bar, etc.).
        static Vector2i GetSize();

        /// Set native window's size.
        static void SetSize(int width, int height);

        /// Set native window's size.
        static void SetSize(const Vector2i& size);

        /// Get native window's position in screen.
        static Vector2i GetPosition();

        /// Set native window's position in screen.
        static void SetPosition(int x, int y);

        /// Set native window's position in screen.
        static void SetPosition(const Vector2i& pos);

        /// Set icon of window through bytes in memory.
        /// @param width Icon width.
        /// @param height Icon height.
        /// @param pixels Icon bytes data in RGBA8888.
        static void SetIcon(unsigned int width, unsigned int height, const std::byte* pixels);

        /// Set window title in UTF8.
        static void SetTitle(const std::string& title);

        /// Set window visible.
        static void SetWindowVisible(bool show);

        /// Get is cursor visible.
        static bool IsCursorVisible();

        /// Set cursor visible.
        static void SetCursorVisible(bool show);

        /// Is cursor limited in window client area.
        static bool IsCursorLimitedInWindow();

        /// Set cursor limited in window client area.
        static void SetCursorLimitedInWindow(bool capture);

        /// Get is cursor inside window now (frame not included).
        static bool IsCursorInsideWindow();

        /// Called when window is created.
        static void SetCallbackOnWindowCreated(const std::function<void()>& callback);

        /// Called when window is trying to close, return true to let
        /// window close, return false to refuse window to close.
        static void SetCallbackOnWindowTryToClose(const std::function<bool()>& callback);

        /// Called when window is closed.
        static void SetCallbackOnWindowClosed(const std::function<void()>& callback);

        /// Called when window is about to be destroyed, at this moment, WM_DESTROY
        /// is not sent to window yet, and all resources (icon, or something) are still
        /// held by window.
        static void SetCallbackOnWindowPreDestroyed(const std::function<void()>& callback);

        /// Called when window is already destroyed, at this moment, WM_DESTROY is
        /// already received, all resources are released, window is closed.
        static void SetCallbackOnWindowPostDestroyed(const std::function<void()>& callback);

        /// Called when window received WM_MOVE
        ///
        /// - int: left up corner x.
        /// - int: left up corner y.
        static void SetCallbackOnWindowMoved(const std::function<void(Vector2i)>& callback);

        /// Called when window received WM_SIZE
        /// - int: new size width.
        /// - int: new size height.
        static void SetCallbackOnWindowResize(const std::function<void(Vector2i)>& callback);

        /// Called when window get or lost focus,
        /// true for enter focus and false for lose focus.
        static void SetCallbackOnWindowFocusChanged(const std::function<void(bool)>& callback);

        /// Called when cursor enters or leaves window, true for
        /// enters window and false for leaves window.
        static void SetCallbackOnWindowCursorEnteredOrLeaved(const std::function<void(bool)>& callback);

        /// Called when cursor's visibility changes, true for shown and false for hided.
        static void SetCallbackOnWindowCursorVisibleChanged(const std::function<void(bool)>& callback);

        static void* GetSDLWindowPtr();

        static const Input* GetInput();

        static ImGui* GetImGui();

        static Render* GetRender();

    private:
        static void EventLoop(bool* quitLoop);
        static void HandleEvent(const void* pEvent, bool* quitLoop);

    private:
        // Window handle
        static void* _pWindow;

        // Close request
        static bool _ignoreNextQuit;

        // Window callbacks
        static std::function<void()>           _onWindowCreated;
        static std::function<void(Vector2i)>   _onWindowMoved;
        static std::function<bool()>           _onWindowTryToClose;
        static std::function<void()>           _onWindowClosed;
        static std::function<void()>           _onWindowPreDestroyed;
        static std::function<void()>           _onWindowPostDestroyed;
        static std::function<void(Vector2i)>   _onWindowResize;
        static std::function<void(bool)>       _onWindowFocusChanged;
        static std::function<void(bool)>       _onWindowCursorEnteredOrLeaved;
        static std::function<void(bool)>       _onWindowCursorVisibleChanged;

        // Input
        static std::unique_ptr<Input> _pInput;

        // ImGui
        static std::unique_ptr<ImGui> _pImGui;

        // Render
        static std::unique_ptr<Render> _pRender;
    };
}

