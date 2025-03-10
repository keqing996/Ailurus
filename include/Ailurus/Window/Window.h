#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <memory>

#include "Input/Input.h"
#include "ImGui/ImGui.h"
#include "Ailurus/Math/Vector.hpp"
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Renderer/Renderer.h"

namespace Ailurus
{
    class Window final: NonCopyable
    {
    public:
        struct Style
        {
            bool canResize = true;
            bool haveBorder = true;
        };

    public:
        Window();
        ~Window();

    public:
        /// Create window instance.
        /// @param width Window width.
        /// @param height Window height.
        /// @param title Window title in UTF8.
        /// @param style Window style.
        /// @return Create success.
        bool Create(int width, int height, const std::string& title, Style style);

        /// Destroy window instance.
        void Destroy();

        /// Check is window instance created.
        bool IsWindowValid() const;

        /// Window main loop.
        /// @param loopFunction called every frame.
        void Loop(const std::function<void()>& loopFunction);

        /// Get native window's size(client area, without borders, caption bar, etc.).
        Vector2i GetSize() const;

        /// Set native window's size.
        void SetSize(int width, int height);

        /// Set native window's size.
        void SetSize(const Vector2i& size);

        /// Get native window's position in screen.
        Vector2i GetPosition() const;

        /// Set native window's position in screen.
        void SetPosition(int x, int y);

        /// Set native window's position in screen.
        void SetPosition(const Vector2i& pos);

        /// Set icon of window through bytes in memory.
        /// @param width Icon width.
        /// @param height Icon height.
        /// @param pixels Icon bytes data in RGBA8888.
        void SetIcon(unsigned int width, unsigned int height, const std::byte* pixels);

        /// Set window title in UTF8.
        void SetTitle(const std::string& title);

        /// Set window visible.
        void SetWindowVisible(bool show);

        /// Get is cursor visible.
        bool IsCursorVisible() const;

        /// Set cursor visible.
        void SetCursorVisible(bool show);

        /// Is cursor limited in window client area.
        bool IsCursorLimitedInWindow() const;

        /// Set cursor limited in window client area.
        void SetCursorLimitedInWindow(bool capture);

        /// Get is cursor inside window now (frame not included).
        bool IsCursorInsideWindow() const;

        /// Called when window is created.
        void SetCallbackOnWindowCreated(const std::function<void()>& callback);

        /// Called when window is trying to close, return true to let
        /// window close, return false to refuse window to close.
        void SetCallbackOnWindowTryToClose(const std::function<bool()>& callback);

        /// Called when window is closed.
        void SetCallbackOnWindowClosed(const std::function<void()>& callback);

        /// Called when window is about to be destroyed, at this moment, WM_DESTROY
        /// is not sent to window yet, and all resources (icon, or something) are still
        /// held by window.
        void SetCallbackOnWindowPreDestroyed(const std::function<void()>& callback);

        /// Called when window is already destroyed, at this moment, WM_DESTROY is
        /// already received, all resources are released, window is closed.
        void SetCallbackOnWindowPostDestroyed(const std::function<void()>& callback);

        /// Called when window received WM_MOVE
        ///
        /// - int: left up corner x.
        /// - int: left up corner y.
        void SetCallbackOnWindowMoved(const std::function<void(Vector2i)>& callback);

        /// Called when window received WM_SIZE
        /// - int: new size width.
        /// - int: new size height.
        void SetCallbackOnWindowResize(const std::function<void(Vector2i)>& callback);

        /// Called when window get or lost focus,
        /// true for enter focus and false for lose focus.
        void SetCallbackOnWindowFocusChanged(const std::function<void(bool)>& callback);

        /// Called when cursor enters or leaves window, true for
        /// enters window and false for leaves window.
        void SetCallbackOnWindowCursorEnteredOrLeaved(const std::function<void(bool)>& callback);

        /// Called when cursor's visibility changes, true for shown and false for hided.
        void SetCallbackOnWindowCursorVisibleChanged(const std::function<void(bool)>& callback);

        void* GetSDLWindowPtr();

        const Input* GetInput() const;

        const ImGui* GetImGui() const;

    private:
        void EventLoop(bool* quitLoop);
        void HandleEvent(const void* pEvent, bool* quitLoop);

    private:
        // Window handle
        void* _pWindow = nullptr;

        // Close request
        bool _ignoreNextQuit = false;

        // Renderer
        std::unique_ptr<Renderer> _pRenderer = nullptr;

        // Window callbacks
        std::function<void()>           _onWindowCreated = nullptr;
        std::function<void(Vector2i)>   _onWindowMoved = nullptr;
        std::function<bool()>           _onWindowTryToClose = nullptr;
        std::function<void()>           _onWindowClosed = nullptr;
        std::function<void()>           _onWindowPreDestroyed = nullptr;
        std::function<void()>           _onWindowPostDestroyed = nullptr;
        std::function<void(Vector2i)>   _onWindowResize = nullptr;
        std::function<void(bool)>       _onWindowFocusChanged = nullptr;
        std::function<void(bool)>       _onWindowCursorEnteredOrLeaved = nullptr;
        std::function<void(bool)>       _onWindowCursorVisibleChanged = nullptr;

        // Input
        std::unique_ptr<Input> _pInput = nullptr;

        // ImGui
        std::unique_ptr<ImGui> _pImGui = nullptr;
    };
}

