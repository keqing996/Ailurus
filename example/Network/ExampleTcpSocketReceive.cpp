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

    ScopeGuard pauseGuard([]()->void { getchar(); });

    Network::Initialize();

    auto socketOpt = TcpSocket::Create(IpAddress::Family::IpV4);
    if (!socketOpt)
    {
        std::cout << "Socket create failed." << std::endl;
        return -1;
    }

    TcpSocket socket = socketOpt.value();
    ScopeGuard guard([&socket]()->void { socket.Close(); });

    auto listenRet = socket.Listen(ip, port);
    if (listenRet != SocketState::Success)
    {
        std::cout << std::format("Socket listen failed with {}.", (int)listenRet) << std::endl;
        return -1;
    }

    auto acceptResult = socket.Accept();
    auto acceptRet = acceptResult.first;
    auto& clientSocket = acceptResult.second;
    if (acceptRet != SocketState::Success)
    {
        std::cout << std::format("Socket accept failed with {}.", (int)listenRet) << std::endl;
        return -1;
    }

    ScopeGuard clientGuard([&clientSocket]()->void { clientSocket.Close(); });

    std::vector<char> receiveBuf(1024);
    auto [recvRet, recvSize] = clientSocket.Receive(receiveBuf.data(), receiveBuf.size());
    if (recvRet != SocketState::Success)
    {
        std::cout << std::format("Socket recv failed with {}.", (int)recvRet) << std::endl;
        return -1;
    }

    std::cout << std::format("Receive: {}", receiveBuf.data()) << std::endl;

    std::string_view str = "Hello World!";
    auto [sendRet, sendSize] = clientSocket.Send((void*)str.data(), str.length());
    if (sendRet != SocketState::Success)
    {
        std::cout << std::format("Socket send failed with {}.", (int)sendRet) << std::endl;
        return -1;
    }

    std::cout << std::format("Send: {}", str) << std::endl;

    return 0;
}