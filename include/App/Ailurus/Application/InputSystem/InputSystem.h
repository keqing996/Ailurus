#pragma once

#include <functional>
#include <unordered_set>
#include "InputEnum.h"
#include "Ailurus/Math/Vector2.hpp"
#include "Ailurus/Utility/NonCopyable.h"

namespace Ailurus
{
    class InputSystem: public NonCopyable
    {
    public:
        ~InputSystem() override;

    public:
        bool IsButtonPressed(ButtonType key) const;
        Vector2f GetMousePosition() const;
        Vector2f GetMouseWheel() const;

        bool GetIsAutoRepeat() const;
        void SetIsAutoRepeat(bool autoRepeat);

        void SetCallbackOnMouseMove(const std::function<void(Vector2f, Vector2f)>& callback);
        void SetCallbackOnMouseWheel(const std::function<void(Vector2f)>& callback);
        void SetCallbackOnButtonPressed(const std::function<void(ButtonType)>& callback);
        void SetCallbackOnButtonReleased(const std::function<void(ButtonType)>& callback);

    private:
        friend class Application;
        InputSystem();

    private:
        void OnEventButtonPressed(ButtonType button);
        void OnEventButtonReleased(ButtonType button);
        void BeforeEventLoop();
        void HandleEvent(void* sdlWindowPtr, const void* pEvent);
        void AfterEventLoop();

    private:
        Vector2f _lastMousePos;
        Vector2f _mousePos;
        Vector2f _mouseWheel;

        bool _enableAutoRepeat = true;
        std::unordered_set<ButtonType> _pressedButton;

        std::function<void(Vector2f, Vector2f)> _onMouseMove = nullptr;
        std::function<void(Vector2f)> _onMouseWheel = nullptr;
        std::function<void(ButtonType)> _onButtonPressed = nullptr;
        std::function<void(ButtonType)> _onButtonReleased = nullptr;

    };
}