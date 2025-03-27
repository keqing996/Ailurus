#include <SDL3/SDL_main.h>

extern int Main(int argc, char* argv[]);

int SDL_main(int argc, char* argv[])
{
    return Main(argc, argv);
}