#include <cstring>
#include "SocketUtil/SocketUtil.h"
#include "Ailurus/Network/TcpSocket.h"
#include "Platform/PlatformApi.h"
#include "Ailurus/Utility/ScopeGuard.h"

namespace Ailurus
{
    static SocketState ConnectNoSelect(int64_t handle, sockaddr* pSockAddr, int structLen)
    {
        if (::connect(Npi::ToNativeHandle(handle), pSockAddr, structLen) == -1)
            return Npi::GetErrorState();

        return SocketState::Success;
    }

    static SocketState ConnectWithSelect(const Socket* pSocket, sockaddr* pSockAddr, int structLen, int timeOutInMs)
    {
        // Connect once, if connection is success, no need to select.
        if (::connect(Npi::ToNativeHandle(pSocket->GetNativeHandle()), pSockAddr, structLen) >= 0)
            return SocketState::Success;

        SocketState selectResult = Socket::SelectWrite(pSocket, timeOutInMs);
        if (selectResult != SocketState::Success)
            return selectResult;

        // `select` only tells us the socket is writeable; SO_ERROR reveals whether the
        // non-blocking connect actually succeeded or completed with an error such as ECONNREFUSED.
        int socketError = 0;
        SockLen optLen = sizeof(socketError);
        if (::getsockopt(Npi::ToNativeHandle(pSocket->GetNativeHandle()), SOL_SOCKET, SO_ERROR,
                reinterpret_cast<char*>(&socketError), &optLen) < 0)
        {
            return Npi::GetErrorState();
        }

        if (socketError != 0)
        {
            Npi::SetLastSocketError(socketError);
            return Npi::GetErrorState();
        }

        return SocketState::Success;
    }

    std::optional<TcpSocket> TcpSocket::Create(IpAddress::Family af, bool blocking)
    {
        auto addressFamily = SocketUtil::GetAddressFamily(af);
        auto [wsaProtocol, wsaSocketType] = SocketUtil::GetTcpProtocol();

        const SocketHandle handle = ::socket(addressFamily, wsaSocketType, wsaProtocol);
        if (handle == Npi::GetInvalidSocket())
            return std::nullopt;

        return Create(af, Npi::ToGeneralHandle(handle), blocking);
    }

    std::optional<TcpSocket> TcpSocket::Create(IpAddress::Family af, int64_t nativeHandle, bool blocking)
    {
        const int64_t invalidHandle = Npi::ToGeneralHandle(Npi::GetInvalidSocket());
        if (nativeHandle == invalidHandle)
            return std::nullopt;

        TcpSocket socket(af, nativeHandle, blocking);

        // Set blocking
        socket.SetBlocking(socket._isBlocking, true);

        // Disable Nagle optimization by default.
        int flagDisableNagle = 1;
        ::setsockopt(Npi::ToNativeHandle(socket._handle), SOL_SOCKET, TCP_NODELAY,
            reinterpret_cast<char*>(&flagDisableNagle), sizeof(flagDisableNagle));

#if AILURUS_PLATFORM_MAC
        // Ignore SigPipe on macos.
        // https://stackoverflow.com/questions/108183/how-to-prevent-sigpipes-or-handle-them-properly
        int flagDisableSigPipe = 1;
        ::setsockopt(Npi::ToNativeHandle(socket._handle), SOL_SOCKET, SO_NOSIGPIPE,
            reinterpret_cast<char*>(&flagDisableSigPipe), sizeof(flagDisableSigPipe));
#endif

        return socket;
    }

    TcpSocket TcpSocket::InvalidSocket(IpAddress::Family af, bool blocking)
    {
        return { af, static_cast<int64_t>(Npi::GetInvalidSocket()), blocking };
    }

    std::optional<EndPoint> TcpSocket::TryGetRemoteEndpoint() const
    {
        if (!IsValid())
            return std::nullopt;

        if (_addressFamily == IpAddress::Family::IpV4)
        {
            sockaddr_in address{};
            SockLen structLen = sizeof(sockaddr_in);
            if (::getpeername(Npi::ToNativeHandle(_handle), reinterpret_cast<sockaddr*>(&address), &structLen) != -1)
            {
                return EndPoint(IpAddress(ntohl(address.sin_addr.s_addr)), ntohs(address.sin_port));
            }

            return std::nullopt;
        }

        if (_addressFamily == IpAddress::Family::IpV6)
        {
            sockaddr_in6 address{};
            SockLen structLen = sizeof(sockaddr_in6);
            if (::getpeername(Npi::ToNativeHandle(_handle), reinterpret_cast<sockaddr*>(&address), &structLen) != -1)
            {
                return EndPoint(IpAddress(address.sin6_addr.s6_addr), ntohs(address.sin6_port));
            }

            return std::nullopt;
        }

        return std::nullopt;
    }

    std::pair<SocketState, size_t> TcpSocket::Send(void* pData, size_t size) const
    {
        if (!IsValid())
            return { SocketState::InvalidSocket, 0 };

        if (pData == nullptr || size == 0)
            return { SocketState::Error, 0 };

        const char* buffer = static_cast<const char*>(pData);
        const size_t maxChunk = Npi::GetMaxSendLength();

        // may include MSG_NOSIGNAL on POSIX to suppress SIGPIPE
        const int sendFlags = Npi::GetSendFlags(); 

        size_t bytesSent = 0;
        while (bytesSent < size)
        {
            size_t remaining = size - bytesSent;
            if (maxChunk > 0 && remaining > maxChunk)
                remaining = maxChunk;

            int64_t result = Npi::Send(_handle, buffer + bytesSent, remaining, sendFlags);

            if (result == 0)
                return { SocketState::Disconnect, bytesSent };

            if (result < 0)
                return { Npi::GetErrorState(), bytesSent };

            bytesSent += static_cast<size_t>(result);

            if (static_cast<size_t>(result) < remaining)
                break;
        }

        return { SocketState::Success, bytesSent };
    }

    std::pair<SocketState, size_t> TcpSocket::Receive(void* pBuffer, size_t size) const
    {
        if (!IsValid())
            return { SocketState::InvalidSocket, 0 };

        if (pBuffer == nullptr || size == 0)
            return { SocketState::Error, 0 };

        char* buffer = static_cast<char*>(pBuffer);
        const size_t maxChunk = Npi::GetMaxReceiveLength();

        size_t requested = size;
        if (maxChunk > 0 && requested > maxChunk)
            requested = maxChunk;

        int64_t result = Npi::Receive(_handle, buffer, requested, 0);
        if (result == 0)
            return { SocketState::Disconnect, 0 };

        if (result < 0)
            return { Npi::GetErrorState(), 0 };

        return { SocketState::Success, static_cast<size_t>(result) };
    }

    SocketState TcpSocket::Connect(const std::string& ip, uint16_t port, int timeOutInMs)
    {
        auto ipOp = IpAddress::TryParse(ip);
        if (!ipOp)
            return SocketState::Error;

        return Connect(ipOp.value(), port, timeOutInMs);
    }

    SocketState TcpSocket::Connect(const IpAddress& ip, uint16_t port, int timeOutInMs)
    {
        return Connect(EndPoint(ip, port), timeOutInMs);
    }

    SocketState TcpSocket::Connect(const EndPoint& endpoint, int timeOutInMs)
    {
        if (!IsValid())
            return SocketState::InvalidSocket;

        // Check address families match.
        if (endpoint.GetAddressFamily() != _addressFamily)
            return SocketState::AddressFamilyNotMatch;

        sockaddr sockAddr {};
        SockLen structLen;
        if (!SocketUtil::CreateSocketAddress(endpoint, &sockAddr, &structLen))
            return SocketState::Error;

        // [NonTimeout + Blocking/NonBlocking] -> just connect
        if (timeOutInMs <= 0)
            return ConnectNoSelect(_handle, &sockAddr, structLen);

        // [Timeout + NonBlocking] -> just connect
        if (!IsBlocking())
            return ConnectNoSelect(_handle, &sockAddr, structLen);

        // [Timeout + Blocking] -> set nonblocking and select
        SetBlocking(false);
        ScopeGuard guard([this]()->void { SetBlocking(true); });

        return ConnectWithSelect(this, &sockAddr, structLen, timeOutInMs);
    }

    SocketState TcpSocket::Listen(const std::string &ip, uint16_t port)
    {
        auto ipOp = IpAddress::TryParse(ip);
        if (!ipOp)
            return SocketState::Error;

        return Listen(ipOp.value(), port);
    }

    SocketState TcpSocket::Listen(const IpAddress &ip, uint16_t port)
    {
        return Listen(EndPoint(ip, port));
    }

    SocketState TcpSocket::Listen(const EndPoint& endpoint)
    {
        if (!IsValid())
            return SocketState::InvalidSocket;

        // Check address families match.
        if (endpoint.GetAddressFamily() != _addressFamily)
            return SocketState::AddressFamilyNotMatch;

        sockaddr sockAddr {};
        SockLen structLen;
        if (!SocketUtil::CreateSocketAddress(endpoint, &sockAddr, &structLen))
            return SocketState::Error;

        if (::bind(Npi::ToNativeHandle(_handle), &sockAddr, structLen) == -1)
            return Npi::GetErrorState();

        if (::listen(Npi::ToNativeHandle(_handle), SOMAXCONN) == -1)
            return Npi::GetErrorState();

        return SocketState::Success;
    }

    std::pair<SocketState, TcpSocket> TcpSocket::Accept()
    {
        if (!IsValid())
            return { SocketState::InvalidSocket, InvalidSocket(_addressFamily) };

        sockaddr_storage address {};
        SockLen length = sizeof(address);
        SocketHandle result = ::accept(Npi::ToNativeHandle(_handle), reinterpret_cast<sockaddr*>(&address), &length);

        if (result == Npi::GetInvalidSocket())
            return { Npi::GetErrorState(), InvalidSocket(_addressFamily) };

        TcpSocket resultSocket(_addressFamily, Npi::ToGeneralHandle(result), _isBlocking);
        resultSocket.SetBlocking(_isBlocking, true);

        return { SocketState::Success, resultSocket };
    }

    TcpSocket::TcpSocket(IpAddress::Family af, int64_t handle, bool blocking)
        : Socket(af, handle, blocking)
    {
    }
}
