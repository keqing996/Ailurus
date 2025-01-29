#include "Ailurus/System/Process.h"
#include <iostream>
#include <array>

int main()
{
    auto currPid = Ailurus::Process::GetCurrentProcessId();
    auto currName = Ailurus::Process::GetProcessName(currPid);

    std::cout << "CurrentProcessPid = " << currPid << std::endl;
    std::cout << "CurrentProcessName = " << currName << std::endl;
    std::cout << std::endl;

    std::cout << "Start process: " << std::endl;
    auto subProcess = Ailurus::Process::Create("ls", {"-l", "-a"});
    if (subProcess)
    {
        while (subProcess->IsRunning())
        {
            std::array<char, 4096> buffer{};
            auto readResult = subProcess->ReadStdout(buffer.data(), buffer.size());
            if (readResult && *readResult > 0)
                std::cout << buffer.data();
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