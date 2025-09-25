#include "PlatformApi.h"

#if AILURUS_PLATFORM_SUPPORT_POSIX

#include <cerrno>

namespace Ailurus
{
    void Npi::GlobalInit()
    {
        // do nothing
    }

    void Npi::GlobalShutdown()
    {
        // nothing to tear down on POSIX platforms
    }

    SocketHandle Npi::GetInvalidSocket()
    {
        return static_cast<SocketHandle>(-1);
    }

    SocketState Npi::ConfigureListenerSocket(int64_t handle)
    {
        // Ensure server sockets can be rebound quickly after shutdown by enabling the
        // standard POSIX reuse flags before bind() is attempted.
        const SocketHandle nativeHandle = ToNativeHandle(handle);

        int reuseAddr = 1;
        if (::setsockopt(nativeHandle, SOL_SOCKET, SO_REUSEADDR,
                reinterpret_cast<const char*>(&reuseAddr), sizeof(reuseAddr)) != 0)
        {
            return GetErrorState();
        }

#if defined(SO_REUSEPORT)
        if (::setsockopt(nativeHandle, SOL_SOCKET, SO_REUSEPORT,
                reinterpret_cast<const char*>(&reuseAddr), sizeof(reuseAddr)) != 0)
        {
            // Some platforms (e.g. older macOS) may not support SO_REUSEPORT; treat those cases as success.
            if (errno != ENOPROTOOPT && errno != EINVAL)
                return GetErrorState();
        }
#endif

        return SocketState::Success;
    }

    void Npi::CloseSocket(int64_t handle)
    {
        ::close(ToNativeHandle(handle));
    }

    bool Npi::SetSocketBlocking(int64_t handle, bool block)
    {
        int sock = ToNativeHandle(handle);
        const int status = ::fcntl(sock, F_GETFL);
        if (status == -1)
            return false;

        int desiredFlags = status;
        
        if (block)
            desiredFlags &= ~O_NONBLOCK;
        else
            desiredFlags |= O_NONBLOCK;

        if (desiredFlags == status)
            return true;

        int ret = ::fcntl(sock, F_SETFL, desiredFlags);
        if (ret == -1)
            return false;

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

    bool Npi::CheckErrorInterrupted()
    {
        return errno == EINTR;
    }
}

#endif


