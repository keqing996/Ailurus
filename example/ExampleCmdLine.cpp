
#include <iostream>
#include "Ailurus/Utility/CommandLine.h"

using namespace Ailurus;

int main()
{
    const char* argv[8] = {
        {"cmdline.exe"},
        {"--apple"},
        {"-b"},
        {"banana para 1"},
        {"banana para 2"},
        {"banana para 3"},
        {"--orange"},
        {"orange para 1"}
    };

    CommandLine commandLine;

    commandLine.AddOption("apple", 'a', "apple apple");
    commandLine.AddOption("banana", 'b', "banana banana");
    commandLine.AddOption("orange", 'o', "orange orange");

    commandLine.Parse(8, argv);


    return 0;
}