
#include <iostream>
#include <format>
#include "Ailurus/Utility/CommandLine.h"

using namespace Ailurus;

void PrintCmdResult(const CommandLine& commandLine, const std::string& fullname)
{
    auto* pResult = commandLine[fullname];
    if (pResult != nullptr)
    {
        if (!pResult->values.empty())
        {
            std::cout << std::format("'{}' get, values: \n", fullname);
            for (const auto& valueStr: pResult->values)
                std::cout << std::format("\tvalue: {}\n", valueStr);
        }
        else
            std::cout << std::format("'{}' get, no values.\n", fullname);
    }
    else
    {
        std::cout << std::format("'{}' not in command line.\n", fullname);
    }
}

int main()
{
    constexpr int argc = 10;
    const char* argv[argc] = {
        {"cmdline.exe"},
        {"--apple"},
        {"-b"},
        {"banana para 1"},
        {"banana para 2"},
        {"banana para 3"},
        {"--strawberry"},
        {"strawberry para 1"},
        {"--orange"},
        {"orange para 1"},
    };

    CommandLine commandLine;

    commandLine.AddOption("apple", 'a', "description of apple apple");
    commandLine.AddOption("banana", 'b', "description of banana banana");
    commandLine.AddOption("pineapple", 'p', "description of pineapple pineapple");
    commandLine.AddOption("orange", 'o', "description of orange orange");

    commandLine.Parse(argc, argv);

    PrintCmdResult(commandLine, "apple");
    PrintCmdResult(commandLine, "banana");
    PrintCmdResult(commandLine, "pineapple");
    PrintCmdResult(commandLine, "orange");

    std::cout << std::endl;

    std::cout << "Help message:\n" << commandLine.GenerateHelpMsg() << std::endl;

    std::cout << std::endl;

    auto errorLogs = commandLine.GetInvalidInput();
    std::cout << "Error:\n";
    for (const auto& err: errorLogs)
        std::cout << err << "\n";

    return 0;
}