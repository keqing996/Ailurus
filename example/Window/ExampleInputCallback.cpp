#include <iostream>
#include <Ailurus/Application/Application.h>

using namespace Ailurus;

int Main(int argc, char *argv[])
{
    Application::Create(800, 600, "Test", Application::Style());

    Application::GetInputManager().SetCallbackOnMouseMove([](Vector2f source, Vector2f target) -> void
    {
        std::cout << "[MouseMove]   (" << source.x() << ", " << source.y() << ") -> ("
            << target.x() << ", " << target.y() << ")" << std::endl;
    });

    Application::GetInputManager().SetCallbackOnMouseWheel([](Vector2f delta) -> void
    {
        std::cout << "[MouseWheel]   (" << delta.x() << ", " << delta.y() << ")" << std::endl;
    });

    Application::GetInputManager().SetCallbackOnButtonPressed([](ButtonType btn) -> void
    {
        std::cout << "[BtnPressed]  " << ButtonTypeUtility::GetName(btn) << std::endl;
    });

    Application::GetInputManager().SetCallbackOnButtonReleased([](ButtonType btn) -> void
    {
        std::cout << "[BtnRelease]  " << ButtonTypeUtility::GetName(btn) << std::endl;
    });

    Application::Loop([&]()->void
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
