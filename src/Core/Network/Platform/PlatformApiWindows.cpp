#include "PlatformApi.h"

#if AILURUS_PLATFORM_WINDOWS

#include <memory>
#include <limits>

struct WinSocketGuard
{
    WinSocketGuard()
    {
        WSADATA init;
        ::WSAStartup(MAKEWORD(2, 2), &init);
    }

    ~WinSocketGuard()
    {
        ::WSACleanup();
    }
};

std::unique_ptr<WinSocketGuard> pWinSocketGuard = nullptr;

namespace Ailurus
{
    void Npi::GlobalInit()
    {
        if (!pWinSocketGuard)
        {
            pWinSocketGuard = std::make_unique<WinSocketGuard>();
        }
    }

    void Npi::GlobalShutdown()
    {
        pWinSocketGuard.reset();
    }

    SocketHandle Npi::GetInvalidSocket()
    {
        return INVALID_SOCKET;
    }

    SocketState Npi::ConfigureListenerSocket(int64_t handle)
    {
        // Windows prefers SO_EXCLUSIVEADDRUSE so servers can reclaim ports without
        // accidentally sharing the listener with other processes, matching the intent
        // of the POSIX reuse flags in a platform-specific way.
        const SocketHandle nativeHandle = ToNativeHandle(handle);

        BOOL exclusiveAddrUse = TRUE;
        if (::setsockopt(nativeHandle, SOL_SOCKET, SO_EXCLUSIVEADDRUSE,
                reinterpret_cast<const char*>(&exclusiveAddrUse), sizeof(exclusiveAddrUse)) != 0)
        {
            return GetErrorState();
        }

        return SocketState::Success;
    }

    void Npi::CloseSocket(int64_t handle)
    {
        ::closesocket(ToNativeHandle(handle));
    }

    bool Npi::SetSocketBlocking(int64_t handle, bool block)
    {
        u_long blocking = block ? 0 : 1;
        auto ret = ::ioctlsocket(ToNativeHandle(handle), FIONBIO, &blocking);
        return ret == 0;
    }

    SocketState Npi::GetErrorState()
    {
        switch (::WSAGetLastError())
        {
            // https://stackoverflow.com/questions/14546362/how-to-resolve-wsaewouldblock-error
            // WSAEWOULDBLOCK is not really an error but simply tells you that your send buffers are full.
            // This can happen if you saturate the network or if the other side simply doesn't acknowledge
            // the received data.
            case WSAEWOULDBLOCK:
            // Operation already in progress.
            // An operation was attempted on a nonblocking socket with an operation
            // already in progress—that is, calling connect a second time on a
            // nonblocking socket that is already connecting, or canceling an
            // asynchronous request(WSAAsyncGetXbyY) that has already been canceled
            // or completed.
            case WSAEALREADY:
                return SocketState::Busy;
            // Software caused connection abort.
            case WSAECONNABORTED:
            // Connection reset by peer.
            case WSAECONNRESET:
            // Connection timed out.
            case WSAETIMEDOUT:
            // Network dropped connection on reset.
            case WSAENETRESET:
            // Socket is not connected.
            case WSAENOTCONN:
                return SocketState::Disconnect;
            // Socket is already connected.
            case WSAEISCONN:
                return SocketState::Success; // when connecting a non-blocking socket
            default:
                return SocketState::Error;
        }
    }

    void Npi::SetLastSocketError(int errorCode)
    {
        ::WSASetLastError(errorCode);
    }

    size_t Npi::GetMaxSendLength()
    {
        return static_cast<size_t>(std::numeric_limits<int>::max());
    }

    size_t Npi::GetMaxReceiveLength()
    {
        return static_cast<size_t>(std::numeric_limits<int>::max());
    }

    int Npi::GetSendFlags()
    {
        return 0;
    }

    int64_t Npi::Send(int64_t handle, const void* buffer, size_t length, int flags)
    {
        const size_t maxLength = GetMaxSendLength();
        int sendLength = length > maxLength ? static_cast<int>(maxLength) : static_cast<int>(length);
        return ::send(ToNativeHandle(handle), static_cast<const char*>(buffer), sendLength, flags);
    }

    int64_t Npi::Receive(int64_t handle, void* buffer, size_t length, int flags)
    {
        const size_t maxLength = GetMaxReceiveLength();
        int recvLength = length > maxLength ? static_cast<int>(maxLength) : static_cast<int>(length);
        return ::recv(ToNativeHandle(handle), static_cast<char*>(buffer), recvLength, flags);
    }

    bool Npi::CheckErrorInterrupted()
    {
        return false;
    }
}


#endif