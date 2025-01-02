#include "PlatformApi.h"

#if AILURUS_PLATFORM_SUPPORT_POSIX

namespace Ailurus
{
    void Npi::GlobalInit()
    {
        // do nothing
    }

    SocketHandle Npi::GetInvalidSocket()
    {
        return 0;
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
        // todo
        return SocketState::Error;
    }
}

#endif


