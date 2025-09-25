
#include <chrono>
#include "Ailurus/Network/Socket.h"
#include "Platform/PlatformApi.h"

namespace Ailurus
{
	static SocketState SelectImpl(const Socket* pSocket, int timeoutInMs, bool waitForRead)
	{
		if (pSocket == nullptr)
			return SocketState::Error;

		const int64_t handle = pSocket->GetNativeHandle();
		const SocketHandle nativeHandle = Npi::ToNativeHandle(handle);

		const bool hasTimeout = timeoutInMs >= 0;
		std::chrono::steady_clock::time_point deadline;
		if (hasTimeout)
			deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutInMs);

		while (true)
		{
			if (hasTimeout)
			{
				auto now = std::chrono::steady_clock::now();
				if (now >= deadline)
					return SocketState::Busy;
			}

			fd_set readSet;
			fd_set writeSet;
			FD_ZERO(&readSet);
			FD_ZERO(&writeSet);

			if (waitForRead)
				FD_SET(nativeHandle, &readSet);
			else
				FD_SET(nativeHandle, &writeSet);

			timeval timeoutValue{};
			timeval* pTimeout = nullptr;
			if (hasTimeout)
			{
				auto remaining = std::chrono::duration_cast<std::chrono::microseconds>(deadline - std::chrono::steady_clock::now());
				if (remaining.count() <= 0)
					return SocketState::Busy;

				// Express the remaining time in the timeval format required by select.
				timeoutValue.tv_sec = static_cast<long>(remaining.count() / 1000000);
				timeoutValue.tv_usec = static_cast<long>(remaining.count() % 1000000);
				pTimeout = &timeoutValue;
			}

			// Block until the socket is ready, an error occurs, or the timeout expires.
			int selectResult = ::select(static_cast<int>(nativeHandle + 1),
				waitForRead ? &readSet : nullptr,
				waitForRead ? nullptr : &writeSet,
				nullptr,
				pTimeout);

			// Socket is ready for the requested operation.
			if (selectResult > 0)
				return SocketState::Success;
            
			// Timed out without any activity.
			if (selectResult == 0)
				return SocketState::Busy;

			// Retry when select is interrupted by a signal (POSIX) or spurious wakeup.
			if (Npi::CheckErrorInterrupted())
				continue;

			return Npi::GetErrorState();
		}
	}

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
        return SelectImpl(pSocket, timeoutInMs, true);
    }

    SocketState Socket::SelectWrite(const Socket* pSocket, int timeoutInMs)
    {
        return SelectImpl(pSocket, timeoutInMs, false);
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
