#include <iostream>
#include <Ailurus/Platform/Windows/Console.h>
#include <Ailurus/Utility/Logger.h>

int main()
{
    Ailurus::Logger::LogInfo("Test", "Test log info.");
    Ailurus::Logger::LogWarn("Test", "Test log warn.");
    Ailurus::Logger::LogError("Test", "Test log error.");

    std::cout << '\n';

    Ailurus::Console::SetStdOutColor(Ailurus::Console::Color::Green, Ailurus::Console::Color::Black);
    std::cout << "Ailurus::Console::Color::Green\n";

    Ailurus::Console::SetStdOutColor(Ailurus::Console::Color::Red, Ailurus::Console::Color::Black);
    std::cout << "Ailurus::Console::Color::Red\n";

    Ailurus::Console::SetStdOutColor(Ailurus::Console::Color::Blue, Ailurus::Console::Color::Black);
    std::cout << "Ailurus::Console::Color::Blue\n";

    Ailurus::Console::SetStdOutColor(Ailurus::Console::Color::White, Ailurus::Console::Color::Black);
    std::cout << "Ailurus::Console::Color::White\n";

    Ailurus::Console::SetStdOutColor(Ailurus::Console::Color::Yellow, Ailurus::Console::Color::Black);
    std::cout << "Ailurus::Console::Color::Yellow\n";

    Ailurus::Console::SetStdOutColor(Ailurus::Console::Color::Purple, Ailurus::Console::Color::Black);
    std::cout << "Ailurus::Console::Color::Purple\n";

    Ailurus::Console::SetStdOutColor(Ailurus::Console::Color::Cyan, Ailurus::Console::Color::Black);
    std::cout << "Ailurus::Console::Color::Cyan\n";

    std::cout << '\n';

    Ailurus::Console::SetStdOutColor(Ailurus::Console::Color::Black, Ailurus::Console::Color::Green);
    std::cout << "Ailurus::Console::Color::Green\n";

    Ailurus::Console::SetStdOutColor(Ailurus::Console::Color::Black, Ailurus::Console::Color::Red);
    std::cout << "Ailurus::Console::Color::Red\n";

    Ailurus::Console::SetStdOutColor(Ailurus::Console::Color::Black, Ailurus::Console::Color::Blue);
    std::cout << "Ailurus::Console::Color::Blue\n";

    Ailurus::Console::SetStdOutColor(Ailurus::Console::Color::Black, Ailurus::Console::Color::White);
    std::cout << "Ailurus::Console::Color::White\n";

    Ailurus::Console::SetStdOutColor(Ailurus::Console::Color::Black, Ailurus::Console::Color::Yellow);
    std::cout << "Ailurus::Console::Color::Yellow\n";

    Ailurus::Console::SetStdOutColor(Ailurus::Console::Color::Black, Ailurus::Console::Color::Purple);
    std::cout << "Ailurus::Console::Color::Purple\n";

    Ailurus::Console::SetStdOutColor(Ailurus::Console::Color::Black, Ailurus::Console::Color::Cyan);
    std::cout << "Ailurus::Console::Color::Cyan\n";

    std::cout << '\n';

    Ailurus::Console::SetStdOutColor(Ailurus::Console::Color::White, Ailurus::Console::Color::Black);

    Ailurus::Console::ProgressBar(0.25, 70, false);
    Ailurus::Console::ProgressBar(0.50, 70, false);
    Ailurus::Console::ProgressBar(0.75, 70, false);
    Ailurus::Console::ProgressBar(1, 70, false);

    system("pause");

    return 0;
}