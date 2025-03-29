
#include <Ailurus/Application/Application.h>

int Main(int argc, char *argv[])
{
	Ailurus::Application::Create(800, 600, "Test", Ailurus::Application::Style{});
    Ailurus::Application::Loop(nullptr);
	Ailurus::Application::Destroy();
    return 0;
}