
#include <iostream>
#include <Ailurus/Platform/Windows/Window/Window.h>
#include <Ailurus/Platform/Windows/Window/Service/InputService/InputService.h>

int main()
{
    Ailurus::Window window;
    window.AddService<Ailurus::InputService>();

    window.SetCallbackOnWindowCreated([]()->void
    {
        std::cout << "Created" << std::endl;
    });

    window.SetCallbackOnWindowTryToClose([]()->bool
    {
        std::cout << "Try to close" << std::endl;
        return true;
    });

    window.SetCallbackOnWindowClosed([]()->void
    {
        std::cout << "Closed" << std::endl;
    });

    window.SetCallbackOnWindowPreDestroyed([]()->void
    {
        std::cout << "Pre Destroy" << std::endl;
    });

    window.SetCallbackOnWindowPostDestroyed([]()->void
    {
        std::cout << "Post Destroy" << std::endl;
    });

    window.SetCallbackOnWindowMoved([](int x, int y)->void
    {
        std::cout << "Move: " << x << ", " << y << std::endl;
    });

    window.SetCallbackOnWindowResize([](int width, int height)->void
    {
        std::cout << "Resize: " << width << ", " << height << std::endl;
    });

    window.SetCallbackOnWindowFocusChanged([](bool focus)->void
    {
        std::cout << (focus ? "Focus get" : "Focus lost") << std::endl;
    });

    window.SetCallbackOnWindowCursorEnteredOrLeaved([](bool enter)->void
    {
        std::cout << (enter ? "Cursor enter" : "Cursor leave") << std::endl;
    });

    window.SetCallbackOnWindowCursorVisibleChanged([](bool visible)->void
    {
        std::cout << (visible ? "Cursor visible" : "Cursor hide") << std::endl;
    });

    window.Create(800, 600, "Test", Ailurus::WindowStyle::DefaultStyle());

    window.Loop([&]()-> void
    {
        if (window.GetService<Ailurus::InputService>()->IsButtonPressed(Ailurus::ButtonType::KeyboardU))
        {
            bool currentCursorVisible = window.IsCursorVisible();
            window.SetCursorVisible(!currentCursorVisible);
        }
    });

    return 0;
}