#include "Ailurus/Window/Window.h"

#if AILURUS_FEAT_SUPPORT_WINDOW

#include <SDL3/SDL_main.h>

namespace Ailurus
{
    Window::Window()
    {
    }

    Window::~Window()
    {
    }

    bool Window::Create(int width, int height, const std::string& title, WindowStyle style)
    {
        if (!SDL_Init(SDL_INIT_VIDEO))
            return false;

        SDL_WindowFlags flags = 0;
        if (!style.haveTitleBar)
            flags |= SDL_WINDOW_BORDERLESS;
        else
        {
            if (style.haveResize)
                flags |= SDL_WINDOW_RESIZABLE;
            if (style.haveClose)
                flags |= SDL_WINDOW_Clo
        }

        _hWindow = SDL_CreateWindow(width, height)
    }

    void Window::Destroy()
    {
    }
}




#endif
