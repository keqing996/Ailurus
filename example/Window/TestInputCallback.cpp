#include <iostream>
#include <NativeWindow/Window.h>
#include <NativeWindow/Service/InputService/InputService.h>

using namespace Ailurus;

int main()
{
    Window window;

    window.Create(800, 600, "Test", WindowStyle::DefaultStyle());
    window.AddService<InputService>();

    auto inputService = window.GetService<InputService>();

    inputService->SetCallbackOnMouseMove([](std::pair<int, int> source, std::pair<int, int> target) -> void
    {
        std::cout << "[MouseMove]   (" << source.first << ", " << source.second << ") -> ("
            << target.first << ", " << target.second << ")" << std::endl;
    });

    inputService->SetCallbackOnMouseWheel([](float delta) -> void
    {
        std::cout << "[MouseWheel]   (" << delta << ")" << std::endl;
    });

    inputService->SetCallbackOnButtonPressed([](ButtonType btn) -> void
    {
        std::cout << "[BtnPressed]  " << ButtonTypeUtility::GetName(btn) << std::endl;
    });

    inputService->SetCallbackOnButtonReleased([](ButtonType btn) -> void
    {
        std::cout << "[BtnRelease]  " << ButtonTypeUtility::GetName(btn) << std::endl;
    });

    window.Loop([&]()->void
    {
        if (inputService->IsButtonPressed(ButtonType::KeyboardU))
        {
            bool currentCursorVisible = window.IsCursorVisible();
            window.SetCursorVisible(!currentCursorVisible);
        }
    });

    system("pause");

    return 0;
}
