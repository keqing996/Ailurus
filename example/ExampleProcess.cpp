#include "Ailurus/System/Process.h"
#include <iostream>
#include <array>

int main()
{
    auto subProcess = Ailurus::Process::Create("ls", {"-l", "-a"});
    if (subProcess)
    {
        while (subProcess->IsRunning())
        {
            std::array<char, 4096> buffer{};
            auto readResult = subProcess->ReadStdout(buffer.data(), buffer.size());
            if (readResult && *readResult > 0)
            {
                std::cout << "Process Output:\n";
                std::cout << buffer.data();
            }
        }

        auto retcode = subProcess->WaitFinish();
        std::cout << "\nretcode:" << retcode << std::endl;
    }
    else
    {
        std::cout << "Process Error\n";
    }

    return 0;
}