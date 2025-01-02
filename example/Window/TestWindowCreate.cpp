
#include <Windows.h>
#include <NativeWindow/Window.h>

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PWSTR lpCmdLine, int nCmdShow)
{
    NativeWindow::Window window;
    window.Create(800, 600, "Test", NativeWindow::WindowStyle::DefaultStyle());

    while (true)
    {
        bool finish;
        window.Loop(&finish);
        if (finish)
            break;
    }

    return 0;
}