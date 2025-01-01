
#include <vector>
#include <iostream>
#include <Ailurus/Process.hpp>
#include <Ailurus/System.hpp>

int main()
{
    std::string commandLine = "C:\\Users\\epigr\\Desktop\\ConsoleApplication1\\x64\\Debug\\ConsoleApplication1.exe";
    auto processInfo = Infra::Process::CreateProcessWithPipe(commandLine);
    if (!processInfo.has_value())
    {
        auto err = Infra::System::GetLastError();
        std::cout << "Process create failed with last error: " << err << std::endl;
    }

    std::vector<char> readBuffer(1024);
    while (true)
    {
        auto readBytes = Infra::Process::ReadDataFromProcess(processInfo.value(), readBuffer.data(), readBuffer.size());
        if (!readBytes.has_value())
            break;

        std::string msg(readBuffer.data(), readBytes.value());
        std::cout << msg;
        std::cout.flush();
    }

    auto finish = Infra::Process::WaitProcessFinish(processInfo.value());
    std::cout << "Process finish with return code: " << finish << std::endl;

    return 0;
}

