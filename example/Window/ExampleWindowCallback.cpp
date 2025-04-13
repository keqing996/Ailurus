
#include <iostream>
#include <Ailurus/Application/Application.h>

using namespace Ailurus;

int Main(int argc, char *argv[])
{
    Application::SetCallbackOnWindowCreated([]()->void
    {
        std::cout << "Created" << std::endl;
    });

    Application::SetCallbackOnWindowTryToClose([]()->bool
    {
        std::cout << "Try to close" << std::endl;
        return true;
    });

    Application::SetCallbackOnWindowClosed([]()->void
    {
        std::cout << "Closed" << std::endl;
    });

    Application::SetCallbackOnWindowPreDestroyed([]()->void
    {
        std::cout << "Pre Destroy" << std::endl;
    });

    Application::SetCallbackOnWindowPostDestroyed([]()->void
    {
        std::cout << "Post Destroy" << std::endl;
    });

    Application::SetCallbackOnWindowMoved([](Vector2i pos)->void
    {
        std::cout << "Move: " << pos.x << ", " << pos.y << std::endl;
    });

    Application::SetCallbackOnWindowResize([](Vector2i size)->void
    {
        std::cout << "Resize: " << size.x << ", " << size.y << std::endl;
    });

    Application::SetCallbackOnWindowFocusChanged([](bool focus)->void
    {
        std::cout << (focus ? "Focus get" : "Focus lost") << std::endl;
    });

    Application::SetCallbackOnWindowCursorEnteredOrLeaved([](bool enter)->void
    {
        std::cout << (enter ? "Cursor enter" : "Cursor leave") << std::endl;
    });

    Application::SetCallbackOnWindowCursorVisibleChanged([](bool visible)->void
    {
        std::cout << (visible ? "Cursor visible" : "Cursor hide") << std::endl;
    });

    Application::Create(800, 600, "Test", Application::Style());

    Application::Loop([&]()-> void
    {
        if (Application::GetInputManager().IsButtonPressed(ButtonType::KeyboardU))
        {
            bool currentCursorVisible = Application::IsCursorVisible();
            Application::SetCursorVisible(!currentCursorVisible);
        }
    });

	Application::Destroy();

    return 0;
}