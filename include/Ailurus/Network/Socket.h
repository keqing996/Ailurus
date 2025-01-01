#pragma once

#include "SocketState.h"
#include "EndPoint.h"

namespace Ailurus
{
    class Socket
    {
    public:
        virtual ~Socket() = default;

    public:
        // Return is socket in blocking mode.
        bool IsBlocking() const;

        // Set socket blocking mode.
        bool SetBlocking(bool block, bool force = false);

        // Close socket.
        void Close();

        // Return native handle by platform, SOCKET on widnows, int on posix plat.
        int64_t GetNativeHandle() const;

        bool IsValid() const;

        // Select
        static SocketState SelectRead(const Socket* pSocket, int timeoutInMs = -1);
        static SocketState SelectWrite(const Socket* pSocket, int timeoutInMs = -1);
        SocketState SelectRead(int timeoutInMs = -1);
        SocketState SelectWrite(int timeoutInMs = -1);

    protected:
        Socket(IpAddress::Family af, int64_t handle, bool blocking);

    protected:
        IpAddress::Family _addressFamily;
        int64_t _handle;
        bool _isBlocking;
    };
}