#include <iostream>
#include <format>
#include <Ailurus/Utility/ScopeGuard.h>
#include <Ailurus/Network/Socket.h>
#include <Ailurus/Network/Network.h>
#include <Ailurus/Network/TcpSocket.h>
#include <Ailurus/Utility/CommandLine.h>

using namespace Ailurus;

int main(int argc, const char** argv)
{
    CommandLine commandLine;
    commandLine.AddOption("ip", 'i', "Target ip address");
    commandLine.AddOption("port", 'p', "Target port number");

    commandLine.Parse(argc, argv);

    auto& ip = commandLine["ip"]->values[0];
    auto port = std::stoi(commandLine["port"]->values[0]);

    Network::Initialize();

    auto socket = TcpSocket::Create(IpAddress::Family::IpV4);
    if (!socket.has_value())
    {
        std::cout << "Socket create failed." << std::endl;
        system("pause");
        return 0;
    }

    ScopeGuard guard([&socket]()->void { socket->Close(); });

    auto ret = socket->Connect(ip, port);
    if (ret != SocketState::Success)
    {
        std::cout << std::format("Socket connect failed with {}.", SocketStateUtil::GetName(ret)) << std::endl;
        system("pause");
        return 0;
    }

    const char* str = "Hello World!";
    auto [sendRet, sendSize] = socket->Send((void*)str, sizeof(str));
    if (sendRet != SocketState::Success)
    {
        std::cout << std::format("Socket send failed with {}.", SocketStateUtil::GetName(sendRet)) << std::endl;
        system("pause");
        return 0;
    }

    std::cout << std::format("Send: {}", str) << std::endl;

    char receiveBuf[1024];
    auto [recvRet, recvSize] = socket->Receive(receiveBuf, sizeof(receiveBuf));
    if (recvRet != SocketState::Success)
    {
        std::cout << std::format("Socket recv failed with {}.", SocketStateUtil::GetName(recvRet)) << std::endl;
        system("pause");
        return 0;
    }

    std::cout << std::format("Receive: {}", receiveBuf) << std::endl;

    system("pause");
}