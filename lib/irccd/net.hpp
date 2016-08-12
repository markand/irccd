/*
 * net.hpp -- portable C++ socket wrapper
 *
 * Copyright (c) 2013-2016 David Demelier <markand@malikania.fr>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef IRCCD_NET_HPP
#define IRCCD_NET_HPP

/**
 * \file net.hpp
 * \brief Networking
 * \author David Demelier <markand@malikania.fr>
 */

/**
 * \defgroup net-module-tcp Network TCP support
 * \brief TCP support in the networking module.
 */

/**
 * \defgroup net-module-udp Network UDP support
 * \brief UDP support in networking module.
 */

/**
 * \defgroup net-module-tls Network TLS support
 * \brief TLS support in networking module.
 */

/**
 * \defgroup net-module-addresses Network predefined addresses
 * \brief Predefined addresses for sockets
 */

/**
 * \defgroup net-module-options Network predefined options
 * \brief Predefined options for sockets
 */

/**
 * \defgroup net-module-backends Network predefined backends
 * \brief Predefined backends for Listener
 */

/**
 * \defgroup net-module-resolv Network resolver
 * \brief Resolv functions
 */

/**
 * \page Networking Networking
 *
 *   - \subpage net-options
 *   - \subpage net-concepts
 */

/**
 * \page net-options User options
 *
 * The user may set the following variables before compiling these files:
 *
 * # General options
 *
 * - **NET_NO_AUTO_INIT**: (bool) Set to 0 if you don't want Socket class to
 *   automatically calls init function and finish functions.
 *
 * - **NET_NO_SSL**: (bool) Set to 0 if you don't have access to OpenSSL
 *   library.
 *
 * - **NET_NO_AUTO_SSL_INIT**: (bool) Set to 0 if you don't want Socket class
 *   with Tls to automatically init the OpenSSL library. You will need to call
 *   ssl::init and ssl::finish.
 *
 * # General compatibility options.
 *
 * The following options are auto detected but you can override them if you
 * want.
 *
 * - **NET_HAVE_INET_PTON**: (bool) Set to 1 if you have inet_pton function.
 *   True for all platforms and Windows
 *   if _WIN32_WINNT is greater or equal to 0x0600.
 *
 * - **NET_HAVE_INET_NTOP**: (bool) Same as above.
 *
 * **Note:** On Windows, it is highly encouraged to set _WIN32_WINNT to at least
 * 0x0600 on MinGW.
 *
 * # Options for Listener class
 *
 * Feature detection, multiple implementations may be avaible, for example,
 * Linux has poll, select and epoll.
 *
 * We assume that `select(2)` is always available.
 *
 * Of course, you can set the variables yourself if you test it with your build
 * system.
 *
 * - **NET_HAVE_POLL**: Defined on all BSD, Linux. Also defined on Windows
 *   if _WIN32_WINNT is set to 0x0600 or greater.
 * - **NET_HAVE_KQUEUE**: Defined on all BSD and Apple.
 * - **NET_HAVE_EPOLL**: Defined on Linux only.
 * - **NET_DEFAULT_BACKEND**: Which backend to use (e.g. `Select`).
 *
 * The preference priority is ordered from left to right.
 *
 * | System        | Backend                 | Class name   |
 * |---------------|-------------------------|--------------|
 * | Linux         | epoll(7)                | Epoll        |
 * | *BSD          | kqueue(2)               | Kqueue       |
 * | Windows       | poll(2), select(2)      | Poll, Select |
 * | Mac OS X      | kqueue(2)               | Kqueue       |
 */

/**
 * \page net-concepts Concepts
 *
 *   - \subpage net-concept-backend
 *   - \subpage net-concept-option
 *   - \subpage net-concept-stream
 *   - \subpage net-concept-datagram
 */

/**
 * \page net-concept-backend Backend (Concept)
 *
 * A backend is an interface for the Listener class. It is primarily designed to
 * be the most suitable for the host environment.
 *
 * The backend must be default constructible, it is highly encouraged to be move
 * constructible.
 *
 * This concepts requires the following functions:
 *
 * # name
 *
 * Get the backend name, informational only.
 *
 * ## Synopsis
 *
 * ````
 * std::string name() const;
 * ````
 *
 * ## Returns
 *
 * The backend name.
 *
 * # set
 *
 * Set one or more condition for the given handle.
 *
 * ## Synopsis
 *
 * ````
 * void set(const ListenerTable &table, Handle handle, Condition condition,
 * bool add);
 * ````
 *
 * ## Arguments
 *
 *   - **table**: the current table of sockets,
 *   - **handle**: the handle to set,
 *   - **condition**: the condition to add (may be OR'ed),
 *   - **add**: hint set to true if the handle is not currently registered.
 *
 * # unset
 *
 * Unset one or more condition for the given handle.
 *
 * ## Synopsis
 *
 * ````
 * void unset(const ListenerTable &table, Handle handle, Condition condition,
 * bool remove);
 * ````
 *
 * ## Arguments
 *
 *   - **table**: the current table of sockets,
 *   - **handle**: the handle to update,
 *   - **condition**: the condition to remove (may be OR'ed),
 *   - **remove**: hint set to true if the handle will be completely removed.
 *
 * # wait
 *
 * Wait for multiple sockets to be ready.
 *
 * ## Synopsis
 *
 * ````
 * std::vector<ListenerStatus> wait(const ListenerTable &table, int ms);
 * ````
 *
 * ## Arguments
 *
 *   - **table**: the current table,
 *   - **ms**: the number to wait in milliseconds, negative means forever.
 *
 * ## Returns
 *
 * The list of sockets ready paired to their condition.
 */

/**
 * \page net-concept-option Option (Concept)
 *
 * An option can be set or get from a socket.
 *
 * If an operation is not available, provides the function but throws an
 * exception with ENOSYS message.
 *
 * This concepts requires the following functions:
 *
 * # Option (constructor)
 *
 * At least one default constructor must be present.
 *
 * ## Synopsis
 *
 * ````
 * Option() noexcept;
 * ````
 *
 * # set
 *
 * Set the option.
 *
 * ## Synopsis
 *
 * ````
 * template <typename Address>
 * void set(Socket &sc) const;
 * ````
 *
 * ## Arguments
 *
 *   - **sc**: the socket.
 *
 * # get
 *
 * Get an option, T can be any type.
 *
 * ## Synopsis
 *
 * ````
 * template <typename Address>
 * T get(Socket &sc) const;
 * ````
 *
 * ## Arguments
 *
 *   - **sc**: the socket.
 *
 * ## Returns
 *
 * The value.
 */

/**
 * \page net-concept-stream Stream (Concept)
 *
 * This concepts requires the following functions:
 *
 * # type
 *
 * Return the type of socket, usually `SOCK_STREAM`.
 *
 * ## Synopsis
 *
 * ````
 * int type() const noexcept;
 * ````
 *
 * ## Returns
 *
 * The type of socket.
 *
 * # connect
 *
 * Connect to the given address.
 *
 * ## Synopsis
 *
 * ````
 * void connect(const sockaddr *address, socklen_t length); // (0)
 * void connect(const Address &address); // 1 (Optional)
 * ````
 *
 * ## Arguments
 *
 *   - **address**: the address,
 *   - **length**: the address length.
 *
 * ## Throws
 *
 *   - net::WouldBlockError: if the operation would block,
 *   - net::Error: on other error.
 *
 * # accept
 *
 * Accept a new client.
 *
 * If no pending connection is available and operation would block, the
 * implementation must throw WouldBlockError. Any other error can be thrown
 * otherwise a valid socket must be returned.
 *
 * ## Synopsis
 *
 * ````
 * Socket accept();
 * ````
 *
 * ## Returns
 *
 * The new socket.
 *
 * ## Throws
 *
 *   - net::WouldBlockError: if the operation would block,
 *   - net::Error: on other error.
 *
 * # recv
 *
 * Receive data.
 *
 * ## Synopsis
 *
 * ````
 * unsigned recv(void *data, unsigned length);
 * ````
 *
 * ## Arguments
 *
 *   - **data**: the destination buffer,
 *   - **length**: the destination buffer length.
 *
 * ## Returns
 *
 * The number of bytes sent.
 *
 * ## Throws
 *
 *   - net::WouldBlockError: if the operation would block,
 *   - net::Error: on other error.
 *
 * # send
 *
 * Send data.
 *
 * ## Synopsis
 *
 * ````
 * unsigned send(const void *data, unsigned length);
 * ````
 *
 * ## Arguments
 *
 *   - **data**: the data to send,
 *   - **length**: the data length.
 *
 * ## Returns
 *
 * The number of bytes sent.
 *
 * ## Throws
 *
 *   - net::WouldBlockError: if the operation would block,
 *   - net::Error: on other error.
 */

/**
 * \page net-concept-datagram Datagram (Concept)
 *
 * This concepts requires the following functions:
 *
 * # type
 *
 * Return the type of socket, usually `SOCK_DGRAM`.
 *
 * ## Synopsis
 *
 * ````
 * int type() const noexcept;
 * ````
 *
 * ## Returns
 *
 * The type of socket.
 *
 * # recvfrom
 *
 * Receive data.
 *
 * ## Synopsis
 *
 * ````
 * unsigned recvfrom(void *data, unsigned length, sockaddr *address,
 *     socklen_t *addrlen);
 * unsigned recvfrom(void *data, unsigned length, Address *source)
 * ````
 *
 * ## Arguments
 *
 *   - **data**: the data,
 *   - **length**: the length,
 *   - **address**: the source address,
 *   - **addrlen**: the source address in/out length.
 *
 * ## Returns
 *
 * The number of bytes received.
 *
 * ## Throws
 *
 *   - net::WouldBlockError: if the operation would block,
 *   - net::Error: on other error.
 *
 * # sendto
 *
 * ````
 * unsigned sendto(const void *data, unsigned length, const sockaddr *address,
 *     socklen_t addrlen);
 * unsigned sendto(const void *data, unsigned length, const Address &address);
 * ````
 *
 * ## Arguments
 *
 *   - **data**: the data to send,
 *   - **length**: the data length,
 *   - **address**: the destination address,
 *   - **addrlen**: the destination address length.
 *
 * ## Returns
 *
 * The number of bytes sent.
 *
 * ## Throws
 *
 *   - net::WouldBlockError: if the operation would block,
 *   - net::Error: on other error.
 */

/*
 * Headers to include.
 * ------------------------------------------------------------------
 */

/*
 * Include Windows headers before because it brings _WIN32_WINNT if not
 * specified by the user.
 */
#if defined(_WIN32)
#   include <WinSock2.h>
#   include <WS2tcpip.h>
#else
#   include <sys/ioctl.h>
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <sys/un.h>

#   include <arpa/inet.h>

#   include <netinet/in.h>
#   include <netinet/tcp.h>

#   include <fcntl.h>
#   include <netdb.h>
#   include <unistd.h>
#endif

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cerrno>
#include <chrono>
#include <climits>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#if !defined(NET_NO_SSL)

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/ssl.h>

#endif // !NET_NO_SSL

/*
 * Determine which I/O multiplexing implementations are available.
 * ------------------------------------------------------------------
 *
 * May define the following:
 *
 *   - NET_HAVE_EPOLL
 *   - NET_HAVE_KQUEUE
 *   - NET_HAVE_POLL
 */

#if defined(_WIN32)
#   if _WIN32_WINNT >= 0x0600 && !defined(NET_HAVE_POLL)
#       define NET_HAVE_POLL
#   endif
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__APPLE__)
#   if !defined(NET_HAVE_KQUEUE)
#       define NET_HAVE_KQUEUE
#   endif
#   if !defined(NET_HAVE_POLL)
#       define NET_HAVE_POLL
#   endif
#elif defined(__linux__)
#   if !defined(NET_HAVE_EPOLL)
#       define NET_HAVE_EPOLL
#   endif
#   if !defined(NET_HAVE_POLL)
#       define NET_HAVE_POLL
#   endif
#endif

/*
 * Compatibility macros.
 * ------------------------------------------------------------------
 */

/**
 * \brief Tells if inet_pton is available
 */
#if !defined(NET_HAVE_INET_PTON)
#   if defined(_WIN32)
#       if _WIN32_WINNT >= 0x0600
#           define NET_HAVE_INET_PTON
#       endif
#   else
#       define NET_HAVE_INET_PTON
#   endif
#endif

/**
 * \brief Tells if inet_ntop is available
 */
#if !defined(NET_HAVE_INET_NTOP)
#   if defined(_WIN32)
#       if _WIN32_WINNT >= 0x0600
#           define NET_HAVE_INET_NTOP
#       endif
#   else
#       define NET_HAVE_INET_NTOP
#   endif
#endif

/*
 * Define NET_DEFAULT_BACKEND.
 * ------------------------------------------------------------------
 *
 * Define the default I/O multiplexing implementation to use if not specified.
 */

/**
 * \brief Defines the default backend
 */
#if defined(_WIN32)
#   if !defined(NET_DEFAULT_BACKEND)
#       if defined(NET_HAVE_POLL)
#           define NET_DEFAULT_BACKEND Poll
#       else
#           define NET_DEFAULT_BACKEND Select
#       endif
#   endif
#elif defined(__linux__)
#   include <sys/epoll.h>

#   if !defined(NET_DEFAULT_BACKEND)
#       define NET_DEFAULT_BACKEND Epoll
#   endif
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__) || defined(__APPLE__)
#   include <sys/types.h>
#   include <sys/event.h>
#   include <sys/time.h>

#   if !defined(NET_DEFAULT_BACKEND)
#       define NET_DEFAULT_BACKEND Kqueue
#   endif
#else
#   if !defined(NET_DEFAULT_BACKEND)
#       define NET_DEFAULT_BACKEND Select
#   endif
#endif

#if defined(NET_HAVE_POLL) && !defined(_WIN32)
#    include <poll.h>
#endif

namespace irccd {

/**
 * The network namespace.
 */
namespace net {

/*
 * Portables types.
 * ------------------------------------------------------------------
 *
 * The following types are defined differently between Unix and Windows.
 */

#if defined(_WIN32)

/**
 * Socket type, SOCKET.
 */
using Handle = SOCKET;

/**
 * Argument to pass to set.
 */
using ConstArg = const char *;

/**
 * Argument to pass to get.
 */
using Arg = char *;

#else

/**
 * Socket type, int.
 */
using Handle = int;

/**
 * Argument to pass to set.
 */
using ConstArg = const void *;

/**
 * Argument to pass to get.
 */
using Arg = void *;

#endif

/*
 * Portable constants.
 * ------------------------------------------------------------------
 *
 * These constants are needed to check functions return codes, they are rarely
 * needed in end user code.
 */

#if defined(_WIN32)

/**
 * Socket creation failure or invalidation.
 */
const Handle Invalid{INVALID_SOCKET};

/**
 * Socket operation failure.
 */
const int Failure{SOCKET_ERROR};

#else

/**
 * Socket creation failure or invalidation.
 */
const Handle Invalid{-1};

/**
 * Socket operation failure.
 */
const int Failure{-1};

#endif

/**
 * Close the socket library.
 */
inline void finish() noexcept
{
#if defined(_WIN32)
    WSACleanup();
#endif
}

/**
 * Initialize the socket library. Except if you defined NET_NO_AUTO_INIT, you
 * don't need to call this
 * function manually.
 */
inline void init() noexcept
{
#if defined(_WIN32)
    static std::atomic<bool> initialized;
    static std::mutex mutex;

    std::lock_guard<std::mutex> lock(mutex);

    if (!initialized) {
        initialized = true;

        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);

        /*
         * If NET_NO_AUTO_INIT is not set then the user must also call finish
         * himself.
         */
#if !defined(NET_NO_AUTO_INIT)
        atexit(finish);
#endif
    }
#endif
}

/**
 * Get the last system error.
 *
 * \param errn the error number (errno or WSAGetLastError)
 * \return the error
 */
inline std::string error(int errn)
{
#if defined(_WIN32)
    LPSTR str = nullptr;
    std::string errmsg = "Unknown error";

    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        errn,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&str, 0, NULL);


    if (str) {
        errmsg = std::string(str);
        LocalFree(str);
    }

    return errmsg;
#else
    return strerror(errn);
#endif
}

/**
 * Get the last socket system error. The error is set from errno or from
 * WSAGetLastError on Windows.
 *
 * \return a string message
 */
inline std::string error()
{
#if defined(_WIN32)
    return error(WSAGetLastError());
#else
    return error(errno);
#endif
}

#if !defined(NET_NO_SSL)

/**
 * \brief SSL namespace
 */
namespace ssl {

/**
 * \enum Method
 * \brief Which OpenSSL method to use.
 */
enum Method {
    Tlsv1,      //!< TLS v1.2 (recommended)
    Sslv3       //!< SSLv3
};

/**
 * Initialize the OpenSSL library. Except if you defined NET_NO_AUTO_SSL_INIT,
 * you don't need to call this function manually.
 */
inline void init() noexcept
{
    static std::atomic<bool> initialized;
    static std::mutex mutex;

    std::lock_guard<std::mutex> lock(mutex);

    if (!initialized) {
        initialized = true;

        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();

#if !defined(NET_NO_AUTO_SSL_INIT)
        atexit(finish);
#endif
    }
}

/**
 * Close the OpenSSL library.
 */
inline void finish() noexcept
{
    ERR_free_strings();
}

} // !ssl

#endif // !NET_NO_SSL

/*
 * Error class.
 * ------------------------------------------------------------------
 *
 * This is the main exception thrown on socket operations.
 */

/**
 * \brief Base class for sockets error.
 */
class Error : public std::exception {
private:
    std::string m_message;

public:
    /**
     * Construct the error using the specified error from the system.
     *
     * \param code the error code
     * \warning the code must be a Winsock error or errno on Unix
     */
    inline Error(int code) noexcept
        : m_message(error(code))
    {
    }

    /**
     * Construct the error using the custom message.
     *
     * \param message the message
     */
    inline Error(std::string message) noexcept
        : m_message(std::move(message))
    {
    }

    /**
     * Construct the error using the last message from the system.
     */
    inline Error() noexcept
#if defined(_WIN32)
        : Error(WSAGetLastError())
#else
        : Error(errno)
#endif
    {
    }

    /**
     * Get the error (only the error content).
     *
     * \return the error
     */
    const char *what() const noexcept override
    {
        return m_message.c_str();
    }
};

/**
 * \brief Timeout occured.
 */
class TimeoutError : public std::exception {
public:
    /**
     * Get the error message.
     *
     * \return the message
     */
    const char *what() const noexcept override
    {
        return std::strerror(ETIMEDOUT);
    }
};

/**
 * \brief Operation would block.
 */
class WouldBlockError : public std::exception {
public:
    /**
     * Get the error message.
     *
     * \return the message
     */
    const char *what() const noexcept override
    {
        return std::strerror(EWOULDBLOCK);
    }
};

/**
 * \brief Operation requires sending data to complete.
 */
class WantWriteError : public std::exception {
public:
    /**
     * Get the error message.
     *
     * \return the message
     */
    const char *what() const noexcept override
    {
        return "operation requires writing to complete";
    }
};

/**
 * \brief Operation requires reading data to complete.
 */
class WantReadError : public std::exception {
public:
    /**
     * Get the error message.
     *
     * \return the message
     */
    const char *what() const noexcept override
    {
        return "operation requires read to complete";
    }
};

/*
 * Condition enum
 * ------------------------------------------------------------------
 *
 * Defines if we must wait for reading or writing.
 */

/**
 * \enum Condition
 * \brief Define the required condition for the socket.
 */
enum class Condition {
    None,                       //!< No condition is required
    Readable = (1 << 0),        //!< The socket must be readable
    Writable = (1 << 1)         //!< The socket must be writable
};

/**
 * Apply bitwise XOR.
 *
 * \param v1 the first value
 * \param v2 the second value
 * \return the new value
 */
inline Condition operator^(Condition v1, Condition v2) noexcept
{
    return static_cast<Condition>(static_cast<int>(v1) ^ static_cast<int>(v2));
}

/**
 * Apply bitwise AND.
 *
 * \param v1 the first value
 * \param v2 the second value
 * \return the new value
 */
inline Condition operator&(Condition v1, Condition v2) noexcept
{
    return static_cast<Condition>(static_cast<int>(v1) & static_cast<int>(v2));
}

/**
 * Apply bitwise OR.
 *
 * \param v1 the first value
 * \param v2 the second value
 * \return the new value
 */
inline Condition operator|(Condition v1, Condition v2) noexcept
{
    return static_cast<Condition>(static_cast<int>(v1) | static_cast<int>(v2));
}

/**
 * Apply bitwise NOT.
 *
 * \param v the value
 * \return the complement
 */
inline Condition operator~(Condition v) noexcept
{
    return static_cast<Condition>(~static_cast<int>(v));
}

/**
 * Assign bitwise OR.
 *
 * \param v1 the first value
 * \param v2 the second value
 * \return the new value
 */
inline Condition &operator|=(Condition &v1, Condition v2) noexcept
{
    v1 = static_cast<Condition>(static_cast<int>(v1) | static_cast<int>(v2));

    return v1;
}

/**
 * Assign bitwise AND.
 *
 * \param v1 the first value
 * \param v2 the second value
 * \return the new value
 */
inline Condition &operator&=(Condition &v1, Condition v2) noexcept
{
    v1 = static_cast<Condition>(static_cast<int>(v1) & static_cast<int>(v2));

    return v1;
}

/**
 * Assign bitwise XOR.
 *
 * \param v1 the first value
 * \param v2 the second value
 * \return the new value
 */
inline Condition &operator^=(Condition &v1, Condition v2) noexcept
{
    v1 = static_cast<Condition>(static_cast<int>(v1) ^ static_cast<int>(v2));

    return v1;
}

/**
 * \brief Generic socket address storage.
 * \ingroup net-module-addresses
 */
class Address {
private:
    sockaddr_storage m_storage;
    socklen_t m_length;

public:
    /**
     * Construct empty address.
     */
    inline Address() noexcept
        : m_storage{}
        , m_length(0)
    {
    }

    /**
     * Construct address from existing one.
     *
     * \pre address != nullptr
     * \param address the address
     * \param length the address length
     */
    inline Address(const sockaddr *address, socklen_t length) noexcept
        : m_length(length)
    {
        assert(address);

        std::memcpy(&m_storage, address, length);
    }

    /**
     * Get the underlying address.
     *
     * \return the address
     */
    inline const sockaddr *get() const noexcept
    {
        return reinterpret_cast<const sockaddr *>(&m_storage);
    }

    /**
     * Overloaded function
     *
     * \return the address
     */
    inline sockaddr *get() noexcept
    {
        return reinterpret_cast<sockaddr *>(&m_storage);
    }

    /**
     * Get the underlying address as the given type (e.g sockaddr_in).
     *
     * \return the address reference
     */
    template <typename T>
    inline const T &as() const noexcept
    {
        return reinterpret_cast<const T &>(m_storage);
    }

    /**
     * Overloaded function
     *
     * \return the address reference
     */
    template <typename T>
    inline T &as() noexcept
    {
        return reinterpret_cast<T &>(m_storage);
    }

    /**
     * Get the underlying address length.
     *
     * \return the length
     */
    inline socklen_t length() const noexcept
    {
        return m_length;
    }

    /**
     * Get the address domain.
     *
     * \return the domain
     */
    inline int domain() const noexcept
    {
        return m_storage.ss_family;
    }
};

/**
 * \brief Address iterator.
 * \ingroup net-module-addresses
 * \see resolve
 *
 * This iterator can be used to try to connect to an host.
 *
 * When you use resolve with unspecified domain or socket type, the function may
 * retrieve several different addresses that you can iterate over to try to
 * connect to.
 *
 * Example:
 *
 * ````cpp
 * SocketTcp sc;
 * AddressIterator end, it = resolve("hostname.test", "80");
 *
 * while (!connected_condition && it != end)
 *   sc.connect(it->address(), it->length());
 * ````
 *
 * When an iterator equals to a default constructed iterator, it is considered
 * not dereferenceable.
 */
class AddressIterator : public std::iterator<std::forward_iterator_tag, Address> {
private:
    std::vector<Address> m_addresses;
    std::size_t m_index{0};

public:
    /**
     * Construct a null iterator.
     *
     * The default constructed iterator is not dereferenceable.
     */
    inline AddressIterator() noexcept = default;

    /**
     * Construct an address iterator with a set of addresses.
     *
     * \pre index < m_addresses.size()
     * \param addresses the addresses
     * \param index the first index
     */
    inline AddressIterator(std::vector<Address> addresses, std::size_t index = 0) noexcept
        : m_addresses(std::move(addresses))
        , m_index(index)
    {
        assert(index < m_addresses.size());
    }

    /**
     * Get the generic address.
     *
     * \pre this is dereferenceable
     * \return the generic address
     */
    inline const Address &operator*() const noexcept
    {
        assert(m_index <= m_addresses.size());

        return m_addresses[m_index];
    }

    /**
     * Overloaded function.
     *
     * \pre this is dereferenceable
     * \return the generic address
     */
    inline Address &operator*() noexcept
    {
        assert(m_index <= m_addresses.size());

        return m_addresses[m_index];
    }

    /**
     * Get the generic address.
     *
     * \pre this is dereferenceable
     * \return the generic address
     */
    inline const Address *operator->() const noexcept
    {
        assert(m_index <= m_addresses.size());

        return &m_addresses[m_index];
    }

    /**
     * Overloaded function.
     *
     * \pre this is dereferenceable
     * \return the generic address
     */
    inline Address *operator->() noexcept
    {
        assert(m_index <= m_addresses.size());

        return &m_addresses[m_index];
    }

    /**
     * Pre-increment the iterator.
     *
     * \return this
     */
    inline AddressIterator &operator++(int) noexcept
    {
        if (m_index + 1 >= m_addresses.size()) {
            m_addresses.clear();
            m_index = 0;
        } else
            m_index ++;

        return *this;
    }

    /**
     * Post-increment the iterator.
     *
     * \return copy of this
     */
    inline AddressIterator operator++() noexcept
    {
        AddressIterator save = *this;

        if (m_index + 1 >= m_addresses.size()) {
            m_addresses.clear();
            m_index = 0;
        } else
            m_index ++;

        return save;
    }

    friend bool operator==(const AddressIterator &, const AddressIterator &) noexcept;
    friend bool operator!=(const AddressIterator &, const AddressIterator &) noexcept;
};

/**
 * Compare two address iterators.
 *
 * \param i1 the first iterator
 * \param i2 the second iterator
 * \return true if they equal
 */
inline bool operator==(const AddressIterator &i1, const AddressIterator &i2) noexcept
{
    return i1.m_addresses == i2.m_addresses && i1.m_index == i2.m_index;
}

/**
 * Compare two address iterators.
 *
 * \param i1 the first iterator
 * \param i2 the second iterator
 * \return false if they equal
 */
inline bool operator!=(const AddressIterator &i1, const AddressIterator &i2) noexcept
{
    return !(i1 == i2);
}

/**
 * Compare two generic addresses.
 *
 * \param a1 the first address
 * \param a2 the second address
 * \return true if they equal
 */
inline bool operator==(const Address &a1, const Address &a2) noexcept
{
    return a1.length() == a2.length() && std::memcmp(a1.get(), a2.get(), a1.length()) == 0;
}

/**
 * Compare two generic addresses.
 *
 * \param a1 the first address
 * \param a2 the second address
 * \return false if they equal
 */
inline bool operator!=(const Address &a1, const Address &a2) noexcept
{
    return !(a1 == a2);
}

/**
 * \brief Base socket class.
 */
class Socket {
protected:
    /**
     * The native handle.
     */
    Handle m_handle{Invalid};

public:
    /**
     * Create a socket handle.
     *
     * This is the primary function and the only one that creates the socket
     * handle, all other constructors are just overloaded functions.
     *
     * \param domain the domain AF_*
     * \param type the type SOCK_*
     * \param protocol the protocol
     * \throw Error on errors
     */
    Socket(int domain, int type, int protocol)
    {
#if !defined(NET_NO_AUTO_INIT)
        init();
#endif
        m_handle = ::socket(domain, type, protocol);

        if (m_handle == Invalid)
            throw Error();
    }

    /**
     * Create the socket with an already defined handle and its protocol.
     *
     * \param handle the handle
     */
    explicit inline Socket(Handle handle) noexcept
        : m_handle(handle)
    {
    }

    /**
     * Create an invalid socket. Can be used when you cannot instanciate the
     * socket immediately.
     */
    explicit inline Socket(std::nullptr_t) noexcept
        : m_handle(Invalid)
    {
    }

    /**
     * Copy constructor deleted.
     */
    Socket(const Socket &) = delete;

    /**
     * Transfer ownership from other to this.
     *
     * \param other the other socket
     */
    inline Socket(Socket &&other) noexcept
        : m_handle(other.m_handle)
    {
        other.m_handle = Invalid;
    }

    /**
     * Default destructor.
     */
    virtual ~Socket()
    {
        close();
    }

    /**
     * Tells if the socket is not invalid.
     *
     * \return true if not invalid
     */
    inline bool isOpen() const noexcept
    {
        return m_handle != Invalid;
    }

    /**
     * Set an option for the socket. Wrapper of setsockopt(2).
     *
     * \pre isOpen()
     * \param level the setting level
     * \param name the name
     * \param arg the value
     * \throw Error on errors
     */
    template <typename Argument>
    inline void set(int level, int name, const Argument &arg)
    {
        assert(m_handle != Invalid);

        if (::setsockopt(m_handle, level, name, (ConstArg)&arg, sizeof (arg)) == Failure)
            throw Error();
    }

    /**
     * Object-oriented option setter.
     *
     * The object must have `set(Socket &) const`.
     *
     * \pre isOpen()
     * \param option the option
     * \throw Error on errors
     */
    template <typename Option>
    inline void set(const Option &option)
    {
        assert(m_handle != Invalid);

        option.set(*this);
    }

    /**
     * Get an option for the socket. Wrapper of getsockopt(2).
     *
     * \pre isOpen()
     * \param level the setting level
     * \param name the name
     * \return the value
     * \throw Error on errors
     */
    template <typename Argument>
    Argument get(int level, int name)
    {
        assert(m_handle != Invalid);

        Argument desired, result{};
        socklen_t size = sizeof (result);

        if (::getsockopt(m_handle, level, name, (Arg)&desired, &size) == Failure)
            throw Error();

        std::memcpy(&result, &desired, size);

        return result;
    }

    /**
     * Object-oriented option getter.
     *
     * The object must have `T get(Socket &) const`, T can be any type and it is
     * the value returned from this function.
     *
     * \pre isOpen()
     * \return the same value as get() in the option
     * \throw Error on errors
     */
    template <typename Option>
    inline auto get() -> decltype(std::declval<Option>().get(*this))
    {
        assert(m_handle != Invalid);

        return Option().get(*this);
    }

    /**
     * Get the native handle.
     *
     * \return the handle
     * \warning Not portable
     */
    inline Handle handle() const noexcept
    {
        return m_handle;
    }

    /**
     * Bind using a native address.
     *
     * \pre isOpen()
     * \param address the address
     * \param length the size
     * \throw Error on errors
     */
    inline void bind(const sockaddr *address, socklen_t length)
    {
        assert(m_handle != Invalid);

        if (::bind(m_handle, address, length) == Failure)
            throw Error();
    }

    /**
     * Overload that takes an address.
     *
     * \pre isOpen()
     * \param address the address
     * \throw Error on errors
     */
    inline void bind(const Address &address)
    {
        assert(m_handle != Invalid);

        bind(address.get(), address.length());
    }

    /**
     * Listen for pending connection.
     *
     * \pre isOpen()
     * \param max the maximum number
     * \throw Error on errors
     */
    inline void listen(int max = 128)
    {
        assert(m_handle != Invalid);

        if (::listen(m_handle, max) == Failure)
            throw Error();
    }

    /**
     * Get the local name. This is a wrapper of getsockname().
     *
     * \pre isOpen()
     * \return the address
     * \throw Error on failures
     */
    Address getsockname() const
    {
        assert(m_handle != Invalid);

        sockaddr_storage ss;
        socklen_t length = sizeof (sockaddr_storage);

        if (::getsockname(m_handle, reinterpret_cast<sockaddr *>(&ss), &length) == Failure)
            throw Error();

        return Address(reinterpret_cast<sockaddr *>(&ss), length);
    }

    /**
     * Get connected address. This is a wrapper for getpeername().
     *
     * \pre isOpen()
     * \return the address
     * \throw Error on failures
     */
    Address getpeername() const
    {
        assert(m_handle != Invalid);

        sockaddr_storage ss;
        socklen_t length = sizeof (sockaddr_storage);

        if (::getpeername(m_handle, reinterpret_cast<sockaddr *>(&ss), &length) == Failure)
            throw Error();

        return Address(reinterpret_cast<sockaddr *>(&ss), length);
    }

    /**
     * Close the socket.
     *
     * Automatically called from the destructor.
     */
    void close()
    {
        if (m_handle != Invalid) {
#if defined(_WIN32)
            ::closesocket(m_handle);
#else
            ::close(m_handle);
#endif
            m_handle = Invalid;
        }
    }

    /**
     * Assignment operator forbidden.
     *
     * \return *this
     */
    Socket &operator=(const Socket &) = delete;

    /**
     * Transfer ownership from other to this. The other socket is left
     * invalid and will not be closed.
     *
     * \param other the other socket
     * \return this
     */
    Socket &operator=(Socket &&other) noexcept
    {
        m_handle = other.m_handle;
        other.m_handle = Invalid;

        return *this;
    }
};

/**
 * Compare two sockets.
 *
 * \param s1 the first socket
 * \param s2 the second socket
 * \return true if they equals
 */
inline bool operator==(const Socket &s1, const Socket &s2)
{
    return s1.handle() == s2.handle();
}

/**
 * Compare two sockets.
 *
 * \param s1 the first socket
 * \param s2 the second socket
 * \return true if they are different
 */
inline bool operator!=(const Socket &s1, const Socket &s2)
{
    return s1.handle() != s2.handle();
}

/**
 * Compare two sockets.
 *
 * \param s1 the first socket
 * \param s2 the second socket
 * \return true if s1 < s2
 */
inline bool operator<(const Socket &s1, const Socket &s2)
{
    return s1.handle() < s2.handle();
}

/**
 * Compare two sockets.
 *
 * \param s1 the first socket
 * \param s2 the second socket
 * \return true if s1 > s2
 */
inline bool operator>(const Socket &s1, const Socket &s2)
{
    return s1.handle() > s2.handle();
}

/**
 * Compare two sockets.
 *
 * \param s1 the first socket
 * \param s2 the second socket
 * \return true if s1 <= s2
 */
inline bool operator<=(const Socket &s1, const Socket &s2)
{
    return s1.handle() <= s2.handle();
}

/**
 * Compare two sockets.
 *
 * \param s1 the first socket
 * \param s2 the second socket
 * \return true if s1 >= s2
 */
inline bool operator>=(const Socket &s1, const Socket &s2)
{
    return s1.handle() >= s2.handle();
}

/**
 * \brief Clear TCP implementation.
 * \ingroup net-module-tcp
 *
 * This is the basic TCP protocol that implements recv, send, connect and accept
 * as wrappers of the usual C functions.
 */
class TcpSocket : public Socket {
public:
    /**
     * Inherited constructors.
     */
    using Socket::Socket;

    /**
     * Construct a TCP socket.
     *
     * \param domain the domain
     * \param protocol the protocol
     * \throw Error on errors
     */
    inline TcpSocket(int domain, int protocol)
        : Socket(domain, SOCK_STREAM, protocol)
    {
    }

    /**
     * Get the type of the socket.
     *
     * \return the type
     */
    inline int type() const noexcept
    {
        return SOCK_STREAM;
    }

    /**
     * Initiate connection.
     *
     * \param address the address
     * \param length the address length
     * \throw WouldBlockError if the socket is marked non-blocking and
     * connection cannot be established immediately
     * \throw Error on other errors
     */
    void connect(const sockaddr *address, socklen_t length)
    {
        if (::connect(this->m_handle, address, length) == Failure) {
#if defined(_WIN32)
            int error = WSAGetLastError();

            if (error == WSAEWOULDBLOCK)
                throw WouldBlockError();
            else
                throw Error(error);
#else
            if (errno == EINPROGRESS)
                throw WouldBlockError();
            else
                throw Error();
#endif
        }
    }

    /**
     * Overloaded function.
     *
     * \param address the address
     * \throw WouldBlockError if the socket is marked non-blocking and
     * connection cannot be established immediately
     * \throw Error on other errors
     */
    void connect(const Address &address)
    {
        connect(address.get(), address.length());
    }

    /**
     * Accept a new client.
     *
     * If there are no pending connection, an invalid socket is returned.
     *
     * \return the new socket
     * \throw WouldBlockError if the socket is marked non-blocking and no
     * connection are available
     * \throw Error on other errors
     */
    TcpSocket accept()
    {
        Handle handle = ::accept(this->m_handle, nullptr, 0);

        if (handle == Invalid) {
#if defined(_WIN32)
            int error = WSAGetLastError();

            if (error == WSAEWOULDBLOCK)
                throw WouldBlockError();
            else
                throw Error(error);
#else
            if (errno == EWOULDBLOCK || errno == EAGAIN)
                throw WouldBlockError();
            else
                throw Error();
#endif
        }

        return TcpSocket(handle);
    }

    /**
     * Receive some data.
     *
     * \param data the destination buffer
     * \param length the buffer length
     * \return the number of bytes received
     */
    unsigned recv(void *data, unsigned length)
    {
        int max = length > INT_MAX ? INT_MAX : static_cast<int>(length);
        int nbread = ::recv(this->m_handle, (Arg)data, max, 0);

        if (nbread == Failure) {
#if defined(_WIN32)
            int error = WSAGetLastError();

            if (error == WSAEWOULDBLOCK)
                throw WouldBlockError();
            else
                throw Error(error);
#else
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                throw WouldBlockError();
            else
                throw Error();
#endif
        }

        return static_cast<unsigned>(nbread);
    }

    /**
     * Send some data.
     *
     * \param data the data to send
     * \param length the length
     * \return the number of bytes sent
     */
    unsigned send(const void *data, unsigned length)
    {
        int max = length > INT_MAX ? INT_MAX : static_cast<int>(length);
        int nbsent = ::send(this->m_handle, (ConstArg)data, max, 0);

        if (nbsent == Failure) {
#if defined(_WIN32)
            int error = WSAGetLastError();

            if (error == WSAEWOULDBLOCK)
                throw WouldBlockError();
            else
                throw Error();
#else
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                throw WouldBlockError();
            else
                throw Error();
#endif
        }

        return static_cast<unsigned>(nbsent);
    }
};

/**
 * \brief Clear UDP type.
 *
 * This class is the basic implementation of UDP sockets.
 */
class UdpSocket : public Socket {
public:
    /**
     * Inherited constructors.
     */
    using Socket::Socket;

    /**
     * Construct a TCP socket.
     *
     * \param domain the domain
     * \param protocol the protocol
     * \throw Error on errors
     */
    inline UdpSocket(int domain, int protocol)
        : Socket(domain, SOCK_DGRAM, protocol)
    {
    }

    /**
     * Get the type of the socket.
     *
     * \return the type
     */
    inline int type() const noexcept
    {
        return SOCK_DGRAM;
    }

    /**
     * Receive some data.
     *
     * \param data the data
     * \param length the length
     * \param address the source address
     * \param addrlen the source address in/out length
     * \return the number of bytes received
     * \throw WouldBlockError if the socket is marked non-blocking and the
     * operation would block
     * \throw Error on other errors
     */
    unsigned recvfrom(void *data, unsigned length, sockaddr *address, socklen_t *addrlen)
    {
        int max = length > INT_MAX ? INT_MAX : static_cast<int>(length);
        int nbread = ::recvfrom(this->m_handle, (Arg)data, max, 0, address, addrlen);

        if (nbread == Failure) {
#if defined(_WIN32)
            int error = WSAGetLastError();

            if (error == EWOULDBLOCK)
                throw WouldBlockError();
            else
                throw Error(error);
#else
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                throw WouldBlockError();
            else
                throw Error();
#endif
        }

        return static_cast<unsigned>(nbread);
    }

    /**
     * Overloaded function.
     *
     * \param data the data
     * \param length the length
     * \param source the source information (optional)
     * \throw WouldBlockError if the socket is marked non-blocking and the
     * operation would block
     * \throw Error on other errors
     */
    inline unsigned recvfrom(void *data, unsigned length, Address *source = nullptr)
    {
        sockaddr_storage st;
        socklen_t socklen = sizeof (sockaddr_storage);

        auto nr = recvfrom(data, length, reinterpret_cast<sockaddr *>(&st), &socklen);

        if (source)
            *source = Address(reinterpret_cast<const sockaddr *>(&st), socklen);

        return nr;
    }

    /**
     * Send some data.
     *
     * \param data the data to send
     * \param length the data length
     * \param address the destination address
     * \param addrlen the destination address length
     * \return the number of bytes sent
     * \throw WouldBlockError if the socket is marked non-blocking and the
     * operation would block
     * \throw Error on other errors
     */
    unsigned sendto(const void *data, unsigned length, const sockaddr *address, socklen_t addrlen)
    {
        int max = length > INT_MAX ? INT_MAX : static_cast<int>(length);
        int nbsent = ::sendto(this->m_handle, (ConstArg)data, max, 0, address, addrlen);

        if (nbsent == Failure) {
#if defined(_WIN32)
            int error = WSAGetLastError();

            if (error == EWOULDBLOCK)
                throw WouldBlockError();
            else
                throw Error(error);
#else
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                throw WouldBlockError();
            else
                throw Error();
#endif
        }

        return static_cast<unsigned>(nbsent);
    }

    /**
     * Overloaded function
     *
     * \param data the data to send
     * \param length the data length
     * \param address the destination address
     * \return the number of bytes sent
     * \throw WouldBlockError if the socket is marked non-blocking and the
     * operation would block
     * \throw Error on other errors
     */
    inline unsigned sendto(const void *data, unsigned length, const Address &address)
    {
        return sendto(data, length, address.get(), address.length());
    }
};

#if !defined(NET_NO_SSL)

/**
 * \brief Experimental TLS support.
 * \ingroup net-module-tls
 * \warning This class is highly experimental.
 */
class TlsSocket : public Socket {
public:
    /**
     * \brief SSL connection mode.
     */
    enum Mode {
        Server,         //!< Use Server when you accept a socket server side,
        Client          //!< Use Client when you connect to a server.
    };

private:
    using Context = std::shared_ptr<SSL_CTX>;
    using Ssl = std::unique_ptr<SSL, void (*)(SSL *)>;

    // Determine if we created a TlsSocket from a temporary or a lvalue.
    bool m_mustclose{false};

    Context m_context;
    Ssl m_ssl{nullptr, nullptr};

    inline std::string error()
    {
        BIO *bio = BIO_new(BIO_s_mem());
        char *buf = nullptr;

        ERR_print_errors(bio);

        std::size_t length = BIO_get_mem_data (bio, &buf);
        std::string result(buf, length);

        BIO_free(bio);

        return result;
    }

    template <typename Function>
    void wrap(Function &&function)
    {
        auto ret = function();

        if (ret <= 0) {
            int no = SSL_get_error(m_ssl.get(), ret);

            switch (no) {
            case SSL_ERROR_WANT_READ:
                throw WantReadError();
            case SSL_ERROR_WANT_WRITE:
                throw WantWriteError();
            default:
                throw Error(error());
            }
        }
    }

    void create(Mode mode, const SSL_METHOD *method)
    {
#if !defined(NET_NO_SSL_AUTO_INIT)
        ssl::init();
#endif
        m_context = Context(SSL_CTX_new(method), SSL_CTX_free);
        m_ssl = Ssl(SSL_new(m_context.get()), SSL_free);

        SSL_set_fd(m_ssl.get(), this->m_handle);

        if (mode == Server)
            SSL_set_accept_state(m_ssl.get());
        else
            SSL_set_connect_state(m_ssl.get());
    }

public:
    /**
     * Create a socket around an existing one.
     *
     * The original socket is moved to this instance and must not be used
     * anymore.
     *
     * \param sock the TCP socket
     * \param mode the mode
     * \param method the method
     */
    TlsSocket(TcpSocket &&sock, Mode mode = Server, const SSL_METHOD *method = TLSv1_method())
        : Socket(std::move(sock))
        , m_mustclose(true)
    {
        create(mode, method);
    }

    /**
     * Wrap a socket around an existing one without taking ownership.
     *
     * The original socket must still exist until this TlsSocket is closed.
     *
     * \param sock the TCP socket
     * \param mode the mode
     * \param method the method
     */
    TlsSocket(TcpSocket &sock, Mode mode = Server, const SSL_METHOD *method = TLSv1_method())
        : Socket(sock.handle())
    {
        create(mode, method);
    }

    /**
     * Destroy the socket if owned.
     */
    ~TlsSocket()
    {
        /**
         * If the socket has been created from a rvalue this class owns the
         * socket and will close it in the parent destructor.
         *
         * Otherwise, when created from a lvalue, mark this socket as invalid
         * to avoid double close'ing it as two sockets points to the same
         * descriptor.
         */
        if (!m_mustclose)
            m_handle = Invalid;
    }

    /**
     * Get the type of socket.
     *
     * \return the type
     */
    inline int type() const noexcept
    {
        return SOCK_STREAM;
    }

    /**
     * Use the specified private key file.
     *
     * \param file the path to the private key
     * \param type the type of file
     */
    inline void setPrivateKey(std::string file, int type = SSL_FILETYPE_PEM)
    {
        if (SSL_use_PrivateKey_file(m_ssl.get(), file.c_str(), type) != 1)
            throw Error(error());
    }

    /**
     * Use the specified certificate file.
     *
     * \param file the path to the file
     * \param type the type of file
     */
    inline void setCertificate(std::string file, int type = SSL_FILETYPE_PEM)
    {
        if (SSL_use_certificate_file(m_ssl.get(), file.c_str(), type) != 1)
            throw Error(error());
    }

    /**
     * Do handshake, needed in some case when you have non blocking sockets.
     */
    void handshake()
    {
        wrap([this] () -> int {
            return SSL_do_handshake(m_ssl.get());
        });
    }

    /**
     * Receive some data.
     *
     * \param data the destination buffer
     * \param length the buffer length
     * \return the number of bytes received
     */
    unsigned recv(void *data, unsigned length)
    {
        int max = length > INT_MAX ? INT_MAX : static_cast<int>(length);
        int nbread = 0;

        wrap([&] () -> int {
            return nbread = SSL_read(m_ssl.get(), data, max);
        });

        return static_cast<unsigned>(nbread < 0 ? 0 : nbread);
    }

    /**
     * Send some data.
     *
     * \param data the data to send
     * \param length the length
     * \return the number of bytes sent
     */
    unsigned send(const void *data, unsigned length)
    {
        int max = length > INT_MAX ? INT_MAX : static_cast<int>(length);
        int nbsent = 0;

        wrap([&] () -> int {
            return nbsent = SSL_write(m_ssl.get(), data, max);
        });

        return static_cast<unsigned>(nbsent < 0 ? 0 : nbsent);
    }
};

#endif // !NET_NO_SSL

/**
 * \brief IPv4 functions.
 */
namespace ipv4 {

/**
 * Create an address to bind on any.
 *
 * \param port the port
 * \return the address
 */
inline Address any(std::uint16_t port)
{
    sockaddr_in sin;
    socklen_t length = sizeof (sockaddr_in);

    std::memset(&sin, 0, sizeof (sockaddr_in));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);

    return Address(reinterpret_cast<const sockaddr *>(&sin), length);
}

/**
 * Create an address from a IPv4 string.
 *
 * \param ip the ip address
 * \param port the port
 * \return the address
 * \throw Error if inet_pton is unavailable
 */
inline Address pton(const std::string &ip, std::uint16_t port)
{
#if defined(NET_HAVE_INET_PTON)
#if !defined(NET_NO_AUTO_INIT)
    init();
#endif

    sockaddr_in sin;
    socklen_t length = sizeof (sockaddr_in);

    std::memset(&sin, 0, sizeof (sockaddr_in));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &sin.sin_addr) <= 0)
        throw Error();

    return Address(reinterpret_cast<const sockaddr *>(&sin), length);
#else
    (void)ip;
    (void)port;

    throw Error(std::strerror(ENOSYS));
#endif
}

/**
 * Get the underlying ip from the given address.
 *
 * \pre address.domain() == AF_INET
 * \param address the IPv6 address
 * \return the ip address
 * \throw Error if inet_ntop is unavailable
 */
inline std::string ntop(const Address &address)
{
    assert(address.domain() == AF_INET);

#if !defined(NET_NO_AUTO_INIT)
    init();
#endif

#if defined(NET_HAVE_INET_NTOP)
    char result[INET_ADDRSTRLEN + 1];

    std::memset(result, 0, sizeof (result));

    if (!inet_ntop(AF_INET, const_cast<in_addr *>(&address.as<sockaddr_in>().sin_addr), result, sizeof (result)))
        throw Error();

    return result;
#else
    (void)address;

    throw Error(std::strerror(ENOSYS));
#endif
}

/**
 * Get the port from the IPv4 address.
 *
 * \pre address.domain() == AF_INET4
 * \param address the address
 * \return the port
 */
inline std::uint16_t port(const Address &address) noexcept
{
    assert(address.domain() == AF_INET);

    return ntohs(address.as<sockaddr_in>().sin_port);
}

} // !ipv4

/**
 * \brief IPv6 functions.
 */
namespace ipv6 {

/**
 * Create an address to bind on any.
 *
 * \param port the port
 * \return the address
 */
inline Address any(std::uint16_t port)
{
    sockaddr_in6 sin6;
    socklen_t length = sizeof (sockaddr_in6);

    std::memset(&sin6, 0, sizeof (sockaddr_in6));
    sin6.sin6_family = AF_INET6;
    sin6.sin6_addr = in6addr_any;
    sin6.sin6_port = htons(port);

    return Address(reinterpret_cast<const sockaddr *>(&sin6), length);
}

/**
 * Create an address from a IPv4 string.
 *
 * \param ip the ip address
 * \param port the port
 * \return the address
 * \throw Error if inet_pton is unavailable
 */
inline Address pton(const std::string &ip, std::uint16_t port)
{
#if defined(NET_HAVE_INET_PTON)
#if !defined(NET_NO_AUTO_INIT)
    init();
#endif

    sockaddr_in6 sin6;
    socklen_t length = sizeof (sockaddr_in6);

    std::memset(&sin6, 0, sizeof (sockaddr_in6));
    sin6.sin6_family = AF_INET6;
    sin6.sin6_port = htons(port);

    if (inet_pton(AF_INET6, ip.c_str(), &sin6.sin6_addr) <= 0)
        throw Error();

    return Address(reinterpret_cast<const sockaddr *>(&sin6), length);
#else
    (void)ip;
    (void)port;

    throw Error(std::strerror(ENOSYS));
#endif
}

/**
 * Get the underlying ip from the given address.
 *
 * \pre address.domain() == AF_INET6
 * \param address the IPv6 address
 * \return the ip address
 * \throw Error if inet_ntop is unavailable
 */
inline std::string ntop(const Address &address)
{
    assert(address.domain() == AF_INET6);

#if defined(NET_HAVE_INET_NTOP)
#if !defined(NET_NO_AUTO_INIT)
    init();
#endif

    char ret[INET6_ADDRSTRLEN];

    std::memset(ret, 0, sizeof (ret));

    if (!inet_ntop(AF_INET6, const_cast<in6_addr *>(&address.as<sockaddr_in6>().sin6_addr), ret, sizeof (ret)))
        throw Error();

    return ret;
#else
    (void)address;

    throw Error(std::strerror(ENOSYS));
#endif
}

/**
 * Get the port from the IPv6 address.
 *
 * \pre address.domain() == AF_INET6
 * \param address the address
 * \return the port
 */
inline std::uint16_t port(const Address &address) noexcept
{
    assert(address.domain() == AF_INET6);

    return ntohs(address.as<sockaddr_in6>().sin6_port);
}

} // !ipv6

#if !defined(_WIN32)

/**
 * \brief Unix domain functions.
 */
namespace local {

/**
 * Construct an address to a path.
 *
 * \pre !path.empty()
 * \param path the path
 * \param rm remove the file before (default: false)
 */
inline Address create(const std::string &path, bool rm = false) noexcept
{
    assert(!path.empty());

    // Silently remove the file even if it fails.
    if (rm)
        remove(path.c_str());

    sockaddr_un sun;
    socklen_t length;

    std::memset(sun.sun_path, 0, sizeof (sun.sun_path));
    std::strncpy(sun.sun_path, path.c_str(), sizeof (sun.sun_path) - 1);

    sun.sun_family = AF_LOCAL;

#if defined(NET_HAVE_SUN_LEN)
    length = SUN_LEN(&sun);
#else
    length = sizeof (sun);
#endif

    return Address(reinterpret_cast<const sockaddr *>(&sun), length);
}

/**
 * Get the path from the address.
 *
 * \pre address.domain() == AF_LOCAL
 * \param address the local address
 * \return the path to the socket file
 */
inline std::string path(const Address &address)
{
    assert(address.domain() == AF_LOCAL);

    return reinterpret_cast<const sockaddr_un *>(address.get())->sun_path;
}

} // !local

#endif // !_WIN32

/**
 * \brief Predefined options.
 */
namespace option {

/**
 * \ingroup net-module-options
 * \brief Set or get the blocking-mode for a socket.
 * \warning On Windows, it's not possible to check if the socket is blocking or
 * not.
 */
class SockBlockMode {
private:
    bool m_value;

public:
    /**
     * Create the option.
     *
     * By default the blocking mode is set to true.
     *
     * \param value set to true to make blocking sockets
     */
    inline SockBlockMode(bool value = true) noexcept
        : m_value(value)
    {
    }

    /**
     * Set the option.
     *
     * \param sc the socket
     * \throw Error on errors
     */
    void set(Socket &sc) const
    {
#if defined(O_NONBLOCK) && !defined(_WIN32)
        int flags;

        if ((flags = fcntl(sc.handle(), F_GETFL, 0)) < 0)
            flags = 0;

        if (m_value)
            flags &= ~(O_NONBLOCK);
        else
            flags |= O_NONBLOCK;

        if (fcntl(sc.handle(), F_SETFL, flags) < 0)
            throw Error();
#else
        unsigned long flags = (m_value) ? 0 : 1;

        if (ioctlsocket(sc.handle(), FIONBIO, &flags) == Failure)
            throw Error();
#endif
    }

    /**
     * Get the option.
     *
     * \param sc the socket
     * \return the value
     * \throw Error on errors
     */
    bool get(Socket &sc) const
    {
#if defined(O_NONBLOCK) && !defined(_WIN32)
        int flags = fcntl(sc.handle(), F_GETFL, 0);

        if (flags < 0)
            throw Error();

        return !(flags & O_NONBLOCK);
#else
        (void)sc;

        throw Error(std::strerror(ENOSYS));
#endif
    }
};

/**
 * \ingroup net-module-options
 * \brief Set or get the input buffer.
 */
class SockReceiveBuffer {
private:
    int m_value;

public:
    /**
     * Create the option.
     *
     * \param size the buffer size
     */
    inline SockReceiveBuffer(int size = 2048) noexcept
        : m_value(size)
    {
    }

    /**
     * Set the option.
     *
     * \param sc the socket
     * \throw Error on errors
     */
    inline void set(Socket &sc) const
    {
        sc.set(SOL_SOCKET, SO_RCVBUF, m_value);
    }

    /**
     * Get the option.
     *
     * \param sc the socket
     * \return the value
     * \throw Error on errors
     */
    inline int get(Socket &sc) const
    {
        return sc.get<int>(SOL_SOCKET, SO_RCVBUF);
    }
};

/**
 * \ingroup net-module-options
 * \brief Reuse address, must be used before calling Socket::bind
 */
class SockReuseAddress {
private:
    bool m_value;

public:
    /**
     * Create the option.
     *
     * By default the option reuses the address.
     *
     * \param value set to true to reuse the address
     */
    inline SockReuseAddress(bool value = true) noexcept
        : m_value(value)
    {
    }

    /**
     * Set the option.
     *
     * \param sc the socket
     * \throw Error on errors
     */
    inline void set(Socket &sc) const
    {
        sc.set(SOL_SOCKET, SO_REUSEADDR, m_value ? 1 : 0);
    }

    /**
     * Get the option.
     *
     * \param sc the socket
     * \return the value
     * \throw Error on errors
     */
    inline bool get(Socket &sc) const
    {
        return sc.get<int>(SOL_SOCKET, SO_REUSEADDR) != 0;
    }
};

/**
 * \ingroup net-module-options
 * \brief Set or get the output buffer.
 */
class SockSendBuffer {
private:
    int m_value;

public:
    /**
     * Create the option.
     *
     * \param size the buffer size
     */
    inline SockSendBuffer(int size = 2048) noexcept
        : m_value(size)
    {
    }

    /**
     * Set the option.
     *
     * \param sc the socket
     * \throw Error on errors
     */
    inline void set(Socket &sc) const
    {
        sc.set(SOL_SOCKET, SO_SNDBUF, m_value);
    }

    /**
     * Get the option.
     *
     * \param sc the socket
     * \return the value
     * \throw Error on errors
     */
    inline int get(Socket &sc) const
    {
        return sc.get<int>(SOL_SOCKET, SO_SNDBUF);
    }
};

/**
 * \ingroup net-module-options
 * \brief Set this option if you want to disable nagle's algorithm.
 */
class TcpNoDelay {
private:
    bool m_value;

public:
    /**
     * Create the option.
     *
     * By default disable TCP delay.
     *
     * \param value set to true to disable TCP delay
     */
    inline TcpNoDelay(bool value = true) noexcept
        : m_value(value)
    {
    }

    /**
     * Set the option.
     *
     * \param sc the socket
     * \throw Error on errors
     */
    inline void set(Socket &sc) const
    {
        sc.set(IPPROTO_TCP, TCP_NODELAY, m_value ? 1 : 0);
    }

    /**
     * Get the option.
     *
     * \param sc the socket
     * \return the value
     * \throw Error on errors
     */
    inline bool get(Socket &sc) const
    {
        return sc.get<int>(IPPROTO_TCP, TCP_NODELAY) != 0;
    }
};

/**
 * \ingroup net-module-options
 * \brief Control IPPROTO_IPV6/IPV6_V6ONLY
 *
 * Note: some systems may or not set this option by default so it's a good idea
 * to set it in any case to either
 * false or true if portability is a concern.
 */
class Ipv6Only {
private:
    bool m_value;

public:
    /**
     * Create the option.
     *
     * By default with want IPv6 only.
     *
     * \param value set to true to use IPv6 only
     */
    inline Ipv6Only(bool value = true) noexcept
        : m_value(value)
    {
    }

    /**
     * Set the option.
     *
     * \param sc the socket
     * \throw Error on errors
     */
    inline void set(Socket &sc) const
    {
        sc.set(IPPROTO_IPV6, IPV6_V6ONLY, m_value ? 1 : 0);
    }

    /**
     * Get the option.
     *
     * \param sc the socket
     * \return the value
     * \throw Error on errors
     */
    inline bool get(Socket &sc) const
    {
        return sc.get<int>(IPPROTO_IPV6, IPV6_V6ONLY) != 0;
    }
};

} // !option

/**
 * \brief Result of polling
 *
 * Result of a select call, returns the first ready socket found with its
 * flags.
 */
class ListenerStatus {
public:
    Handle socket;              //!< which socket is ready
    Condition flags;            //!< the flags
};

/**
 * Table used in the socket listener to store which sockets have been
 * set in which directions.
 */
using ListenerTable = std::unordered_map<Handle, Condition>;

/**
 * \brief Predefined backends for Listener.
 */
namespace backend {

#if defined(NET_HAVE_EPOLL)

/**
 * \ingroup net-module-backends
 * \brief Linux's epoll.
 */
class Epoll {
private:
    int m_handle{-1};
    std::vector<epoll_event> m_events;

    Epoll(const Epoll &) = delete;
    Epoll &operator=(const Epoll &) = delete;

    std::uint32_t toEpoll(Condition condition) const noexcept
    {
        std::uint32_t events = 0;

        if ((condition & Condition::Readable) == Condition::Readable)
            events |= EPOLLIN;
        if ((condition & Condition::Writable) == Condition::Writable)
            events |= EPOLLOUT;

        return events;
    }

    Condition toCondition(std::uint32_t events) const noexcept
    {
        Condition condition = Condition::None;

        if ((events & EPOLLIN) || (events & EPOLLHUP))
            condition |= Condition::Readable;
        if (events & EPOLLOUT)
            condition |= Condition::Writable;

        return condition;
    }

    void update(Handle h, int op, int eflags)
    {
        epoll_event ev;

        std::memset(&ev, 0, sizeof (epoll_event));

        ev.events = eflags;
        ev.data.fd = h;

        if (epoll_ctl(m_handle, op, h, &ev) < 0)
            throw Error();
    }

public:
    /**
     * Create epoll.
     *
     * \throw Error on failures
     */
    inline Epoll()
        : m_handle(epoll_create1(0))
    {
        if (m_handle < 0)
            throw Error();
    }

    /**
     * Move constructor.
     *
     * \param other the other backend
     */
    inline Epoll(Epoll &&other) noexcept
        : m_handle(other.m_handle)
    {
        other.m_handle = -1;
    }

    /**
     * Close the kqueue descriptor.
     */
    inline ~Epoll()
    {
        if (m_handle != -1)
            close(m_handle);
    }

    /**
     * Get the backend name.
     *
     * \return kqueue
     */
    inline std::string name() const noexcept
    {
        return "epoll";
    }

    /**
     * For set and unset, we need to apply the whole flags required, so if the
     * socket was set to Connection::Readable and user **adds**
     * Connection::Writable, we must set both.
     *
     * \param table the listener table
     * \param h the handle
     * \param condition the condition
     * \param add set to true if the socket is new to the backend
     * \throw Error on failures
     */
    void set(const ListenerTable &table, Handle h, Condition condition, bool add)
    {
        if (add) {
            update(h, EPOLL_CTL_ADD, toEpoll(condition));
            m_events.resize(m_events.size() + 1);
        } else
            update(h, EPOLL_CTL_MOD, toEpoll(table.at(h) | condition));
    }

    /**
     * Unset is a bit complicated case because Listener tells us which
     * flag to remove but to update epoll descriptor we need to pass
     * the effective flags that we want to be applied.
     *
     * So we put the same flags that are currently effective and remove the
     * requested one.
     *
     * \param table the listener table
     * \param h the handle
     * \param condition the condition
     * \param add set to true if the socket is new to the backend
     * \throw Error on failures
     */
    void unset(const ListenerTable &table, Handle h, Condition condition, bool remove)
    {
        if (remove) {
            update(h, EPOLL_CTL_DEL, 0);
            m_events.resize(m_events.size() - 1);
        } else
            update(h, EPOLL_CTL_MOD, toEpoll(table.at(h) & ~(condition)));
    }

    /**
     * Wait for sockets to be ready.
     *
     * \param ms the milliseconds timeout
     * \return the sockets ready
     * \throw Error on failures
     */
    std::vector<ListenerStatus> wait(const ListenerTable &, int ms)
    {
        int ret = epoll_wait(m_handle, m_events.data(), m_events.size(), ms);
        std::vector<ListenerStatus> result;

        if (ret == 0)
            throw TimeoutError();
        if (ret < 0)
            throw Error();

        for (int i = 0; i < ret; ++i)
            result.push_back(ListenerStatus{m_events[i].data.fd, toCondition(m_events[i].events)});

        return result;
    }

    /**
     * Move operator.
     *
     * \param other the other
     * \return this
     */
    inline Epoll &operator=(Epoll &&other)
    {
        m_handle = other.m_handle;
        other.m_handle = -1;

        return *this;
    }
};

#endif // !NET_HAVE_EPOLL

#if defined(NET_HAVE_KQUEUE)

/**
 * \ingroup net-module-backends
 * \brief Implements kqueue(2).
 *
 * This implementation is available on all BSD and Mac OS X. It is better than
 * poll(2) because it's O(1), however it's a bit more memory consuming.
 */
class Kqueue {
private:
    std::vector<struct kevent> m_result;
    int m_handle;

    Kqueue(const Kqueue &) = delete;
    Kqueue &operator=(const Kqueue &) = delete;

    void update(Handle h, int filter, int kflags)
    {
        struct kevent ev;

        EV_SET(&ev, h, filter, kflags, 0, 0, nullptr);

        if (kevent(m_handle, &ev, 1, nullptr, 0, nullptr) < 0)
            throw Error();
    }

public:
    /**
     * Create kqueue.
     *
     * \throw Error on failures
     */
    inline Kqueue()
        : m_handle(kqueue())
    {
        if (m_handle < 0)
            throw Error();
    }

    /**
     * Move constructor.
     *
     * \param other the other backend
     */
    inline Kqueue(Kqueue &&other) noexcept
        : m_handle(other.m_handle)
    {
        other.m_handle = -1;
    }

    /**
     * Close the kqueue descriptor.
     */
    inline ~Kqueue()
    {
        if (m_handle != -1)
            close(m_handle);
    }

    /**
     * Get the backend name.
     *
     * \return kqueue
     */
    inline std::string name() const noexcept
    {
        return "kqueue";
    }

    /**
     * Set socket.
     *
     * \param h the handle
     * \param condition the condition
     * \param add set to true if the socket is new to the backend
     * \throw Error on failures
     */
    void set(const ListenerTable &, Handle h, Condition condition, bool add)
    {
        if ((condition & Condition::Readable) == Condition::Readable)
            update(h, EVFILT_READ, EV_ADD | EV_ENABLE);
        if ((condition & Condition::Writable) == Condition::Writable)
            update(h, EVFILT_WRITE, EV_ADD | EV_ENABLE);
        if (add)
            m_result.resize(m_result.size() + 1);
    }

    /**
     * Unset socket.
     *
     * \param h the handle
     * \param condition the condition
     * \param remove set to true if the socket is completely removed
     * \throw Error on failures
     */
    void unset(const ListenerTable &, Handle h, Condition condition, bool remove)
    {
        if ((condition & Condition::Readable) == Condition::Readable)
            update(h, EVFILT_READ, EV_DELETE);
        if ((condition & Condition::Writable) == Condition::Writable)
            update(h, EVFILT_WRITE, EV_DELETE);
        if (remove)
            m_result.resize(m_result.size() - 1);
    }

    /**
     * Wait for sockets to be ready.
     *
     * \param ms the milliseconds timeout
     * \return the sockets ready
     * \throw Error on failures
     */
    std::vector<ListenerStatus> wait(const ListenerTable &, int ms)
    {
        std::vector<ListenerStatus> sockets;
        timespec ts = { 0, 0 };
        timespec *pts = (ms <= 0) ? nullptr : &ts;

        ts.tv_sec = ms / 1000;
        ts.tv_nsec = (ms % 1000) * 1000000;

        int nevents = kevent(m_handle, nullptr, 0, &m_result[0], m_result.capacity(), pts);

        if (nevents == 0)
            throw TimeoutError();
        if (nevents < 0)
            throw Error();

        for (int i = 0; i < nevents; ++i) {
            sockets.push_back(ListenerStatus{
                static_cast<Handle>(m_result[i].ident),
                m_result[i].filter == EVFILT_READ ? Condition::Readable : Condition::Writable
            });
        }

        return sockets;
    }

    /**
     * Move operator.
     *
     * \param other the other
     * \return this
     */
    inline Kqueue &operator=(Kqueue &&other) noexcept
    {
        m_handle = other.m_handle;
        other.m_handle = -1;

        return *this;
    }
};

#endif // !NET_HAVE_KQUEUE

#if defined(NET_HAVE_POLL)

/**
 * \ingroup net-module-backends
 * \brief Implements poll(2).
 *
 * Poll is widely supported and is better than select(2). It is still not the
 * best option as selecting the sockets is O(n).
 */
class Poll {
private:
    std::vector<pollfd> m_fds;

    short toPoll(Condition condition) const noexcept
    {
        short result = 0;

        if ((condition & Condition::Readable) == Condition::Readable)
            result |= POLLIN;
        if ((condition & Condition::Writable) == Condition::Writable)
            result |= POLLOUT;

        return result;
    }

    Condition toCondition(short &event) const noexcept
    {
        Condition condition = Condition::None;

        /*
         * Poll implementations mark the socket differently regarding the
         * disconnection of a socket.
         *
         * At least, even if POLLHUP or POLLIN is set, recv() always return 0 so
         * we mark the socket as readable.
         */
        if ((event & POLLIN) || (event & POLLHUP))
            condition |= Condition::Readable;
        if (event & POLLOUT)
            condition |= Condition::Writable;

        // Reset event for safety.
        event = 0;

        return condition;
    }

public:
    /**
     * Get the backend name.
     *
     * \return kqueue
     */
    inline std::string name() const noexcept
    {
        return "poll";
    }

    /**
     * Set socket.
     *
     * \param h the handle
     * \param condition the condition
     * \param add set to true if the socket is new to the backend
     * \throw Error on failures
     */
    void set(const ListenerTable &, Handle h, Condition condition, bool add)
    {
        if (add)
            m_fds.push_back(pollfd{h, toPoll(condition), 0});
        else {
            auto it = std::find_if(m_fds.begin(), m_fds.end(), [&] (const pollfd &pfd) {
                return pfd.fd == h;
            });

            it->events |= toPoll(condition);
        }
    }

    /**
     * Unset socket.
     *
     * \param h the handle
     * \param condition the condition
     * \param remove set to true if the socket is completely removed
     * \throw Error on failures
     */
    void unset(const ListenerTable &, Handle h, Condition condition, bool remove)
    {
        auto it = std::find_if(m_fds.begin(), m_fds.end(), [&] (const pollfd &pfd) {
            return pfd.fd == h;
        });

        if (remove)
            m_fds.erase(it);
        else
            it->events &= ~(toPoll(condition));
    }

    /**
     * Wait for sockets to be ready.
     *
     * \param ms the milliseconds timeout
     * \return the sockets ready
     * \throw Error on failures
     */
    std::vector<ListenerStatus> wait(const ListenerTable &, int ms)
    {
#if defined(_WIN32)
        auto result = WSAPoll(m_fds.data(), (ULONG)m_fds.size(), ms);
#else
        auto result = poll(m_fds.data(), m_fds.size(), ms);
#endif

        if (result == 0)
            throw TimeoutError();
        if (result < 0)
            throw Error();

        std::vector<ListenerStatus> sockets;

        for (auto &fd : m_fds)
            if (fd.revents != 0)
                sockets.push_back(ListenerStatus{fd.fd, toCondition(fd.revents)});

        return sockets;
    }
};

#endif // !NET_HAVE_POLL

/**
 * \ingroup net-module-backends
 * \brief Implements select(2)
 *
 * This class is the fallback of any other method, it is not preferred at all
 * for many reasons.
 */
class Select {
public:
    /**
     * Get the backend name.
     *
     * \return select
     */
    inline std::string name() const
    {
        return "select";
    }

    /**
     * No-op.
     */
    inline void set(const ListenerTable &, Handle, Condition, bool) noexcept
    {
    }

    /**
     * No-op.
     */
    inline void unset(const ListenerTable &, Handle, Condition, bool) noexcept
    {
    }

    /**
     * Wait for sockets to be ready.
     *
     * \param table the listener table
     * \param ms the milliseconds timeout
     * \return the sockets ready
     * \throw Error on failures
     */
    std::vector<ListenerStatus> wait(const ListenerTable &table, int ms)
    {
        timeval maxwait, *towait;
        fd_set readset;
        fd_set writeset;

        FD_ZERO(&readset);
        FD_ZERO(&writeset);

        Handle max = 0;

        for (const auto &pair : table) {
            if ((pair.second & Condition::Readable) == Condition::Readable)
                FD_SET(pair.first, &readset);
            if ((pair.second & Condition::Writable) == Condition::Writable)
                FD_SET(pair.first, &writeset);
            if (pair.first > max)
                max = pair.first;
        }

        maxwait.tv_sec = 0;
        maxwait.tv_usec = ms * 1000;

        // Set to nullptr for infinite timeout.
        towait = (ms < 0) ? nullptr : &maxwait;

        auto error = ::select(static_cast<int>(max + 1), &readset, &writeset, nullptr, towait);

        if (error == Failure)
            throw Error();
        if (error == 0)
            throw TimeoutError();

        std::vector<ListenerStatus> sockets;

        for (const auto &pair : table) {
            if (FD_ISSET(pair.first, &readset))
                sockets.push_back(ListenerStatus{pair.first, Condition::Readable});
            if (FD_ISSET(pair.first, &writeset))
                sockets.push_back(ListenerStatus{pair.first, Condition::Writable});
        }

        return sockets;
    }
};

} // !backend

/**
 * \brief Synchronous multiplexing
 *
 * Convenient wrapper around the select() system call.
 *
 * This class is implemented using a bridge pattern to allow different uses
 * of listener implementation.
 *
 * You should not reinstanciate a new Listener at each iteartion of your
 * main loop as it can be extremely costly. Instead use the same listener that
 * you can safely modify on the fly.
 *
 * Currently, poll, epoll, select and kqueue are available.
 */
template <typename Backend = backend :: NET_DEFAULT_BACKEND>
class Listener {
private:
    Backend m_backend;
    ListenerTable m_table;

public:
    /**
     * Construct an empty listener.
     */
    Listener() = default;

    /**
     * Get the backend.
     *
     * \return the backend
     */
    inline const Backend &backend() const noexcept
    {
        return m_backend;
    }

    /**
     * Get the non-modifiable table.
     *
     * \return the table
     */
    inline const ListenerTable &table() const noexcept
    {
        return m_table;
    }

    /**
     * Overloaded function.
     *
     * \return the iterator
     */
    inline ListenerTable::const_iterator begin() const noexcept
    {
        return m_table.begin();
    }

    /**
     * Overloaded function.
     *
     * \return the iterator
     */
    inline ListenerTable::const_iterator cbegin() const noexcept
    {
        return m_table.cbegin();
    }

    /**
     * Overloaded function.
     *
     * \return the iterator
     */
    inline ListenerTable::const_iterator end() const noexcept
    {
        return m_table.end();
    }

    /**
     * Overloaded function.
     *
     * \return the iterator
     */
    inline ListenerTable::const_iterator cend() const noexcept
    {
        return m_table.cend();
    }

    /**
     * Add or update a socket to the listener.
     *
     * If the socket is already placed with the appropriate flags, the
     * function is a no-op.
     *
     * If incorrect flags are passed, the function does nothing.
     *
     * \param sc the socket
     * \param condition the condition (may be OR'ed)
     * \throw Error if the backend failed to set
     */
    void set(Handle sc, Condition condition)
    {
        // Invalid or useless flags.
        if (condition == Condition::None || static_cast<int>(condition) > 0x3)
            return;

        auto it = m_table.find(sc);

        // Do not update the table if the backend failed to add or update.
        if (it == m_table.end()) {
            m_backend.set(m_table, sc, condition, true);
            m_table.emplace(sc, condition);
        } else {
            // Remove flag if already present.
            if ((condition & Condition::Readable) == Condition::Readable &&
                (it->second & Condition::Readable) == Condition::Readable)
                condition &= ~(Condition::Readable);
            if ((condition & Condition::Writable) == Condition::Writable &&
                (it->second & Condition::Writable) == Condition::Writable)
                condition &= ~(Condition::Writable);

            // Still need a call?
            if (condition != Condition::None) {
                m_backend.set(m_table, sc, condition, false);
                it->second |= condition;
            }
        }
    }

    /**
     * Unset a socket from the listener, only the flags is removed
     * unless the two flagss are requested.
     *
     * For example, if you added a socket for both reading and writing,
     * unsetting the write flags will keep the socket for reading.
     *
     * \param sc the socket
     * \param condition the condition (may be OR'ed)
     * \see remove
     */
    void unset(Handle sc, Condition condition)
    {
        auto it = m_table.find(sc);

        // Invalid or useless flags.
        if (condition == Condition::None || static_cast<int>(condition) > 0x3 || it == m_table.end())
            return;

        // Like set, do not update if the socket is already at the appropriate state.
        if ((condition & Condition::Readable) == Condition::Readable &&
            (it->second & Condition::Readable) != Condition::Readable)
            condition &= ~(Condition::Readable);
        if ((condition & Condition::Writable) == Condition::Writable &&
            (it->second & Condition::Writable) != Condition::Writable)
            condition &= ~(Condition::Writable);

        if (condition != Condition::None) {
            // Determine if it's a complete removal.
            bool removal = ((it->second) & ~(condition)) == Condition::None;

            m_backend.unset(m_table, sc, condition, removal);

            if (removal)
                m_table.erase(it);
            else
                it->second &= ~(condition);
        }
    }

    /**
     * Remove completely the socket from the listener.
     *
     * It is a shorthand for unset(sc, Condition::Readable |
     * Condition::Writable);
     *
     * \param sc the socket
     */
    inline void remove(Handle sc)
    {
        unset(sc, Condition::Readable | Condition::Writable);
    }

    /**
     * Remove all sockets.
     */
    inline void clear()
    {
        while (!m_table.empty())
            remove(m_table.begin()->first);
    }

    /**
     * Get the number of sockets in the listener.
     *
     * \return the number of sockets
     */
    inline ListenerTable::size_type size() const noexcept
    {
        return m_table.size();
    }

    /**
     * Select a socket. Waits for a specific amount of time specified as the
     * duration.
     *
     * \param duration the duration
     * \return the socket ready
     */
    template <typename Rep, typename Ratio>
    inline ListenerStatus wait(const std::chrono::duration<Rep, Ratio> &duration)
    {
        auto cvt = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
        auto max = cvt.count() > INT_MAX ? INT_MAX : static_cast<int>(cvt.count());

        return m_backend.wait(m_table, max)[0];
    }

    /**
     * Overload with milliseconds.
     *
     * \param timeout the optional timeout in milliseconds
     * \return the socket ready
     */
    inline ListenerStatus wait(long long int timeout = -1)
    {
        return wait(std::chrono::milliseconds(timeout));
    }

    /**
     * Select multiple sockets.
     *
     * \param duration the duration
     * \return the socket ready
     */
    template <typename Rep, typename Ratio>
    inline std::vector<ListenerStatus> waitMultiple(const std::chrono::duration<Rep, Ratio> &duration)
    {
        auto cvt = std::chrono::duration_cast<std::chrono::milliseconds>(duration);

        return m_backend.wait(m_table, cvt.count());
    }

    /**
     * Overload with milliseconds.
     *
     * \param timeout the optional timeout in milliseconds
     * \return the socket ready
     */
    inline std::vector<ListenerStatus> waitMultiple(int timeout = -1)
    {
        return waitMultiple(std::chrono::milliseconds(timeout));
    }
};

/**
 * \ingroup net-module-resolv
 *
 * Resolve an hostname immediately.
 *
 * \param host the hostname
 * \param service the service (e.g. http or port number)
 * \param domain the domain (e.g. AF_INET)
 * \param type the type (e.g. SOCK_STREAM)
 * \return the address iterator
 * \throw Error on failures
 */
inline AddressIterator resolve(const std::string &host,
                               const std::string &service,
                               int domain = AF_UNSPEC,
                               int type = 0)
{
#if !defined(NET_NO_AUTO_INIT)
        init();
#endif

    struct addrinfo hints, *res, *p;

    std::memset(&hints, 0, sizeof (hints));
    hints.ai_family = domain;
    hints.ai_socktype = type;

    int e = getaddrinfo(host.c_str(), service.c_str(), &hints, &res);

    if (e != 0)
        throw Error(gai_strerror(e));

    std::vector<Address> addresses;

    for (p = res; p != nullptr; p = p->ai_next)
        addresses.push_back(Address(p->ai_addr, p->ai_addrlen));

    return AddressIterator(addresses, 0);
}

/**
 * \ingroup net-module-resolv
 *
 * Resolve the first address.
 *
 * \param host the hostname
 * \param service the service name
 * \param domain the domain (e.g. AF_INET)
 * \param type the type (e.g. SOCK_STREAM)
 * \return the first generic address available
 * \throw Error on failures
 * \note do not use AF_UNSPEC and 0 as type for this function
 */
inline Address resolveOne(const std::string &host, const std::string &service, int domain, int type)
{
    AddressIterator it = resolve(host, service, domain, type);
    AddressIterator end;

    if (it == end)
        throw Error("no address available");

    return *it;
}

} // !net

} // !irccd

#endif // !IRCCD_NET_HPP
