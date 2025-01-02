
#pragma once

#include "Ailurus/PlatformDefine.h"

#if PLATFORM_WINDOWS

#include <functional>
#include <unordered_set>
#include <vector>
#include "../Service.h"
#include "InputEvent.h"

namespace Ailurus
{
    class InputService: public Service, public ServiceTypeGetter<ServiceType::Input>
    {
    public:
        explicit InputService(Window* pWindow);

    public:
        void ProcessWinMessage(void* hWnd, uint32_t msg, void* wPara, void* lPara) override;
        void BeforeTick() override;

    public:
        bool IsButtonPressed(ButtonType key) const;
        std::pair<int, int> GetMousePosition() const;
        float GetMouseWheel() const;

        bool GetIsAutoRepeat() const;
        void SetIsAutoRepeat(bool autoRepeat);

        void SetCallbackOnMouseMove(const std::function<void(std::pair<int, int>, std::pair<int, int>)>& fun);
        void SetCallbackOnMouseWheel(const std::function<void(float)>& fun);
        void SetCallbackOnButtonPressed(const std::function<void(ButtonType)>& fun);
        void SetCallbackOnButtonReleased(const std::function<void(ButtonType)>& fun);

    private:
        void ProcessEventQueue();
        ButtonType WinVirtualKeyToButtonType(void* wpara, void* lpara);
        
        static void* MapControlKeyLeftAndRight(void* wpara, void* lpara);

    private:
        std::vector<InputEvent> _eventQueue;

        std::pair<int, int> _mousePos;
        float _mouseWheel;

        bool _enableAutoRepeat = true;
        std::unordered_set<ButtonType> _pressedButton;

        std::function<void(std::pair<int, int>, std::pair<int, int>)> _onMouseMove = nullptr;
        std::function<void(float)> _onMouseWheel = nullptr;
        std::function<void(ButtonType)> _onButtonPressed = nullptr;
        std::function<void(ButtonType)> _onButtonReleased = nullptr;
    };
}

#endif
