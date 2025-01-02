
#include <Windows.h>
#include <iostream>
#include <NativeWindow/Window.h>

int main()
{
    NativeWindow::Window window;
    window.SetCallbackOnWindowMessagePreProcess([](uint32_t msg, void* wpara, void* lpara, int* result) -> bool
    {
        WPARAM wParam = reinterpret_cast<WPARAM>(wpara);
        LPARAM lParam = reinterpret_cast<LPARAM>(lpara);
        switch (msg)
        {
            case WM_SETCURSOR:
                std::cout << "Set Cursor" << std::endl;
                break;
            case WM_MOVE:
            {
                int x = LOWORD(lParam); // left up x
                int y = HIWORD(lParam); // left up y
                std::cout << "Move " << x << ", " << y << std::endl;
                break;
            }
            case WM_SIZE:
                std::cout << "Set Size" << std::endl;
                break;
            case WM_SETFOCUS:
                std::cout << "Get Focus" << std::endl;
                break;
            case WM_KILLFOCUS:
                std::cout << "Kill Focus" << std::endl;
                break;
            case WM_MOUSEMOVE:
                std::cout << "Mouse Move" << std::endl;
                break;
            case WM_MOUSELEAVE:
                std::cout << "Mouse Leave" << std::endl;
                break;
            case WM_CLOSE:
                std::cout << "Close" << std::endl;
                break;
            case WM_DESTROY:
                std::cout << "Destroy" << std::endl;
                break;
        }
        return false;
    });

    window.Create(800, 600, "Test", NativeWindow::WindowStyle::DefaultStyle());

    while (true)
    {
        bool finish;
        window.Loop(&finish);
        if (finish)
            break;
    }

    system("pause");

    return 0;
}