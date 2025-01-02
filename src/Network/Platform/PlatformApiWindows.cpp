#include <memory>
#include "PlatformApi.h"

#if AILURUS_PLATFORM_WINDOWS

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
        pWinSocketGuard = std::make_unique<WinSocketGuard>();
    }

    SocketHandle Npi::GetInvalidSocket()
    {
        return INVALID_SOCKET;
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
}


#endif