#include <iostream>
#include <format>
#include <Ailurus/Network/DNS.h>
#include <Ailurus/Network/Network.h>

using namespace Ailurus;

int main()
{
    Network::Initialize();

    auto hostName = DNS::GetHostName();
    std::cout << std::format("Hostname: {}", hostName) << std::endl;

    auto localAddr = DNS::GetLocalIpAddress();
    for (auto addr: localAddr)
    {
        std::cout << std::format("Host address: {}", addr.ToString()) << std::endl;
    }

    system("pause");

    return 0;
}