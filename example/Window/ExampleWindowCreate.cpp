
#include <Ailurus/Window/Window.h>

int Main(int argc, char *argv[])
{
    Ailurus::Window window;
    window.Create(800, 600, "Test", Ailurus::WindowStyle{});
    window.Loop(nullptr);

    return 0;
}