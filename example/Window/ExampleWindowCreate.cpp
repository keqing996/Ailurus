
#include <Windows.h>
#include <Ailurus/Platform/Windows/Window/Window.h>

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PWSTR lpCmdLine, int nCmdShow)
{
    Ailurus::Window window;
    window.Create(800, 600, "Test", Ailurus::WindowStyle::DefaultStyle());
    window.Loop(nullptr);

    return 0;
}