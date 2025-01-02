
#include <iostream>
#include <NativeWindow/Window.h>

int main()
{
    NativeWindow::Window window;

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

    window.Create(800, 600, "Test", NativeWindow::WindowStyle::DefaultStyle());

    while (true)
    {
        bool finish;
        window.Loop(&finish);
        if (finish)
            break;

        if (window.GetInput().IsButtonPressed(NativeWindow::ButtonType::KeyboardU))
        {
            bool currentCursorVisible = window.IsCursorVisible();
            window.SetCursorVisible(!currentCursorVisible);
        }
    }

    system("pause");

    return 0;
}