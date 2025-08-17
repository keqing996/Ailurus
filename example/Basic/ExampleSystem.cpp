#include <filesystem>
#include <Ailurus/System/System.h>
#include <iostream>

int main()
{
    std::cout << "GetMachineName: " << Ailurus::System::GetMachineName() << std::endl;
    std::cout << "GetCurrentUserName: " << Ailurus::System::GetCurrentUserName() << std::endl;
    std::cout << "GetEnvironmentVariable of TEMP: " << Ailurus::System::GetEnvironmentVariable("TEMP") << std::endl;
    std::cout << "GetHomeDirectory: " << Ailurus::System::GetHomeDirectory() << std::endl;
    std::cout << "GetExecutableDirectory: " << Ailurus::System::GetExecutableDirectory() << std::endl;
    std::cout << "GetTempDirectory: " << Ailurus::System::GetTempDirectory() << std::endl;

    std::cout << '\n';

    std::string currentDir = Ailurus::System::GetCurrentDirectory();
    std::cout << "GetCurrentDirectory: " << currentDir << std::endl;
    std::filesystem::path path(currentDir);
    Ailurus::System::SetCurrentDirectory(path.parent_path().string());
    std::cout << "Change current directory to parent path.\n";
    std::cout << "GetCurrentDirectory After: " << Ailurus::System::GetCurrentDirectory() << std::endl;

    getchar();

    return 0;
}