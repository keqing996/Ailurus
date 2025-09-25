#pragma once

#include <cstddef>
#include <cstdint>
#include "Ailurus/PlatformDefine.h"
#include "Ailurus/Network/SocketState.h"

#if AILURUS_PLATFORM_WINDOWS

#include "Ailurus/Platform/Windows/WindowsDefine.h"
#include <ws2tcpip.h>

using SocketHandle = SOCKET;
using SockLen = int;

#endif

#if AILURUS_PLATFORM_SUPPORT_POSIX

#include <netdb.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/tcp.h>

using SocketHandle = int;
using SockLen = socklen_t;

#endif

namespace Ailurus
{
    // Network platform interface
    class Npi
    {
    public:
        Npi() = delete;

    public:
        AILURUS_FORCE_INLINE
        static SocketHandle ToNativeHandle(int64_t handle)
        {
            return static_cast<SocketHandle>(handle);
        }

        AILURUS_FORCE_INLINE
        static int64_t ToGeneralHandle(SocketHandle sock)
        {
            return static_cast<int64_t>(sock);
        }

    public:
        static void GlobalInit();

        static SocketHandle GetInvalidSocket();

        static void CloseSocket(int64_t handle);

        static bool SetSocketBlocking(int64_t handle, bool block);

        static SocketState GetErrorState();

        static void SetLastSocketError(int errorCode);

        static size_t GetMaxSendLength();
        static size_t GetMaxReceiveLength();
        static int GetSendFlags();
        static int64_t Send(int64_t handle, const void* buffer, size_t length, int flags);
        static int64_t Receive(int64_t handle, void* buffer, size_t length, int flags);
    };
}

