#pragma once

#include "Socket.h"

namespace Ailurus
{
    class TcpSocket: public Socket
    {
    public:
        static std::optional<TcpSocket> Create(IpAddress::Family af, bool blocking = true);
        static std::optional<TcpSocket> Create(IpAddress::Family af, int64_t nativeHandle, bool blocking = true);
        static TcpSocket InvalidSocket(IpAddress::Family af, bool blocking = true);

    public:
        // Get remote ip & port
        bool TryGetRemoteEndpoint(EndPoint& outEndpoint) const;

        // [Client] Send & Recv
        std::pair<SocketState, size_t> Send(void* pData, size_t size) const;
        std::pair<SocketState, size_t> Receive(void* pBuffer, size_t size) const;

        // [Client] Connect an endpoint.
        SocketState Connect(const std::string& ip, uint16_t port, int timeOutInMs = -1);
        SocketState Connect(const IpAddress& ip, uint16_t port, int timeOutInMs = -1);
        SocketState Connect(const EndPoint& endpoint, int timeOutInMs = -1);

        // [Listener] Connect an endpoint.
        SocketState Listen(const std::string& ip, uint16_t port);
        SocketState Listen(const IpAddress& ip, uint16_t port);
        SocketState Listen(const EndPoint& endpoint);

        // [Listener] Accept
        std::pair<SocketState, TcpSocket> Accept();

    private:
        TcpSocket(IpAddress::Family af, int64_t handle, bool blocking);
    };
}