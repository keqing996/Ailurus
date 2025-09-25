#include "PlatformApi.h"

#if AILURUS_PLATFORM_SUPPORT_POSIX

#include <cerrno>

namespace Ailurus
{
    void Npi::GlobalInit()
    {
        // do nothing
    }

    SocketHandle Npi::GetInvalidSocket()
    {
        return static_cast<SocketHandle>(-1);
    }

    void Npi::CloseSocket(int64_t handle)
    {
        ::close(ToNativeHandle(handle));
    }

    bool Npi::SetSocketBlocking(int64_t handle, bool block)
    {
        int sock = ToNativeHandle(handle);
        const int status = ::fcntl(sock, F_GETFL);
        if (block)
        {
            int ret = ::fcntl(sock, F_SETFL, status & ~O_NONBLOCK);
            if (ret == -1)
                return false;
        }
        else
        {
            int ret = ::fcntl(sock, F_SETFL, status | O_NONBLOCK);
            if (ret == -1)
                return false;
        }

        return true;
    }

    SocketState Npi::GetErrorState()
    {
        auto err = errno;

        // Incase define EWOULDBLOCK == EAGAIN, which will cause switch case
        // compile error.
        if (err == EAGAIN)
            return SocketState::Busy;

        switch (err)
        {
            case EWOULDBLOCK:
            case EINPROGRESS:
                return SocketState::Busy;
            case ECONNABORTED:
            case ECONNRESET:
            case ENOTCONN:
            case ETIMEDOUT:
            case ENETRESET:
            case EPIPE:
                return SocketState::Disconnect;
            default:
                return SocketState::Error;
        }
    }

    void Npi::SetLastSocketError(int errorCode)
    {
        errno = errorCode;
    }

    size_t Npi::GetMaxSendLength()
    {
        return 0;
    }

    size_t Npi::GetMaxReceiveLength()
    {
        return 0;
    }

    int Npi::GetSendFlags()
    {
#ifdef MSG_NOSIGNAL
        return MSG_NOSIGNAL;
#else
        return 0;
#endif
    }

    int64_t Npi::Send(int64_t handle, const void* buffer, size_t length, int flags)
    {
        return ::send(ToNativeHandle(handle), buffer, length, flags);
    }

    int64_t Npi::Receive(int64_t handle, void* buffer, size_t length, int flags)
    {
        return ::recv(ToNativeHandle(handle), buffer, length, flags);
    }
}

#endif


