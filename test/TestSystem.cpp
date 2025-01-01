#include <Ailurus/System.hpp>
#include <iostream>

int main()
{
    std::cout << "GetMachineName: " << Infra::System::GetMachineName() << std::endl;
    std::cout << "GetCurrentUserName: " << Infra::System::GetCurrentUserName() << std::endl;
    std::cout << "GetEnvironmentVariable: " << Infra::System::GetEnvironmentVariable("VK_SDK_PATH") << std::endl;
    std::cout << "GetHomeDirectory: " << Infra::System::GetHomeDirectory() << std::endl;
    std::cout << "GetCurrentDirectory: " << Infra::System::GetCurrentDirectory() << std::endl;
    std::cout << "GetExecutableDirectory: " << Infra::System::GetExecutableDirectory() << std::endl;
    std::cout << "GetTempDirectory: " << Infra::System::GetTempDirectory() << std::endl;

    Infra::System::SetCurrentDirectory("D:/");
    std::cout << "GetCurrentDirectory After: " << Infra::System::GetCurrentDirectory() << std::endl;

    return 0;
}