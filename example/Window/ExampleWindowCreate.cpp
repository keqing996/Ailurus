
#include <Ailurus/Window/Window.h>

int Main(int argc, char *argv[])
{
    Ailurus::Window window;
    window.Create(800, 600, "Test", Ailurus::Window::Style{});
    window.Loop(nullptr);

    return 0;
}