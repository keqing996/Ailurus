
#include "Ailurus/Network/Socket.h"
#include "Platform/PlatformApi.h"

namespace Ailurus
{
    int64_t Socket::GetNativeHandle() const
    {
        return _handle;
    }

    bool Socket::IsValid() const
    {
        return _handle != Npi::ToGeneralHandle(Npi::GetInvalidSocket());
    }

    bool Socket::IsBlocking() const
    {
        return _isBlocking;
    }

    bool Socket::SetBlocking(bool block, bool force)
    {
        if (!force && block == IsBlocking())
            return true;

        bool result = Npi::SetSocketBlocking(_handle, block);
        if (result)
            _isBlocking = block;

        return result;
    }

    void Socket::Close()
    {
        if (!IsValid())
            return;

        Npi::CloseSocket(_handle);
        _handle = Npi::ToGeneralHandle(Npi::GetInvalidSocket());
    }

    SocketState Socket::SelectRead(const Socket* pSocket, int timeoutInMs)
    {
        if (pSocket == nullptr)
            return SocketState::Error;

        if (timeoutInMs < -1)
            return SocketState::Error;

        int64_t handle = pSocket->GetNativeHandle();

        fd_set selector;
        FD_ZERO(&selector);
        FD_SET(Npi::ToNativeHandle(handle), &selector);

        timeval time{};
        timeval* pTimeout = nullptr;
        if (timeoutInMs >= 0)
        {
            time.tv_sec = static_cast<long>(timeoutInMs / 1000);
            time.tv_usec = static_cast<long>((timeoutInMs % 1000) * 1000);
            pTimeout = &time;
        }

        int selectResult = ::select(static_cast<int>(Npi::ToNativeHandle(handle) + 1), &selector, nullptr, nullptr, pTimeout);
        if (selectResult > 0)
            return SocketState::Success;

        if (selectResult == 0)
            return SocketState::Busy;

        return Npi::GetErrorState();
    }

    SocketState Socket::SelectWrite(const Socket* pSocket, int timeoutInMs)
    {
        if (pSocket == nullptr)
            return SocketState::Error;

        if (timeoutInMs < -1)
            return SocketState::Error;

        int64_t handle = pSocket->GetNativeHandle();

        fd_set selector;
        FD_ZERO(&selector);
        FD_SET(Npi::ToNativeHandle(handle), &selector);

        timeval time{};
        timeval* pTimeout = nullptr;
        if (timeoutInMs >= 0)
        {
            time.tv_sec = static_cast<long>(timeoutInMs / 1000);
            time.tv_usec = static_cast<long>((timeoutInMs % 1000) * 1000);
            pTimeout = &time;
        }

        int selectResult = ::select(static_cast<int>(Npi::ToNativeHandle(handle) + 1), nullptr, &selector, nullptr, pTimeout);
        if (selectResult > 0)
            return SocketState::Success;

        if (selectResult == 0)
            return SocketState::Busy;

        return Npi::GetErrorState();
    }

    SocketState Socket::SelectRead(int timeoutInMs)
    {
        return SelectRead(this, timeoutInMs);
    }

    SocketState Socket::SelectWrite(int timeoutInMs)
    {
        return SelectWrite(this, timeoutInMs);
    }

    Socket::Socket(IpAddress::Family af, int64_t handle, bool blocking)
        : _addressFamily(af)
        , _handle(handle)
        , _isBlocking(blocking)
    {
    }
}
