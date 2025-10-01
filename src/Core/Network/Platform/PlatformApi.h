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
    /**
     * @brief Network Platform Interface (Npi) - Cross-platform socket abstraction layer
     * 
     * This class provides a unified interface for network operations across different platforms,
     * handling the differences between Windows WinSock and POSIX socket implementations.
     */
    class Npi
    {
    public:
        Npi() = delete;

    public:
        /**
         * @brief Converts a generic 64-bit handle to platform-specific socket handle
         * @param handle Generic 64-bit socket handle
         * @return Platform-specific socket handle (SOCKET on Windows, int on POSIX)
         */
        AILURUS_FORCE_INLINE
        static SocketHandle ToNativeHandle(int64_t handle)
        {
            return static_cast<SocketHandle>(handle);
        }

        /**
         * @brief Converts platform-specific socket handle to generic 64-bit handle
         * @param sock Platform-specific socket handle
         * @return Generic 64-bit socket handle for cross-platform use
         */
        AILURUS_FORCE_INLINE
        static int64_t ToGeneralHandle(SocketHandle sock)
        {
            return static_cast<int64_t>(sock);
        }

    public:
        /**
         * @brief Initialize global network subsystem
         * 
         * On Windows: Initializes WinSock library (WSAStartup)
         * On POSIX: No operation required
         * Must be called before any other network operations
         */
        static void GlobalInit();

        /**
         * @brief Shutdown global network subsystem
         * 
         * On Windows: Cleans up WinSock library (WSACleanup)
         * On POSIX: No operation required
         * Should be called when network operations are no longer needed
         */
        static void GlobalShutdown();

        /**
         * @brief Get the platform-specific invalid socket value
         * @return INVALID_SOCKET on Windows, -1 on POSIX platforms
         */
        static SocketHandle GetInvalidSocket();

        /**
         * @brief Close a socket and release its resources
         * @param handle Generic socket handle to close
         * 
         * Uses closesocket() on Windows, close() on POSIX
         */
        static void CloseSocket(int64_t handle);

        /**
         * @brief Set socket blocking/non-blocking mode
         * @param handle Generic socket handle
         * @param block true for blocking mode, false for non-blocking
         * @return true on success, false on failure
         * 
         * On Windows: Uses ioctlsocket() with FIONBIO
         * On POSIX: Uses fcntl() with O_NONBLOCK flag
         */
        static bool SetSocketBlocking(int64_t handle, bool block);

        /**
         * @brief Get current socket error state from system error codes
         * @return SocketState enum representing the current error condition
         * 
         * Maps platform-specific error codes to unified SocketState:
         * - Success: Operation completed successfully
         * - Busy: Operation would block (WSAEWOULDBLOCK/EAGAIN)
         * - Disconnect: Connection was closed or reset
         * - Error: Other error conditions
         */
        static SocketState GetErrorState();

        /**
         * @brief Set the last socket error code for the current thread
         * @param errorCode Platform-specific error code to set
         * 
         * On Windows: Uses WSASetLastError()
         * On POSIX: Sets errno
         */
        static void SetLastSocketError(int errorCode);

        /**
         * @brief Configure socket options for listener/server sockets
         * @param handle Generic socket handle to configure
         * @return SocketState::Success on success, error state on failure
         * 
         * On Windows: Sets SO_EXCLUSIVEADDRUSE to prevent port sharing
         * On POSIX: Sets SO_REUSEADDR and SO_REUSEPORT for quick rebinding
         * Should be called before bind() for server sockets
         */
        static SocketState ConfigureListenerSocket(int64_t handle);

        /**
         * @brief Get maximum number of bytes that can be sent in a single send() call
         * @return Maximum send length (INT_MAX on Windows, 0/unlimited on POSIX)
         */
        static size_t GetMaxSendLength();

        /**
         * @brief Get maximum number of bytes that can be received in a single recv() call
         * @return Maximum receive length (INT_MAX on Windows, 0/unlimited on POSIX)
         */
        static size_t GetMaxReceiveLength();

        /**
         * @brief Get platform-specific flags for send operations
         * @return MSG_NOSIGNAL on POSIX (prevents SIGPIPE), 0 on Windows
         */
        static int GetSendFlags();

        /**
         * @brief Send data through a socket
         * @param handle Generic socket handle
         * @param buffer Pointer to data buffer to send
         * @param length Number of bytes to send
         * @param flags Platform-specific send flags
         * @return Number of bytes sent on success, -1 on error
         * 
         * On Windows: Limits length to INT_MAX and casts buffer to char*
         * On POSIX: Direct call to send() with original parameters
         */
        static int64_t Send(int64_t handle, const void* buffer, size_t length, int flags);

        /**
         * @brief Receive data from a socket
         * @param handle Generic socket handle
         * @param buffer Pointer to buffer for received data
         * @param length Maximum number of bytes to receive
         * @param flags Platform-specific receive flags
         * @return Number of bytes received on success, -1 on error
         * 
         * On Windows: Limits length to INT_MAX and casts buffer to char*
         * On POSIX: Direct call to recv() with original parameters
         */
        static int64_t Receive(int64_t handle, void* buffer, size_t length, int flags);

        /**
         * @brief Check if the last operation was interrupted by a signal
         * @return true if interrupted (errno == EINTR on POSIX), false on Windows
         * 
         * On POSIX: Checks for EINTR error (signal interruption)
         * On Windows: Always returns false (no signal interruption concept)
         */
        static bool CheckErrorInterrupted();
    };
}

