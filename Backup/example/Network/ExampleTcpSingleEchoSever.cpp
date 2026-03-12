#include <iostream>
#include <format>
#include <ranges>
#include <algorithm>
#include <Ailurus/Utility/Logger.h>
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
        Logger::LogError("Socket create failed.");
        return -1;
    }

    TcpSocket socket = socketOpt.value();
    ScopeGuard guard([&socket]()->void { socket.Close(); });

    auto listenRet = socket.Listen(ip, port);
    if (listenRet != SocketState::Success)
    {
        Logger::LogError(std::format("Socket listen failed with {}.\n", SocketStateUtil::GetName(listenRet)));
        return -1;
    }

    auto acceptResult = socket.Accept();
    auto acceptRet = acceptResult.first;
    auto& clientSocket = acceptResult.second;
    if (acceptRet != SocketState::Success)
    {
        Logger::LogError(std::format("Socket accept failed with {}.", SocketStateUtil::GetName(listenRet)));
        return -1;
    }

    auto remote = clientSocket.TryGetRemoteEndpoint();
    if (!remote.has_value())
    {
        Logger::LogError("Get remote ip & port failed.");
        return -1;
    }

    Logger::LogWarn(std::format(
        "Connected client: {}:{}", remote->GetIp().ToString(), remote->GetPort()));

    ScopeGuard clientGuard([&clientSocket]()->void { clientSocket.Close(); });

    std::vector<char> receiveBuf(1024);

    while (true)
    {
        std::ranges::fill(receiveBuf, 0);

        auto [recvRet, recvSize] = clientSocket.Receive(receiveBuf.data(), receiveBuf.size());
        if (recvRet != SocketState::Success)
        {
            Logger::LogError(std::format("Socket recv failed with {}.", SocketStateUtil::GetName(recvRet)));
            break;
        }

        Logger::LogInfo(std::format("Receive: {}", receiveBuf.data()));

        auto [sendRet, sendSize] = clientSocket.Send(receiveBuf.data(), recvSize);
        if (sendRet != SocketState::Success)
        {
            Logger::LogError(std::format("Socket send failed with {}.", SocketStateUtil::GetName(sendRet)));
            break;
        }
    }

    return 0;
}