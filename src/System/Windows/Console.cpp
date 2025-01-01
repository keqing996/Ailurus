#include "Ailurus/Platform/Windows/Console.h"

#if PLATFORM_WINDOWS

#include <iostream>
#include "Ailurus/Platform/Windows/WindowsDefine.h"

namespace Ailurus
{
    void Console::CreateConsole()
    {
        if (!::AllocConsole())
            return;

        FILE* fDummy;
        ::freopen_s(&fDummy, "CONIN$", "r", stdin);
        ::freopen_s(&fDummy, "CONOUT$", "w", stderr);
        ::freopen_s(&fDummy, "CONOUT$", "w", stdout);

        // clear old std
        std::cout.clear();
        std::clog.clear();
        std::cerr.clear();
        std::cin.clear();

        // std wide char stream
        HANDLE hConOut = ::CreateFileW(L"CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        HANDLE hConIn = ::CreateFileW(L"CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        ::SetStdHandle(STD_OUTPUT_HANDLE, hConOut);
        ::SetStdHandle(STD_ERROR_HANDLE, hConOut);
        ::SetStdHandle(STD_INPUT_HANDLE, hConIn);

        std::wcout.clear();
        std::wclog.clear();
        std::wcerr.clear();
        std::wcin.clear();
    }

    void Console::AttachConsole()
    {
        ::AttachConsole(ATTACH_PARENT_PROCESS);
    }

    void Console::DetachConsole()
    {
        ::FreeConsole();
    }

    void Console::SetConsoleOutputUtf8()
    {
        ::SetConsoleOutputCP(CP_UTF8);
    }

    void Console::SetStdOutColor(Color foreground, Color background, bool foregroundIntensity, bool backgroundIntensity)
    {
        WORD result = 0;

        switch (foreground)
        {
            case Color::None:
            case Color::White:
                result |= FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN;
                break;
            case Color::Black:
                result |= 0;
                break;
            case Color::Green:
                result |= FOREGROUND_GREEN;
                break;
            case Color::Blue:
                result |= FOREGROUND_BLUE;
                break;
            case Color::Red:
                result |= FOREGROUND_RED;
                break;
            case Color::Purple:
                result |= FOREGROUND_BLUE | FOREGROUND_RED;
                break;
            case Color::Cyan:
                result |= FOREGROUND_BLUE | FOREGROUND_GREEN;
                break;
            case Color::Yellow:
                result |= FOREGROUND_RED | FOREGROUND_GREEN;
                break;
            default:
                break;
        }

        if (foregroundIntensity)
            result |= FOREGROUND_INTENSITY;

        switch (background)
        {
            case Color::None:
            case Color::Black:
                result |= 0;
                break;
            case Color::White:
                result |= BACKGROUND_BLUE | BACKGROUND_RED | BACKGROUND_GREEN;
                break;
            case Color::Green:
                result |= BACKGROUND_GREEN;
                break;
            case Color::Blue:
                result |= BACKGROUND_BLUE;
                break;
            case Color::Red:
                result |= BACKGROUND_RED;
                break;
            case Color::Purple:
                result |= BACKGROUND_BLUE | BACKGROUND_RED;
                break;
            case Color::Cyan:
                result |= BACKGROUND_BLUE | BACKGROUND_GREEN;
                break;
            case Color::Yellow:
                result |= BACKGROUND_RED | BACKGROUND_GREEN;
                break;
            default:
                break;
        }

        if (backgroundIntensity)
            result |= BACKGROUND_INTENSITY;

        const auto handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
        ::SetConsoleTextAttribute(handle, result);
    }

    // https://learn.microsoft.com/zh-cn/windows/console/clearing-the-screen
    void Console::ClearScreen()
    {
        const auto hConsole = ::GetConsoleWindow();
        if (hConsole == nullptr)
            return;

        CONSOLE_SCREEN_BUFFER_INFO csbi;
        SMALL_RECT scrollRect;
        COORD scrollTarget;
        CHAR_INFO fill;

        // Get the number of character cells in the current buffer.
        if (!::GetConsoleScreenBufferInfo(hConsole, &csbi))
        {
            return;
        }

        // Scroll the rectangle of the entire buffer.
        scrollRect.Left = 0;
        scrollRect.Top = 0;
        scrollRect.Right = csbi.dwSize.X;
        scrollRect.Bottom = csbi.dwSize.Y;

        // Scroll it upwards off the top of the buffer with a magnitude of the entire height.
        scrollTarget.X = 0;
        scrollTarget.Y = static_cast<SHORT>(0 - csbi.dwSize.Y);

        // Fill with empty spaces with the buffer's default text attribute.
        fill.Char.UnicodeChar = TEXT(' ');
        fill.Attributes = csbi.wAttributes;

        // Do the scroll
        ::ScrollConsoleScreenBuffer(hConsole, &scrollRect, nullptr, scrollTarget, &fill);

        // Move the cursor to the top left corner too.
        csbi.dwCursorPosition.X = 0;
        csbi.dwCursorPosition.Y = 0;

        ::SetConsoleCursorPosition(hConsole, csbi.dwCursorPosition);
    }

    void Console::ProgressBar(float progress, int barWidth)
    {
        if (progress < 0)
            progress = 0;
        if (progress > 1)
            progress = 1.0f;

        std::cout << "[";
        int pos = barWidth * progress;
        for (int i = 0; i < barWidth; ++i)
        {
            if (i < pos)
                std::cout << "=";
            else if (i == pos)
                std::cout << ">";
            else
                std::cout << " ";
        }
        std::cout << "] " << progress * 100.0 << " %\r";
        std::cout.flush();
    }
}

#endif