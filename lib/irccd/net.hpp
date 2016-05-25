/*
 * net.hpp -- portable C++ socket wrappers
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

#ifndef NET_HPP
#define NET_HPP

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
 *   - \subpage net-configuration
 *   - \subpage net-options
 *   - \subpage net-concepts
 */

/**
 * \page net-configuration Configuration
 *
 * # General compatibility options.
 *
 * The following options are auto detected but you can override them if you want.
 *
 * - **NET_HAVE_INET_PTON**: (bool) Set to 1 if you have inet_pton function. True for all platforms and Windows
 *   if _WIN32_WINNT is greater or equal to 0x0600.
 *
 * - **NET_HAVE_INET_NTOP**: (bool) Same as above.
 *
 * **Note:** On Windows, it is highly encouraged to set _WIN32_WINNT to at least 0x0600 on MinGW.
 *
 * # Options for Listener class
 *
 * Feature detection, multiple implementations may be avaible, for example, Linux has poll, select and epoll.
 *
 * We assume that `select(2)` is always available.
 *
 * Of course, you can set the variables yourself if you test it with your build system.
 *
 * - **NET_HAVE_POLL**: Defined on all BSD, Linux. Also defined on Windows
 *   if _WIN32_WINNT is set to 0x0600 or greater.
 *
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
 * \page net-options User options
 *
 * The user may set the following variables before compiling these files:
 *
 * # General options
 *
 * - **NET_NO_AUTO_INIT**: (bool) Set to 0 if you don't want Socket class to
 * automatically calls net::init function and net::finish functions.
 *
 * - **NET_NO_SSL**: (bool) Set to 0 if you don't have access to OpenSSL library.
 *
 * - **NET_NO_AUTO_SSL_INIT**: (bool) Set to 0 if you don't want Socket class with Tls to automatically init
 *   the OpenSSL library. You will need to call net::ssl::init and net::ssl::finish.
 */

/**
 * \page net-concepts Concepts
 *
 *   - \subpage net-concept-address
 *   - \subpage net-concept-backend
 *   - \subpage net-concept-option
 *   - \subpage net-concept-stream
 *   - \subpage net-concept-datagram
 */

/**
 * \page net-concept-address Address (Concept)
 *
 * An address is used in many place for creating, binding, connecting, receiving and sending. They are implemented as
 * templates to allow any type of address and to make sure the same address is used for a given socket.
 *
 * This concepts requires the following functions:
 *
 * # Address (constructor)
 *
 * The address must have the following constructor overloads.
 *
 * **Note**: the user can add custom constructors.
 *
 * ## Synopsis
 *
 * ````
 * Address(); // (0);
 * Address(const sockaddr *sa, socklen_t length); // (1)
 * ````
 *
 * ## Arguments
 *
 *   - **ss**: the storage to construct from,
 *   - **length**: the storage length.
 *
 * # domain
 *
 * Get the domain (e.g. AF_INET).
 *
 * ## Synopsis
 *
 * ````
 * int domain() const noexcept;
 * ````
 *
 * ## Returns
 *
 * The domain.
 *
 * # address
 *
 * Get the underlying address.
 *
 * ## Synopsis
 *
 * ````
 * const sockaddr *address() const noexcept;
 * ````
 *
 * ## Returns
 *
 * The address.
 *
 * # length
 *
 * Get the underlying length.
 *
 * ## Synopsis
 *
 * ````
 * socklen_t length() const noexcept;
 * ````
 *
 * ## Returns
 *
 * The length.
 */

/**
 * \page net-concept-backend Backend (Concept)
 *
 * A backend is an interface for the Listener class. It is primarily designed to be the most suitable for the host
 * environment.
 *
 * The backend must be default constructible, it is highly encouraged to be move constructible.
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
 * void set(const ListenerTable &table, Handle handle, Condition condition, bool add);
 * ````
 *
 * ## Arguments
 *
 *   - **table**: the current table of sockets,
 *   - **handle**: the handle to set,
 *   - **condition**: the condition to add (may be OR'ed),
 *   - **add**: hint set to true if the handle is not currently registered at all.
 *
 * # unset
 *
 * Unset one or more condition for the given handle.
 *
 * ## Synopsis
 *
 * ````
 * void unset(const ListenerTable &table, Handle handle, Condition condition, bool remove);
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
 * If an operation is not available, provides the function but throws an exception with ENOSYS errno code.
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
 * template <typename Address, typename Protocol>
 * inline void set(Socket<Address, Protocol> &sc) const;
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
 * template <typename Address, typename Protocol>
 * inline bool get(Socket<Address, Protocol> &sc) const;
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
 * # create
 *
 * Function called immediately after creation of socket. Interface must provides this function even if the
 * interface does not require anything.
 *
 * ## Synopsis
 *
 * ````
 * template <typename Address>
 * void create(Socket<Address, Tcp> &) const noexcept;
 * ````
 *
 * # connect
 *
 * Initial connect function.
 *
 * In this function, the interface receive the socket, address and a condition (initially set to None). If the
 * underlying socket is marked non-blocking and the operation would block, the interface must set the condition
 * to the required one.
 *
 * ## Synopsis
 *
 * ````
 * template <typename Address, typename Protocol>
 * void connect(Socket<Address, Protocol> &sc, const sockaddr *address, socklen_t length, Condition &cond);
 * ````
 *
 * ## Arguments
 *
 *   - **sc**: the socket,
 *   - **address**: the address,
 *   - **length**: the address length,
 *   - **cond**: the condition to update.
 *
 * # resumeConnect
 *
 * Continue the connection.
 *
 * ## Synopsis
 *
 * ````
 * template <typename Address, typename Protocol>
 * void resumeConnect(Socket<Address, Protocol> &sc, Condition &cond)
 * ````
 *
 * ## Arguments
 *
 *   - **sc**: the socket,
 *   - **cond**: the condition to update.
 *
 * # accept
 *
 * Accept a new client.
 *
 * If the interface has no pending connection, an invalid socket SHOULD be returned, otherwise return the client and
 * set the condition if the accept process is not complete yet.
 *
 * The interface MUST stores the client information into address and length parameters, they are guaranted to never be
 * null.
 *
 * ## Synopsis
 *
 * ````
 * template <typename Address, typename Protocol>
 * Socket<Address, Protocol> accept(Socket<Address, Protocol> &sc, sockaddr *address, socklen_t *length, Condition &cond);
 * ````
 *
 * ## Arguments
 *
 *   - **sc**: the socket,
 *   - **address**: the information address,
 *   - **length**: the address initial length (sockaddr_storage),
 *   - **cond**: the condition to update.
 *
 * # resumeAccept
 *
 * Continue the accept process on the returned client.
 *
 * ## Synopsis
 *
 * ````
 * template <typename Address, typename Protocol>
 * void accept(Socket<Address, Protocol> &sc, Condition &cond) const noexcept;
 * ````
 *
 * ## Arguments
 *
 *   - **sc**: the socket,
 *   - **cond**: the condition to update.
 *
 * # recv
 *
 * Receive data.
 *
 * ## Synopsis
 *
 * ````
 * template <typename Address>
 * std::size_t recv(Socket<Address, Tcp> &sc, void *data, std::size_t length, Condition &cond);
 * ````
 *
 * ## Arguments
 *
 *   - **sc**: the socket,
 *   - **data**: the destination buffer,
 *   - **length**: the destination buffer length,
 *   - **cond**: the condition to update.
 *
 * ## Returns
 *
 * The number of bytes sent.
 *
 * # send
 *
 * Send data.
 *
 * ## Synopsis
 *
 * ````
 * template <typename Address>
 * std::size_t send(Socket<Address, Tcp> &sc, const void *data, std::size_t length, Condition &cond);
 * ````
 *
 * ## Arguments
 *
 *   - **sc**: the socket,
 *   - **data**: the data to send,
 *   - **length**: the data length,
 *   - **cond**: the condition to update.
 *
 * ## Returns
 *
 * The number of bytes sent.
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
 * template <typename Address, typename Protocol>
 * std::size_t recvfrom(Socket<Address, Protocol> &sc, void *data, std::size_t length, sockaddr *address, socklen_t *addrlen, Condition &cond);
 * ````
 *
 * ## Arguments
 *
 *   - **sc**: the socket,
 *   - **data**: the data,
 *   - **length**: the length,
 *   - **address**: the source address,
 *   - **addrlen**: the source address in/out length,
 *   - **cond**: the condition.
 *
 * ## Returns
 *
 * The number of bytes received.
 *
 * # sendto
 *
 * ````
 * template <typename Address, typename Protocol>
 * std::size_t sendto(Socket<Address, Protocol> &sc, const void *data, std::size_t length, const sockaddr *address, socklen_t addrlen, Condition &cond);
 * ````
 *
 * ## Arguments
 *
 *   - **sc**: the socket,
 *   - **data**: the data to send,
 *   - **length**: the data length,
 *   - **address**: the destination address,
 *   - **addrlen**: the destination address length,
 *   - **cond**: the condition.
 *
 * ## Returns
 *
 * The number of bytes sent.
 */

/*
 * Headers to include.
 * ------------------------------------------------------------------
 */

// Include Windows headers before because it brings _WIN32_WINNT if not specified by the user.
#if defined(_WIN32)
#  include <WinSock2.h>
#  include <WS2tcpip.h>
#else
#  include <sys/ioctl.h>
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <sys/un.h>

#  include <arpa/inet.h>

#  include <netinet/in.h>
#  include <netinet/tcp.h>

#  include <fcntl.h>
#  include <netdb.h>
#  include <unistd.h>
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
#  if _WIN32_WINNT >= 0x0600 && !defined(NET_HAVE_POLL)
#    define NET_HAVE_POLL
#  endif
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__APPLE__)
#  if !defined(NET_HAVE_KQUEUE)
#    define NET_HAVE_KQUEUE
#  endif
#  if !defined(NET_HAVE_POLL)
#    define NET_HAVE_POLL
#  endif
#elif defined(__linux__)
#  if !defined(NET_HAVE_EPOLL)
#    define NET_HAVE_EPOLL
#  endif
#  if !defined(NET_HAVE_POLL)
#    define NET_HAVE_POLL
#  endif
#endif

/*
 * Compatibility macros.
 * ------------------------------------------------------------------
 */

/**
 * \brief Tells if inet_pton is available
 */
#if !defined(NET_HAVE_INET_PTON)
#  if defined(_WIN32)
#    if _WIN32_WINNT >= 0x0600
#      define NET_HAVE_INET_PTON
#    endif
#  else
#    define NET_HAVE_INET_PTON
#  endif
#endif

/**
 * \brief Tells if inet_ntop is available
 */
#if !defined(NET_HAVE_INET_NTOP)
#  if defined(_WIN32)
#    if _WIN32_WINNT >= 0x0600
#      define NET_HAVE_INET_NTOP
#    endif
#  else
#    define NET_HAVE_INET_NTOP
#  endif
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
#  if !defined(NET_DEFAULT_BACKEND)
#    if defined(NET_HAVE_POLL)
#      define NET_DEFAULT_BACKEND Poll
#    else
#      define NET_DEFAULT_BACKEND Select
#    endif
#  endif
#elif defined(__linux__)
#  include <sys/epoll.h>

#  if !defined(NET_DEFAULT_BACKEND)
#    define NET_DEFAULT_BACKEND Epoll
#  endif
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__) || defined(__APPLE__)
#  include <sys/types.h>
#  include <sys/event.h>
#  include <sys/time.h>

#  if !defined(NET_DEFAULT_BACKEND)
#    define NET_DEFAULT_BACKEND Kqueue
#  endif
#else
#  if !defined(NET_DEFAULT_BACKEND)
#    define NET_DEFAULT_BACKEND Select
#  endif
#endif

#if defined(NET_HAVE_POLL) && !defined(_WIN32)
#  include <poll.h>
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
 * These constants are needed to check functions return codes, they are rarely needed in end user code.
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
 * Initialize the socket library. Except if you defined NET_NO_AUTO_INIT, you don't need to call this
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

		// If NET_NO_AUTO_INIT is not set then the user must also call finish himself.
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
 * Get the last socket system error. The error is set from errno or from WSAGetLastError on Windows.
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

namespace ssl {

/**
 * \enum Method
 * \brief Which OpenSSL method to use.
 */
enum Method {
	Tlsv1,		//!< TLS v1.2 (recommended)
	Sslv3		//!< SSLv3
};

/**
 * Initialize the OpenSSL library. Except if you defined NET_NO_AUTO_SSL_INIT, you don't need to call this function
 * manually.
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
 * \brief Base class for sockets error
 */
class Error : public std::exception {
public:
	/**
	 * \enum Code
	 * \brief Which kind of error
	 */
	enum Code {
		Timeout,		///!< The action did timeout
		System,			///!< There is a system error
		Other			///!< Other custom error
	};

private:
	Code m_code;
	std::string m_function;
	std::string m_error;

public:
	/**
	 * Constructor that use the last system error.
	 *
	 * \param code which kind of error
	 * \param function the function name
	 */
	inline Error(Code code, std::string function)
		: m_code(code)
		, m_function(std::move(function))
		, m_error(error())
	{
	}

	/**
	 * Constructor that use the system error set by the user.
	 *
	 * \param code which kind of error
	 * \param function the function name
	 * \param n the error
	 */
	inline Error(Code code, std::string function, int n)
		: m_code(code)
		, m_function(std::move(function))
		, m_error(error(n))
	{
	}

	/**
	 * Constructor that set the error specified by the user.
	 *
	 * \param code which kind of error
	 * \param function the function name
	 * \param error the error
	 */
	inline Error(Code code, std::string function, std::string error)
		: m_code(code)
		, m_function(std::move(function))
		, m_error(std::move(error))
	{
	}

	/**
	 * Get which function has triggered the error.
	 *
	 * \return the function name (e.g connect)
	 */
	inline const std::string &function() const noexcept
	{
		return m_function;
	}

	/**
	 * The error code.
	 *
	 * \return the code
	 */
	inline Code code() const noexcept
	{
		return m_code;
	}

	/**
	 * Get the error (only the error content).
	 *
	 * \return the error
	 */
	const char *what() const noexcept override
	{
		return m_error.c_str();
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
 *
 * As explained in Action enumeration, some operations required to be called several times, before calling these
 * operations, the user must wait the socket to be readable or writable. This can be checked with Socket::condition.
 */
enum class Condition {
	None,			//!< No condition is required
	Readable = (1 << 0),	//!< The socket must be readable
	Writable = (1 << 1)	//!< The socket must be writable
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

/*
 * Base Socket class
 * ------------------------------------------------------------------
 *
 * This base class has operations that are common to all types of sockets but you usually instanciate
 * a SocketTcp or SocketUdp
 */

/**
 * \brief Base socket class for socket operations.
 *
 * **Important:** When using non-blocking sockets, some considerations must be taken. See the implementation of the
 * underlying protocol for more details.
 *
 * When using non-blocking sockets, it is important to pass the condition to functions which may block, they indicate
 * the condition to wait to perform or continue the operation if they would block.
 *
 * For example, when trying to connect with non-blocking, user should do the following:
 *
 * 1. Call Socket::connect() with the condition,
 * 2. Loop until condition is not set to Condition::None (or an exception is thrown),
 * 3. Wait with a listener for the condition to be ready (see Listener::poll),
 * 4. Call Socket::resumeConnect() with the condition again.
 *
 * \see protocol::Tls
 * \see protocol::Tcp
 * \see protocol::Udp
 */
template <typename Address, typename Protocol>
class Socket {
private:
	Protocol m_proto;

protected:
	/**
	 * The native handle.
	 */
	Handle m_handle{Invalid};

public:
	/**
	 * Create a socket handle.
	 *
	 * This is the primary function and the only one that creates the socket handle, all other constructors
	 * are just overloaded functions.
	 *
	 * \param domain the domain AF_*
	 * \param type the type SOCK_*
	 * \param protocol the protocol
	 * \param iface the implementation
	 * \throw net::Error on errors
	 */
	Socket(int domain, int type, int protocol, Protocol iface = {})
		: m_proto(std::move(iface))
	{
#if !defined(NET_NO_AUTO_INIT)
		init();
#endif
		m_handle = ::socket(domain, type, protocol);

		if (m_handle == Invalid)
			throw Error(Error::System, "socket");

		m_proto.create(*this);
	}

	/**
	 * This tries to create a socket.
	 *
	 * Domain and type are determined by the Address and Protocol object.
	 *
	 * \param address which type of address
	 * \param protocol the protocol
	 * \throw net::Error on errors
	 */
	explicit inline Socket(const Address &address = {}, Protocol protocol = {})
		: Socket(address.domain(), protocol.type(), 0, std::move(protocol))
	{
	}

	/**
	 * Create the socket with an already defined handle and its protocol.
	 *
	 * \param handle the handle
	 * \param protocol the protocol
	 */
	explicit inline Socket(Handle handle, Protocol protocol = {}) noexcept
		: m_proto(std::move(protocol))
		, m_handle(handle)
	{
	}

	/**
	 * Create an invalid socket. Can be used when you cannot instanciate the socket immediately.
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
		: m_proto(std::move(other.m_proto))
		, m_handle(other.m_handle)
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
	 * Access the implementation.
	 *
	 * \return the implementation
	 * \warning use this function with care
	 */
	inline const Protocol &protocol() const noexcept
	{
		return m_proto;
	}

	/**
	 * Overloaded function.
	 *
	 * \return the implementation
	 */
	inline Protocol &protocol() noexcept
	{
		return m_proto;
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
	 * \throw net::Error on errors
	 */
	template <typename Argument>
	inline void set(int level, int name, const Argument &arg)
	{
		assert(m_handle != Invalid);

		if (setsockopt(m_handle, level, name, (ConstArg)&arg, sizeof (arg)) == Failure)
			throw Error(Error::System, "set");
	}

	/**
	 * Object-oriented option setter.
	 *
	 * The object must have `set(Socket<Address, Protocol> &) const`.
	 *
	 * \pre isOpen()
	 * \param option the option
	 * \throw net::Error on errors
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
	 * \throw net::Error on errors
	 */
	template <typename Argument>
	Argument get(int level, int name)
	{
		assert(m_handle != Invalid);

		Argument desired, result{};
		socklen_t size = sizeof (result);

		if (getsockopt(m_handle, level, name, (Arg)&desired, &size) == Failure)
			throw Error(Error::System, "get");

		std::memcpy(&result, &desired, size);

		return result;
	}

	/**
	 * Object-oriented option getter.
	 *
	 * The object must have `T get(Socket<Address, Protocol> &) const`, T can be any type and it is the value
	 * returned from this function.
	 *
	 * \pre isOpen()
	 * \return the same value as get() in the option
	 * \throw net::Error on errors
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
	 * \throw net::Error on errors
	 */
	inline void bind(const sockaddr *address, socklen_t length)
	{
		assert(m_handle != Invalid);

		if (::bind(m_handle, address, length) == Failure)
			throw Error(Error::System, "bind");
	}

	/**
	 * Overload that takes an address.
	 *
	 * \pre isOpen()
	 * \param address the address
	 * \throw net::Error on errors
	 */
	inline void bind(const Address &address)
	{
		assert(m_handle != Invalid);

		if (::bind(m_handle, address.address(), address.length()) == Failure)
			throw Error(Error::System, "bind");
	}

	/**
	 * Listen for pending connection.
	 *
	 * \pre isOpen()
	 * \param max the maximum number
	 * \throw net::Error on errors
	 */
	inline void listen(int max = 128)
	{
		assert(m_handle != Invalid);

		if (::listen(this->m_handle, max) == Failure)
			throw Error(Error::System, "listen");
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
			throw Error(Error::System, "getsockname");

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
			throw Error(Error::System, "getpeername");

		return Address(reinterpret_cast<sockaddr *>(&ss), length);
	}

	/**
	 * Initialize connection to the given address.
	 *
	 * \pre isOpen()
	 * \param address the address
	 * \param length the address length
	 * \param cond the condition
	 * \throw net::Error on failures
	 */
	inline void connect(const sockaddr *address, socklen_t length, Condition &cond)
	{
		assert(m_handle != Invalid);

		cond = Condition::None;

		m_proto.connect(*this, address, length, cond);
	}

	/**
	 * Overloaded function.
	 *
	 * \pre isOpen()
	 * \param address the address
	 * \param length the address length
	 * \throw net::Error on failures
	 */
	inline void connect(const sockaddr *address, socklen_t length)
	{
		Condition dummy;

		connect(address, length, dummy);
	}

	/**
	 * Overloaded function.
	 *
	 * \pre isOpen()
	 * \param address the address
	 * \param cond the condition
	 * \throw net::Error on failures
	 */
	inline void connect(const Address &address, Condition &cond)
	{
		connect(address.address(), address.length(), cond);
	}

	/**
	 * Overloaded function.
	 *
	 * \pre isOpen()
	 * \param address the address
	 * \throw net::Error on failures
	 */
	inline void connect(const Address &address)
	{
		Condition dummy;

		connect(address.address(), address.length(), dummy);
	}

	/**
	 * Continue connect process.
	 *
	 * \pre isOpen()
	 * \param cond the condition require for next selection
	 * \throw net::Error on failures
	 */
	inline void resumeConnect(Condition &cond)
	{
		assert(m_handle != Invalid);

		cond = Condition::None;

		m_proto.resumeConnect(*this, cond);
	}

	/**
	 * Continue connect process.
	 *
	 * \pre isOpen()
	 * \throw net::Error on failures
	 */
	inline void resumeConnect()
	{
		Condition dummy;

		resumeConnect(dummy);
	}

	/**
	 * Accept a new client.
	 *
	 * If no connection is available immediately, returns an invalid socket.
	 *
	 * \pre isOpen()
	 * \param address the client information
	 * \param cond the condition to wait to complete accept on the **client**
	 * \return the new client or an invalid if no client is immediately available
	 * \throw net::Error on failures
	 */
	Socket<Address, Protocol> accept(Address &address, Condition &cond)
	{
		assert(m_handle != Invalid);

		sockaddr_storage storage;
		socklen_t length = sizeof (storage);

		cond = Condition::None;

		Socket<Address, Protocol> client = m_proto.accept(*this, reinterpret_cast<sockaddr *>(&storage), &length, cond);

		address = Address(reinterpret_cast<sockaddr *>(&storage), length);

		return client;
	}

	/**
	 * Overloaded function.
	 *
	 * \pre isOpen()
	 * \param address the client information
	 * \return the new client or an invalid if no client is immediately available
	 * \throw net::Error on failures
	 */
	inline Socket<Address, Protocol> accept(Address &address)
	{
		Condition dummy;

		return accept(address, dummy);
	}

	/**
	 * Overlaoded function.
	 *
	 * \pre isOpen()
	 * \return the new client or an invalid if no client is immediately available
	 * \throw net::Error on failures
	 */
	inline Socket<Address, Protocol> accept()
	{
		Address da;
		Condition dc;

		return accept(da, dc);
	}

	/**
	 * Continue accept process.
	 *
	 * \pre isOpen()
	 * \param cond the condition
	 * \throw net::Error on failures
	 * \note This should be called on the returned client from accept
	 */
	inline void resumeAccept(Condition &cond)
	{
		assert(m_handle != Invalid);

		cond = Condition::None;

		m_proto.resumeAccept(*this, cond);
	}

	/**
	 * Overloaded function.
	 *
	 * \pre isOpen()
	 * \throw net::Error on failures
	 */
	inline void resumeAccept()
	{
		Condition dummy;

		resumeAccept(dummy);
	}

	/**
	 * Receive some data.
	 *
	 * \pre isOpen()
	 * \param data the destination buffer
	 * \param length the data length
	 * \param cond the condition
	 * \return the number of bytes received
	 * \throw net::Error on failures
	 */
	inline std::size_t recv(void *data, std::size_t length, Condition &cond)
	{
		assert(m_handle != Invalid);

		cond = Condition::None;

		return m_proto.recv(*this, data, length, cond);
	}

	/**
	 * Overloaded function.
	 *
	 * \pre isOpen()
	 * \param data the destination buffer
	 * \param length the data length
	 * \return the number of bytes received
	 * \throw net::Error on failures
	 */
	inline std::size_t recv(void *data, std::size_t length)
	{
		Condition dummy;

		return recv(data, length, dummy);
	}

	/**
	 * Overloaded function.
	 *
	 * \pre isOpen()
	 * \param count number of bytes desired
	 * \param cond the condition
	 * \return the result string
	 * \throw net::Error on failures
	 */
	std::string recv(std::size_t count, Condition &cond)
	{
		assert(m_handle != Invalid);

		std::string result;

		result.resize(count);
		auto n = recv(const_cast<char *>(result.data()), count, cond);
		result.resize(n);

		return result;
	}

	/**
	 * Overloaded function.
	 *
	 * \pre isOpen()
	 * \param count number of bytes desired
	 * \return the result string
	 * \throw net::Error on failures
	 */
	inline std::string recv(std::size_t count)
	{
		Condition dummy;

		return recv(count, dummy);
	}

	/**
	 * Send some data.
	 *
	 * \pre isOpen()
	 * \param data the data to send
	 * \param length the length
	 * \param cond the condition
	 * \return the number of bytes sent
	 * \throw net::Error on failures
	 */
	inline std::size_t send(const void *data, std::size_t length, Condition &cond)
	{
		assert(m_handle != Invalid);

		cond = Condition::None;

		return m_proto.send(*this, data, length, cond);
	}

	/**
	 * Overloaded function.
	 *
	 * \pre isOpen()
	 * \param data the data to send
	 * \param length the length
	 * \return the number of bytes sent
	 * \throw net::Error on failures
	 */
	inline std::size_t send(const void *data, std::size_t length)
	{
		Condition dummy;

		return send(data, length, dummy);
	}

	/**
	 * Overloaded function.
	 *
	 * \pre isOpen()
	 * \param data the data to send
	 * \param cond the condition
	 * \return the number of bytes sent
	 * \throw net::Error on failures
	 */
	inline std::size_t send(const std::string &data, Condition &cond)
	{
		return send(data.c_str(), data.length(), cond);
	}

	/**
	 * Overloaded function.
	 *
	 * \pre isOpen()
	 * \param data the data to send
	 * \return the number of bytes sent
	 * \throw net::Error on failures
	 */
	inline std::size_t send(const std::string &data)
	{
		Condition dummy;

		return send(data.c_str(), data.length(), dummy);
	}

	/**
	 * Send some data to the given client.
	 *
	 * \pre isOpen()
	 * \param data the data
	 * \param length the length
	 * \param address the client address
	 * \param addrlen the client address length
	 * \param cond the condition
	 * \return the number of bytes sent
	 * \throw net::Error on failures
	 */
	inline std::size_t sendto(const void *data, std::size_t length, const sockaddr *address, socklen_t addrlen, Condition &cond)
	{
		assert(m_handle != Invalid);

		cond = Condition::None;

		return m_proto.sendto(*this, data, length, address, addrlen, cond);
	}

	/**
	 * Overloaded function.
	 *
	 * \pre isOpen()
	 * \param data the data
	 * \param length the length
	 * \param address the client address
	 * \param addrlen the client address length
	 * \return the number of bytes sent
	 * \throw net::Error on failures
	 */
	inline std::size_t sendto(const void *data, std::size_t length, const sockaddr *address, socklen_t addrlen)
	{
		Condition dummy;

		return send(data, length, address, addrlen, dummy);
	}

	/**
	 * Overloaded function.
	 *
	 * \pre isOpen()
	 * \param data the data
	 * \param length the length
	 * \param address the client address
	 * \param cond the condition
	 * \return the number of bytes sent
	 * \throw net::Error on failures
	 */
	inline std::size_t sendto(const void *data, std::size_t length, const Address &address, Condition &cond)
	{
		return sendto(data, length, address.address(), address.length(), cond);
	}

	/**
	 * Overloaded function.
	 *
	 * \pre isOpen()
	 * \param data the data
	 * \param length the length
	 * \param address the client address
	 * \return the number of bytes sent
	 * \throw net::Error on failures
	 */
	inline std::size_t sendto(const void *data, std::size_t length, const Address &address)
	{
		Condition dummy;

		return sendto(data, length, address.address(), address.length(), dummy);
	}

	/**
	 * Overloaded function.
	 *
	 * \pre isOpen()
	 * \param data the data
	 * \param cond the condition
	 * \param address the client address
	 * \return the number of bytes sent
	 * \throw net::Error on failures
	 */
	inline std::size_t sendto(const std::string &data, const Address &address, Condition &cond)
	{
		return sendto(data.c_str(), data.length(), address.address(), address.length(), cond);
	}

	/**
	 * Overloaded function.
	 *
	 * \pre isOpen()
	 * \param data the data
	 * \param address the client address
	 * \return the number of bytes sent
	 * \throw net::Error on failures
	 */
	inline std::size_t sendto(const std::string &data, const Address &address)
	{
		Condition dummy;

		return sendto(data.c_str(), data.length(), address.address(), address.length(), dummy);
	}

	/**
	 * Receive some data from a client.
	 *
	 * \pre isOpen()
	 * \param data the destination buffer
	 * \param length the buffer length
	 * \param address the client information
	 * \param addrlen the client address initial length
	 * \param cond the condition
	 * \return the number of bytes received
	 * \throw net::Error on failures
	 */
	inline std::size_t recvfrom(void *data, std::size_t length, sockaddr *address, socklen_t *addrlen, Condition &cond)
	{
		assert(m_handle != Invalid);

		cond = Condition::None;

		return m_proto.recvfrom(*this, data, length, address, addrlen, cond);
	}

	/**
	 * Overloaded function.
	 *
	 * \pre isOpen()
	 * \param data the destination buffer
	 * \param length the buffer length
	 * \param address the client information
	 * \param addrlen the client address initial length
	 * \return the number of bytes received
	 * \throw net::Error on failures
	 */
	inline std::size_t recvfrom(void *data, std::size_t length, sockaddr *address, socklen_t *addrlen)
	{
		Condition dummy;

		return recvfrom(data, length, address, addrlen, dummy);
	}

	/**
	 * Overloaded function.
	 *
	 * \pre isOpen()
	 * \param data the destination buffer
	 * \param length the buffer length
	 * \param address the client information
	 * \param cond the condition
	 * \return the number of bytes received
	 * \throw net::Error on failures
	 */
	std::size_t recvfrom(void *data, std::size_t length, Address &address, Condition &cond)
	{
		sockaddr_storage storage;
		socklen_t addrlen = sizeof (sockaddr_storage);

		auto n = recvfrom(data, length, reinterpret_cast<sockaddr *>(&storage), &addrlen, cond);

		if (n != 0 && cond == Condition::None)
			address = Address(reinterpret_cast<sockaddr *>(&storage), addrlen);

		return n;
	}

	/**
	 * Overloaded function.
	 *
	 * \pre isOpen()
	 * \param data the destination buffer
	 * \param length the buffer length
	 * \param address the client information
	 * \return the number of bytes received
	 * \throw net::Error on failures
	 */
	inline std::size_t recvfrom(void *data, std::size_t length, Address &address)
	{
		Condition dummy;

		return recvfrom(data, length, address, dummy);
	}

	/**
	 * Overloaded function.
	 *
	 * \pre isOpen()
	 * \param data the destination buffer
	 * \param length the buffer length
	 * \return the number of bytes received
	 * \throw net::Error on failures
	 */
	inline std::size_t recvfrom(void *data, std::size_t length)
	{
		Address da;
		Condition dc;

		return recvfrom(data, length, da, dc);
	}

	/**
	 * Overloaded function.
	 *
	 * \pre isOpen()
	 * \param count the number of bytes desired
	 * \param address the client information
	 * \param cond the condition
	 * \return the result string
	 * \throw net::Error on failures
	 */
	std::string recvfrom(std::size_t count, Address &address, Condition &cond)
	{
		std::string result;

		result.resize(count);
		auto n = recvfrom(const_cast<char *>(result.data()), count, address, cond);
		result.resize(n);

		return result;
	}

	/**
	 * Overloaded function.
	 *
	 * \pre isOpen()
	 * \param count the number of bytes desired
	 * \param address the client information
	 * \return the result string
	 * \throw net::Error on failures
	 */
	inline std::string recvfrom(std::size_t count, Address &address)
	{
		Condition dummy;

		return recvfrom(count, address, dummy);
	}

	/**
	 * Overloaded function.
	 *
	 * \pre isOpen()
	 * \param count the number of bytes desired
	 * \return the result string
	 * \throw net::Error on failures
	 */
	inline std::string recvfrom(std::size_t count)
	{
		Address da;
		Condition dc;

		return recvfrom(count, da, dc);
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
		m_proto = std::move(other.m_proto);

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
template <typename Address, typename Protocol>
inline bool operator==(const Socket<Address, Protocol> &s1, const Socket<Address, Protocol> &s2)
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
template <typename Address, typename Protocol>
inline bool operator!=(const Socket<Address, Protocol> &s1, const Socket<Address, Protocol> &s2)
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
template <typename Address, typename Protocol>
inline bool operator<(const Socket<Address, Protocol> &s1, const Socket<Address, Protocol> &s2)
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
template <typename Address, typename Protocol>
inline bool operator>(const Socket<Address, Protocol> &s1, const Socket<Address, Protocol> &s2)
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
template <typename Address, typename Protocol>
inline bool operator<=(const Socket<Address, Protocol> &s1, const Socket<Address, Protocol> &s2)
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
template <typename Address, typename Protocol>
inline bool operator>=(const Socket<Address, Protocol> &s1, const Socket<Address, Protocol> &s2)
{
	return s1.handle() >= s2.handle();
}

/**
 * \brief Predefined protocols.
 */
namespace protocol {

/**
 * \brief Clear TCP implementation.
 * \ingroup net-module-tcp
 *
 * This is the basic TCP protocol that implements recv, send, connect and accept as wrappers of the usual
 * C functions.
 */
class Tcp {
public:
	/**
	 * Socket type.
	 *
	 * \return SOCK_STREAM
	 */
	inline int type() const noexcept
	{
		return SOCK_STREAM;
	}

	/**
	 * Do nothing.
	 *
	 * This function is just present for compatibility, it should never be called.
	 */
	template <typename Address>
	inline void create(Socket<Address, Tcp> &) const noexcept
	{
	}

	/**
	 * Initiate connection.
	 *
	 * \param sc the socket
	 * \param address the address
	 * \param length the address length
	 * \param cond the condition
	 */
	template <typename Address, typename Protocol>
	void connect(Socket<Address, Protocol> &sc, const sockaddr *address, socklen_t length, Condition &cond)
	{
		if (::connect(sc.handle(), address, length) == Failure) {
			/*
			 * Determine if the error comes from a non-blocking connect that cannot be
			 * accomplished yet.
			 */
#if defined(_WIN32)
			int error = WSAGetLastError();

			if (error == WSAEWOULDBLOCK)
				cond = Condition::Writable;
			else
				throw Error(Error::System, "connect", error);
#else
			if (errno == EINPROGRESS)
				cond = Condition::Writable;
			else
				throw Error(Error::System, "connect");
#endif
		}
	}

	/**
	 * Resume the connection.
	 *
	 * Just check for SOL_SOCKET/SO_ERROR.
	 *
	 * User is responsible to wait before the socket is writable, otherwise behavior is undefined.
	 *
	 * \param sc the socket
	 * \param cond the condition
	 */
	template <typename Address, typename Protocol>
	void resumeConnect(Socket<Address, Protocol> &sc, Condition &cond)
	{
		int error = sc.template get<int>(SOL_SOCKET, SO_ERROR);

#if defined(_WIN32)
		if (error == WSAEWOULDBLOCK)
			cond = Condition::Writable;
		else if (error != 0)
			throw Error(Error::System, "connect", error);
#else
		if (error == EINPROGRESS)
			cond = Condition::Writable;
		else if (error != 0)
			throw Error(Error::System, "connect", error);
#endif
	}

	/**
	 * Accept a new client.
	 *
	 * If there are no pending connection, an invalid socket is returned, condition is left to Condition::None.
	 *
	 * \param sc the socket
	 * \param address the address
	 * \param length the length
	 * \return the new socket
	 */
	template <typename Address, typename Protocol>
	Socket<Address, Protocol> accept(Socket<Address, Protocol> &sc, sockaddr *address, socklen_t *length, Condition &)
	{
		Handle handle = ::accept(sc.handle(), address, length);

		if (handle == Invalid)
			return Socket<Address, Protocol>();

		return Socket<Address, Protocol>(handle);
	}

	/**
	 * Resume accept process.
	 *
	 * No-op for TCP.
	 */
	template <typename Address, typename Protocol>
	inline void resumeAcept(Socket<Address, Protocol> &, Condition &) const noexcept
	{
	}

	/**
	 * Receive some data.
	 *
	 * \param sc the socket
	 * \param data the destination buffer
	 * \param length the buffer length
	 * \param cond the condition
	 * \return the number of byte received
	 */
	template <typename Address>
	std::size_t recv(Socket<Address, Tcp> &sc, void *data, std::size_t length, Condition &cond)
	{
		int max = length > INT_MAX ? INT_MAX : static_cast<int>(length);
		int nbread = ::recv(sc.handle(), (Arg)data, max, 0);

		if (nbread == Failure) {
#if defined(_WIN32)
			int error = WSAGetLastError();

			if (error == WSAEWOULDBLOCK) {
				nbread = 0;
				cond = Condition::Readable;
			} else
				throw Error(Error::System, "recv", error);
#else
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				nbread = 0;
				cond = Condition::Readable;
			} else
				throw Error(Error::System, "recv");
#endif
		}

		return static_cast<std::size_t>(nbread);
	}

	/**
	 * Send some data.
	 *
	 * \param sc the socket
	 * \param data the data to send
	 * \param length the length
	 * \param cond the condition
	 * \return the number of bytes sent
	 */
	template <typename Address>
	std::size_t send(Socket<Address, Tcp> &sc, const void *data, std::size_t length, Condition &cond)
	{
		int max = length > INT_MAX ? INT_MAX : static_cast<int>(length);
		int nbsent = ::send(sc.handle(), (ConstArg)data, max, 0);

		if (nbsent == Failure) {
#if defined(_WIN32)
			int error = WSAGetLastError();

			if (error == WSAEWOULDBLOCK) {
				nbsent = 0;
				cond = Condition::Writable;
			} else
				throw Error(Error::System, "send", error);
#else
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				nbsent = 0;
				cond = Condition::Writable;
			} else
				throw Error(Error::System, "send");
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
class Udp {
public:
	/**
	 * Socket type.
	 *
	 * \return SOCK_DGRAM
	 */
	inline int type() const noexcept
	{
		return SOCK_DGRAM;
	}

	/**
	 * Do nothing.
	 */
	template <typename Address, typename Protocol>
	inline void create(Socket<Address, Protocol> &) noexcept
	{
	}

	/**
	 * Receive some data.
	 *
	 * \param sc the socket
	 * \param data the data
	 * \param length the length
	 * \param address the source address
	 * \param addrlen the source address in/out length
	 * \param cond the condition
	 * \return the number of bytes received
	 */
	template <typename Address, typename Protocol>
	std::size_t recvfrom(Socket<Address, Protocol> &sc, void *data, std::size_t length, sockaddr *address, socklen_t *addrlen, Condition &cond)
	{
		int max = length > INT_MAX ? INT_MAX : static_cast<int>(length);
		int nbread;

		nbread = ::recvfrom(sc.handle(), (Arg)data, max, 0, address, addrlen);

		if (nbread == Failure) {
#if defined(_WIN32)
			int error = WSAGetLastError();

			if (error == WSAEWOULDBLOCK) {
				nbread = 0;
				cond = Condition::Writable;
			} else
				throw Error(Error::System, "recvfrom");
#else
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				nbread = 0;
				cond = Condition::Writable;
			} else
				throw Error(Error::System, "recvfrom");
#endif
		}

		return static_cast<unsigned>(nbread);
	}

	/**
	 * Send some data.
	 *
	 * \param sc the socket
	 * \param data the data to send
	 * \param length the data length
	 * \param address the destination address
	 * \param addrlen the destination address length
	 * \param cond the condition
	 * \return the number of bytes sent
	 */
	template <typename Address, typename Protocol>
	std::size_t sendto(Socket<Address, Protocol> &sc, const void *data, std::size_t length, const sockaddr *address, socklen_t addrlen, Condition &cond)
	{
		int max = length > INT_MAX ? INT_MAX : static_cast<int>(length);
		int nbsent;

		nbsent = ::sendto(sc.handle(), (ConstArg)data, max, 0, address, addrlen);
		if (nbsent == Failure) {
#if defined(_WIN32)
			int error = WSAGetLastError();

			if (error == WSAEWOULDBLOCK) {
				nbsent = 0;
				cond = Condition::Writable;
			} else
				throw Error(Error::System, "sendto", error);
#else
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				nbsent = 0;
				cond = Condition::Writable;
			} else
				throw Error(Error::System, "sendto");
#endif
		}

		return static_cast<unsigned>(nbsent);
	}
};

#if !defined(NET_NO_SSL)

/**
 * \brief Experimental TLS support.
 * \ingroup net-module-tls
 * \warning This class is highly experimental.
 */
class Tls : private Tcp {
private:
	using Context = std::shared_ptr<SSL_CTX>;
	using Ssl = std::unique_ptr<SSL, void (*)(SSL *)>;

	// OpenSSL objects.
	Context m_context;
	Ssl m_ssl{nullptr, nullptr};

	// Status.
	bool m_tcpconnected{false};

	/*
	 * User definable parameters.
	 */
	ssl::Method m_method{ssl::Tlsv1};
	std::string m_key;
	std::string m_certificate;
	bool m_verify{false};

	// Construct with a context and ssl, for Tls::accept.
	Tls(Context context, Ssl ssl)
		: m_context(std::move(context))
		, m_ssl(std::move(ssl))
	{
	}

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
	void wrap(const std::string &func, Condition &cond, Function &&function)
	{
		auto ret = function();

		if (ret <= 0) {
			int no = SSL_get_error(m_ssl.get(), ret);

			switch (no) {
			case SSL_ERROR_WANT_READ:
				cond = Condition::Readable;
				break;
			case SSL_ERROR_WANT_WRITE:
				cond = Condition::Writable;
				break;
			default:
				throw Error(Error::System, func, error());
			}
		}
	}

	template <typename Address, typename Protocol>
	void doConnect(Socket<Address, Protocol> &, Condition &cond)
	{
		wrap("connect", cond, [&] () -> int {
			return SSL_connect(m_ssl.get());
		});
	}

	template <typename Address, typename Protocol>
	void doAccept(Socket<Address, Protocol> &, Condition &cond)
	{
		wrap("accept", cond, [&] () -> int {
			return SSL_accept(m_ssl.get());
		});
	}

public:
	/**
	 * \copydoc Tcp::type
	 */
	inline int type() const noexcept
	{
		return SOCK_STREAM;
	}

	/**
	 * Empty TLS constructor.
	 */
	inline Tls()
	{
#if !defined(NET_NO_SSL_AUTO_INIT)
		ssl::init();
#endif
	}

	/**
	 * Set the method.
	 *
	 * \param method the method
	 * \pre the socket must not be already created
	 */
	inline void setMethod(ssl::Method method) noexcept
	{
		assert(!m_context);
		assert(!m_ssl);

		m_method = method;
	}

	/**
	 * Use the specified private key file.
	 *
	 * \param file the path to the private key
	 */
	inline void setPrivateKey(std::string file) noexcept
	{
		m_key = std::move(file);
	}

	/**
	 * Use the specified certificate file.
	 *
	 * \param file the path to the file
	 */
	inline void setCertificate(std::string file) noexcept
	{
		m_certificate = std::move(file);
	}

	/**
	 * Set to true if we must verify the certificate and private key.
	 *
	 * \param verify the mode
	 */
	inline void setVerify(bool verify = true) noexcept
	{
		m_verify = verify;
	}

	/**
	 * Initialize the SSL objects after have created.
	 *
	 * \param sc the socket
	 * \throw net::Error on errors
	 */
	template <typename Address>
	void create(Socket<Address, Tls> &sc)
	{
		auto method = (m_method == ssl::Tlsv1) ? TLSv1_method() : SSLv23_method();

		m_context = Context(SSL_CTX_new(method), SSL_CTX_free);
		m_ssl = Ssl(SSL_new(m_context.get()), SSL_free);

		SSL_set_fd(m_ssl.get(), static_cast<int>(sc.handle()));

		/*
		 * Load certificates, the wrap function requires a condition so just add a dummy value.
		 */
		Condition dummy;

		if (m_certificate.size() > 0)
			wrap("SSL_CTX_use_certificate_file", dummy, [&] () -> int {
				return SSL_CTX_use_certificate_file(m_context.get(), m_certificate.c_str(), SSL_FILETYPE_PEM);
			});
		if (m_key.size() > 0)
			wrap("SSL_CTX_use_PrivateKey_file", dummy, [&] () -> int {
				return SSL_CTX_use_PrivateKey_file(m_context.get(), m_key.c_str(), SSL_FILETYPE_PEM);
			});
		if (m_verify && !SSL_CTX_check_private_key(m_context.get()))
			throw Error(Error::System, "(openssl)", "unable to verify key");
	}

	/**
	 * Initiate connection.
	 *
	 * \param sc the socket
	 * \param address the address
	 * \param length the address length
	 * \param cond the condition
	 */
	template <typename Address, typename Protocol>
	void connect(Socket<Address, Protocol> &sc, const sockaddr *address, socklen_t length, Condition &cond)
	{
		// 1. Connect using raw TCP.
		Tcp::connect(sc, address, length, cond);

		// 2. If the connection is complete (e.g. non-blocking), try handshake.
		if (cond == Condition::None) {
			m_tcpconnected = true;
			doConnect(sc, cond);
		}
	}

	/**
	 * Resume the connection.
	 *
	 * \param sc the socket
	 * \param cond the condition to wait
	 */
	template <typename Address, typename Protocol>
	void connect(Socket<Address, Protocol> &sc, Condition &cond)
	{
		// 1. Be sure to complete standard connect before.
		if (!m_tcpconnected) {
			Tcp::connect(sc, cond);
			m_tcpconnected = (cond == Condition::None);
		}

		// 2. Do SSL connect.
		if (m_tcpconnected)
			doConnect(sc, cond);
	}

	/**
	 * Accept a new client.
	 *
	 * If there are no pending connection, an invalid socket is returned, condition is left to Condition::None.
	 *
	 * \param sc the socket
	 * \param address the address
	 * \param length the length
	 * \param cond the condition to wait
	 * \return the new socket
	 */
	template <typename Address>
	Socket<Address, Tls> accept(Socket<Address, Tls> &sc, sockaddr *address, socklen_t *length, Condition &cond)
	{
		// 1. TCP returns empty client if no pending connection is available.
		auto client = Tcp::accept(sc, address, length, cond);

		// 2. If a client is available, try initial accept.
		if (client.isOpen()) {
			Tls &proto = client.protocol();

			// 2.1. Share the context.
			proto.m_context = m_context;

			// 2.2. Create new SSL instance.
			proto.m_ssl = Ssl(SSL_new(m_context.get()), SSL_free);

			SSL_set_fd(proto.m_ssl.get(), static_cast<int>(client.handle()));

			// 2.3. Try accept process on the **new** client.
			proto.doAccept(client, cond);
		}

		return client;
	}

	/**
	 * Resume accept process.
	 *
	 * \param sc the socket
	 * \param cond the condition to wait
	 * \throw net::Error on failures
	 */
	template <typename Address, typename Protocol>
	inline void accept(Socket<Address, Protocol> &sc, Condition &cond)
	{
		doAccept(sc, cond);
	}

	/**
	 * Receive some data.
	 *
	 * \param data the destination buffer
	 * \param length the buffer length
	 * \param cond the condition
	 * \return the number of bytes received
	 */
	template <typename Address>
	std::size_t recv(Socket<Address, Tls> &, void *data, std::size_t length, Condition &cond)
	{
		int max = length > INT_MAX ? INT_MAX : static_cast<int>(length);
		int nbread = 0;

		wrap("recv", cond, [&] () -> int {
			return (nbread = SSL_read(m_ssl.get(), data, max));
		});

		return static_cast<std::size_t>(nbread < 0 ? 0 : nbread);
	}

	/**
	 * Send some data.
	 *
	 * \param data the data to send
	 * \param length the length
	 * \param cond the condition
	 * \return the number of bytes sent
	 */
	template <typename Address>
	std::size_t send(Socket<Address, Tls> &, const void *data, std::size_t length, Condition &cond)
	{
		int max = length > INT_MAX ? INT_MAX : static_cast<int>(length);
		int nbsent = 0;

		wrap("send", cond, [&] () -> int {
			return (nbsent = SSL_write(m_ssl.get(), data, max));
		});

		return static_cast<std::size_t>(nbsent < 0 ? 0 : nbsent);
	}
};

#endif // !NET_NO_SSL

} // !protocol

/**
 * \brief Predefined addresses.
 */
namespace address {

/**
 * \brief Generic address.
 * \ingroup net-module-addresses
 *
 * This address can store anything that fits into a sockaddr_storage.
 */
class GenericAddress {
private:
	sockaddr_storage m_address;
	socklen_t m_length{0};

public:
	/**
	 * Construct a null address.
	 */
	inline GenericAddress() noexcept
	{
		std::memset(&m_address, 0, sizeof (sockaddr_storage));
	}

	/**
	 * Construct an address.
	 *
	 * \pre address is not null
	 * \pre length <= sizeof (sockaddr_storage)
	 * \param address the address to copy
	 * \param length the address length
	 */
	inline GenericAddress(const sockaddr *address, socklen_t length) noexcept
		: m_length(length)
	{
		assert(address);
		assert(length <= sizeof (sockaddr_storage));

		std::memset(&m_address, 0, sizeof (sockaddr_storage));
		std::memcpy(&m_address, address, length);
	}

	/**
	 * Get the address family.
	 *
	 * \return the address family
	 */
	inline int domain() const noexcept
	{
		return m_address.ss_family;
	}

	/**
	 * Get the underlying address.
	 *
	 * \return the address
	 */
	inline sockaddr *address() noexcept
	{
		return reinterpret_cast<sockaddr *>(&m_address);
	}

	/**
	 * Overloaded function.
	 *
	 * \return the address
	 */
	inline const sockaddr *address() const noexcept
	{
		return reinterpret_cast<const sockaddr *>(&m_address);
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
};

/**
 * Compare two generic addresses.
 *
 * \param a1 the first address
 * \param a2 the second address
 * \return true if they equal
 */
inline bool operator==(const GenericAddress &a1, const GenericAddress &a2) noexcept
{
	return a1.length() == a2.length() && std::memcmp(a1.address(), a2.address(), a1.length()) == 0;
}

/**
 * Compare two generic addresses.
 *
 * \param a1 the first address
 * \param a2 the second address
 * \return false if they equal
 */
inline bool operator!=(const GenericAddress &a1, const GenericAddress &a2) noexcept
{
	return !(a1 == a2);
}

/**
 * \brief Generic IP address.
 * \ingroup net-module-addresses
 *
 * You can use this address instead of Ipv4 or Ipv6 if you don't know which address to use at runtime. However,
 * when creating your socket, you will need to define the correct domain.
 */
class Ip {
private:
	union {
		sockaddr_in6 m_sin6;
		sockaddr_in m_sin;
	};

	int m_domain;

public:
	/**
	 * Create IP address, defaults to IPv4.
	 *
	 * \pre domain must be AF_INET or AF_INET6
	 * \param domain the domain
	 */
	inline Ip(int domain = AF_INET) noexcept
		: m_domain(domain)
	{
		assert(domain == AF_INET || domain == AF_INET6);

		std::memset(&m_sin, 0, sizeof (sockaddr_in));
	}

	/**
	 * Create an IP address on the specific ip address.
	 *
	 * \pre domain must be AF_INET or AF_INET6
	 * \param ip the address or "*" for any
	 * \param port the port
	 * \param domain the domain
	 * \warning If NET_HAVE_INET_PTON is undefined, host can not be other than "*"
	 * \throw net::Error on failures or if inet_pton is unavailable
	 */
	inline Ip(const std::string &ip, std::uint16_t port, int domain)
		: Ip(domain)
	{
		if (m_domain == AF_INET)
			make(ip, port, m_sin);
		else
			make(ip, port, m_sin6);
	}

	/**
	 * Create the IP address from the storage.
	 *
	 * \pre the storage domain must be AF_INET or AF_INET6
	 * \param ss the the storage
	 * \param length the storage length
	 */
	inline Ip(const sockaddr *ss, socklen_t length) noexcept
		: Ip(ss->sa_family)
	{
		assert(ss->sa_family == AF_INET || ss->sa_family == AF_INET6);

		if (ss->sa_family == AF_INET)
			std::memcpy(&m_sin, ss, length);
		else
			std::memcpy(&m_sin6, ss, length);
	}

	/**
	 * Get the domain.
	 *
	 * \return AF_INET or AF_INET6
	 */
	inline int domain() const noexcept
	{
		return m_domain;
	}

	/**
	 * Get the underlying address, may be a sockaddr_in or sockaddr_in6.
	 *
	 * \return the address
	 */
	inline const sockaddr *address() const noexcept
	{
		return m_domain == AF_INET ? reinterpret_cast<const sockaddr *>(&m_sin) : reinterpret_cast<const sockaddr *>(&m_sin6);
	}

	/**
	 * Get the address length.
	 *
	 * \return the address length
	 */
	inline socklen_t length() const noexcept
	{
		return m_domain == AF_INET ? sizeof (sockaddr_in) : sizeof (sockaddr_in6);
	}

	/**
	 * Retrieve the port.
	 *
	 * \return the port
	 */
	inline std::uint16_t port() const noexcept
	{
		return m_domain == AF_INET ? ntohs(m_sin.sin_port) : ntohs(m_sin6.sin6_port);
	}

	/**
	 * Get the ip address.
	 *
	 * \return the ip address
	 * \throw net::Error on errors or if inet_ntop is unavailable
	 */
	inline std::string ip() const
	{
		return m_domain == AF_INET ? ip(m_sin) : ip(m_sin6);
	}

	/**
	 * Prepare the sockaddr_in structure with the given ip.
	 *
	 * \param ip the ip address
	 * \param port the port
	 * \param sin the Ipv4 address
	 * \throw net::Error if inet_pton is unavailable
	 */
	static void make(const std::string &ip, std::uint16_t port, sockaddr_in &sin)
	{
#if !defined(NET_NO_AUTO_INIT)
		net::init();
#endif

		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);

		if (ip == "*")
			sin.sin_addr.s_addr = INADDR_ANY;
#if defined(NET_HAVE_INET_PTON)
		else if (inet_pton(AF_INET, ip.c_str(), &sin.sin_addr) <= 0)
			throw Error(Error::System, "inet_pton");
#else
		else
			throw Error(Error::System, "inet_pton", std::strerror(ENOSYS));
#endif
	}

	/**
	 * Prepare the sockaddr_in structure with the given ip.
	 *
	 * \param ip the ip address
	 * \param port the port
	 * \param sin6 the Ipv6 address
	 * \throw net::Error if inet_pton is unavailable
	 */
	static void make(const std::string &ip, std::uint16_t port, sockaddr_in6 &sin6)
	{
#if !defined(NET_NO_AUTO_INIT)
		net::init();
#endif

		sin6.sin6_family = AF_INET6;
		sin6.sin6_port = htons(port);

		if (ip == "*")
			sin6.sin6_addr = in6addr_any;
#if defined(NET_HAVE_INET_PTON)
		else if (inet_pton(AF_INET6, ip.c_str(), &sin6.sin6_addr) <= 0)
			throw Error(Error::System, "inet_pton");
#else
		else
			throw Error(Error::System, "inet_pton", std::strerror(ENOSYS));
#endif
	}

	/**
	 * Get the underlying ip from the given address.
	 *
	 * \param sin the Ipv4 address
	 * \return the ip address
	 * \throw net::Error if inet_ntop is unavailable
	 */
	static std::string ip(const sockaddr_in &sin)
	{
#if !defined(NET_NO_AUTO_INIT)
		net::init();
#endif

#if !defined(NET_HAVE_INET_NTOP)
		(void)sin;

		throw Error(Error::System, "inet_ntop", std::strerror(ENOSYS));
#else
		char result[INET_ADDRSTRLEN + 1];

		std::memset(result, 0, sizeof (result));

		if (!inet_ntop(AF_INET, const_cast<in_addr *>(&sin.sin_addr), result, sizeof (result)))
			throw Error(Error::System, "inet_ntop");

		return result;
#endif
	}

	/**
	 * Get the underlying ip from the given address.
	 *
	 * \param sin6 the Ipv6 address
	 * \return the ip address
	 * \throw net::Error if inet_ntop is unavailable
	 */
	static std::string ip(const sockaddr_in6 &sin6)
	{
#if !defined(NET_NO_AUTO_INIT)
		net::init();
#endif

#if !defined(NET_HAVE_INET_NTOP)
		(void)sin6;

		throw Error(Error::System, "inet_ntop", std::strerror(ENOSYS));
#else
		char result[INET6_ADDRSTRLEN];

		std::memset(result, 0, sizeof (result));

		if (!inet_ntop(AF_INET6, const_cast<in6_addr *>(&sin6.sin6_addr), result, sizeof (result)))
			throw Error(Error::System, "inet_ntop");

		return result;
#endif
	}

	/**
	 * Resolve an hostname.
	 *
	 * This function wraps getaddrinfo and returns the first result.
	 *
	 * \param host the hostname
	 * \param service the service name (port or name)
	 * \param domain the domain (e.g. AF_INET)
	 * \param type the socket type (e.g. SOCK_STREAM)
	 * \return the resolved address
	 * \throw net::Error on failures
	 */
	static Ip resolve(const std::string &host, const std::string &service, int domain = AF_INET, int type = SOCK_STREAM)
	{
		assert(domain == AF_INET || domain == AF_INET6);
#if !defined(NET_NO_AUTO_INIT)
		net::init();
#endif

		struct addrinfo hints, *res;

		std::memset(&hints, 0, sizeof (struct addrinfo));
		hints.ai_family = domain;
		hints.ai_socktype = type;

		int e = getaddrinfo(host.c_str(), service.c_str(), &hints, &res);

		if (e != 0)
			throw Error(Error::System, "getaddrinfo", gai_strerror(e));

		Ip ip(res->ai_addr, res->ai_addrlen);

		freeaddrinfo(res);

		return ip;
	}
};

/**
 * \brief Ipv4 only address.
 * \ingroup net-module-addresses
 */
class Ipv4 {
private:
	sockaddr_in m_sin;

public:
	/**
	 * Create an Ipv4 address.
	 */
	inline Ipv4() noexcept
	{
		std::memset(&m_sin, 0, sizeof (sockaddr_in));
	}

	/**
	 * Create an Ipv4 address on the specific ip address.
	 *
	 * \param ip the address or "*" for any
	 * \param port the port
	 * \warning If NET_HAVE_INET_PTON is undefined, host can not be other than "*"
	 * \throw net::Error on failures or if inet_pton is unavailable
	 */
	inline Ipv4(const std::string &ip, std::uint16_t port)
		: Ipv4()
	{
		Ip::make(ip, port, m_sin);
	}

	/**
	 * Create the IP address from the storage.
	 *
	 * \pre the storage domain must be AF_INET
	 * \param ss the the storage
	 * \param length the storage length
	 */
	inline Ipv4(const sockaddr *ss, socklen_t length) noexcept
	{
		assert(ss->sa_family == AF_INET);

		std::memcpy(&m_sin, ss, length);
	}

	/**
	 * Get the domain.
	 *
	 * \return AF_INET
	 */
	inline int domain() const noexcept
	{
		return AF_INET;
	}

	/**
	 * Get the underlying address.
	 *
	 * \return the address
	 */
	inline const sockaddr *address() const noexcept
	{
		return reinterpret_cast<const sockaddr *>(&m_sin);
	}

	/**
	 * Get the address length.
	 *
	 * \return the size of sockaddr_in
	 */
	inline socklen_t length() const noexcept
	{
		return sizeof (sockaddr_in);
	}

	/**
	 * Get the port.
	 *
	 * \return the port
	 */
	inline std::uint16_t port() const noexcept
	{
		return ntohs(m_sin.sin_port);
	}

	/**
	 * Get the ip address.
	 *
	 * \return the ip address
	 * \throw net::Error on errors or if inet_ntop is unavailable
	 */
	inline std::string ip() const
	{
		return Ip::ip(m_sin);
	}

	/**
	 * Same as Ip::resolve with AF_INET as domain.
	 *
	 * \param host the hostname
	 * \param service the service name (port or name)
	 * \param type the socket type (e.g. SOCK_STREAM)
	 * \return the resolved address
	 * \throw net::Error on failures
	 */
	static Ipv4 resolve(const std::string &host, const std::string &service, int type = SOCK_STREAM)
	{
		Ip result = Ip::resolve(host, service, AF_INET, type);

		return Ipv4(result.address(), result.length());
	}
};

/**
 * \brief Ipv4 only address.
 * \ingroup net-module-addresses
 */
class Ipv6 {
private:
	sockaddr_in6 m_sin6;

public:
	/**
	 * Create an Ipv6 address.
	 */
	inline Ipv6() noexcept
	{
		std::memset(&m_sin6, 0, sizeof (sockaddr_in6));
	}

	/**
	 * Create an Ipv6 address on the specific ip address.
	 *
	 * \param ip the address or "*" for any
	 * \param port the port
	 * \warning If NET_HAVE_INET_PTON is undefined, host can not be other than "*"
	 * \throw net::Error on failures or if inet_pton is unavailable
	 */
	inline Ipv6(const std::string &ip, std::uint16_t port)
		: Ipv6()
	{
		Ip::make(ip, port, m_sin6);
	}

	/**
	 * Create the IP address from the storage.
	 *
	 * \pre the storage domain must be AF_INET6
	 * \param ss the the storage
	 * \param length the storage length
	 */
	inline Ipv6(const sockaddr *ss, socklen_t length) noexcept
	{
		assert(ss->sa_family == AF_INET6);

		std::memcpy(&m_sin6, ss, length);
	}

	/**
	 * Get the domain.
	 *
	 * \return AF_INET6
	 */
	inline int domain() const noexcept
	{
		return AF_INET6;
	}

	/**
	 * Get the underlying address.
	 *
	 * \return the address
	 */
	inline const sockaddr *address() const noexcept
	{
		return reinterpret_cast<const sockaddr *>(&m_sin6);
	}

	/**
	 * Get the address length.
	 *
	 * \return the size of sockaddr_in
	 */
	inline socklen_t length() const noexcept
	{
		return sizeof (sockaddr_in6);
	}

	/**
	 * Get the port.
	 *
	 * \return the port
	 */
	inline std::uint16_t port() const noexcept
	{
		return ntohs(m_sin6.sin6_port);
	}

	/**
	 * Get the ip address.
	 *
	 * \return the ip address
	 * \throw net::Error on errors or if inet_ntop is unavailable
	 */
	inline std::string ip() const
	{
		return Ip::ip(m_sin6);
	}

	/**
	 * Same as Ip::resolve with AF_INET6 as domain.
	 *
	 * \param host the hostname
	 * \param service the service name (port or name)
	 * \param type the socket type (e.g. SOCK_STREAM)
	 * \return the resolved address
	 * \throw net::Error on failures
	 */
	static Ipv6 resolve(const std::string &host, const std::string &service, int type = SOCK_STREAM)
	{
		Ip result = Ip::resolve(host, service, AF_INET6, type);

		return Ipv6(result.address(), result.length());
	}
};

#if !defined(_WIN32)

/**
 * \brief unix family sockets
 * \ingroup net-module-addresses
 *
 * Create an address to a specific path. Only available on Unix.
 */
class Local {
private:
	sockaddr_un m_sun;
	std::string m_path;

public:
	/**
	 * Get the domain AF_LOCAL.
	 *
	 * \return AF_LOCAL
	 */
	inline int domain() const noexcept
	{
		return AF_LOCAL;
	}

	/**
	 * Default constructor.
	 */
	inline Local() noexcept
	{
		std::memset(&m_sun, 0, sizeof (sockaddr_un));
	}

	/**
	 * Construct an address to a path.
	 *
	 * \param path the path
	 * \param rm remove the file before (default: false)
	 */
	Local(std::string path, bool rm = false) noexcept
		: m_path(std::move(path))
	{
		// Silently remove the file even if it fails.
		if (rm)
			::remove(m_path.c_str());

		// Copy the path.
		std::memset(m_sun.sun_path, 0, sizeof (m_sun.sun_path));
		std::strncpy(m_sun.sun_path, m_path.c_str(), sizeof (m_sun.sun_path) - 1);

		// Set the parameters.
		m_sun.sun_family = AF_LOCAL;
	}

	/**
	 * Construct an unix address from a storage address.
	 *
	 * \pre storage's domain must be AF_LOCAL
	 * \param ss the storage
	 * \param length the length
	 */
	Local(const sockaddr *ss, socklen_t length) noexcept
	{
		assert(ss->sa_family == AF_LOCAL);

		std::memcpy(&m_sun, ss, length);
		m_path = reinterpret_cast<const sockaddr_un &>(m_sun).sun_path;
	}

	/**
	 * Get the sockaddr_un.
	 *
	 * \return the address
	 */
	inline const sockaddr *address() const noexcept
	{
		return reinterpret_cast<const sockaddr *>(&m_sun);
	}

	/**
	 * Get the address length.
	 *
	 * \return the length
	 */
	inline socklen_t length() const noexcept
	{
#if defined(NET_HAVE_SUN_LEN)
		return SUN_LEN(&m_sun);
#else
		return sizeof (m_sun);
#endif
	}
};

#endif // !_WIN32

/**
 * \brief Address iterator.
 * \ingroup net-module-addresses
 * \see resolve
 *
 * This iterator can be used to try to connect to an host.
 *
 * When you use net::resolve with unspecified domain or socket type, the function may retrieve several different addresses that you can
 * iterate over to try to connect to.
 *
 * Example:
 *
 * ````cpp
 * net::SocketTcpIp sc;
 * net::AddressIterator end, it = net::resolve("hostname.test", "80");
 *
 * while (!connected_condition && it != end)
 *   sc.connect(it->address(), it->length());
 * ````
 *
 * When an iterator equals to a default constructed iterator, it is considered not dereferenceable.
 */
class AddressIterator : public std::iterator<std::forward_iterator_tag, GenericAddress> {
private:
	std::vector<GenericAddress> m_addresses;
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
	inline AddressIterator(std::vector<GenericAddress> addresses, std::size_t index = 0) noexcept
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
	inline const GenericAddress &operator*() const noexcept
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
	inline GenericAddress &operator*() noexcept
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
	inline const GenericAddress *operator->() const noexcept
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
	inline GenericAddress *operator->() noexcept
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

} // !address

/**
 * \brief Predefined options.
 */
namespace option {

/**
 * \ingroup net-module-options
 * \brief Set or get the blocking-mode for a socket.
 * \warning On Windows, it's not possible to check if the socket is blocking or not.
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
	template <typename Address, typename Protocol>
	void set(Socket<Address, Protocol> &sc) const
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
			throw Error(Error::System, "fcntl");
#else
		unsigned long flags = (m_value) ? 0 : 1;

		if (ioctlsocket(sc.handle(), FIONBIO, &flags) == Failure)
			throw Error(Error::System, "fcntl");
#endif
	}

	/**
	 * Get the option.
	 *
	 * \param sc the socket
	 * \return the value
	 * \throw Error on errors
	 */
	template <typename Address, typename Protocol>
	bool get(Socket<Address, Protocol> &sc) const
	{
#if defined(O_NONBLOCK) && !defined(_WIN32)
		int flags = fcntl(sc.handle(), F_GETFL, 0);

		if (flags < 0)
			throw Error(Error::System, "fcntl");

		return !(flags & O_NONBLOCK);
#else
		(void)sc;

		throw Error(Error::Other, "get", std::strerror(ENOSYS));
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
	template <typename Address, typename Protocol>
	inline void set(Socket<Address, Protocol> &sc) const
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
	template <typename Address, typename Protocol>
	inline int get(Socket<Address, Protocol> &sc) const
	{
		return sc.template get<int>(SOL_SOCKET, SO_RCVBUF);
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
	template <typename Address, typename Protocol>
	inline void set(Socket<Address, Protocol> &sc) const
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
	template <typename Address, typename Protocol>
	inline bool get(Socket<Address, Protocol> &sc) const
	{
		return sc.template get<int>(SOL_SOCKET, SO_REUSEADDR) != 0;
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
	template <typename Address, typename Protocol>
	inline void set(Socket<Address, Protocol> &sc) const
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
	template <typename Address, typename Protocol>
	inline int get(Socket<Address, Protocol> &sc) const
	{
		return sc.template get<int>(SOL_SOCKET, SO_SNDBUF);
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
	template <typename Address, typename Protocol>
	inline void set(Socket<Address, Protocol> &sc) const
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
	template <typename Address, typename Protocol>
	inline bool get(Socket<Address, Protocol> &sc) const
	{
		return sc.template get<int>(IPPROTO_TCP, TCP_NODELAY) != 0;
	}
};

/**
 * \ingroup net-module-options
 * \brief Control IPPROTO_IPV6/IPV6_V6ONLY
 *
 * Note: some systems may or not set this option by default so it's a good idea to set it in any case to either
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
	template <typename Address, typename Protocol>
	inline void set(Socket<Address, Protocol> &sc) const
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
	template <typename Address, typename Protocol>
	inline bool get(Socket<Address, Protocol> &sc) const
	{
		return sc.template get<int>(IPPROTO_IPV6, IPV6_V6ONLY) != 0;
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
	Handle socket;		//!< which socket is ready
	Condition flags;	//!< the flags
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
			throw Error(Error::System, "epoll_ctl");
	}

public:
	/**
	 * Create epoll.
	 *
	 * \throw net::Error on failures
	 */
	inline Epoll()
		: m_handle(epoll_create1(0))
	{
		if (m_handle < 0)
			throw Error(Error::System, "epoll_create");
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
	 * For set and unset, we need to apply the whole flags required, so if the socket
	 * was set to Connection::Readable and user *8adds** Connection::Writable, we must
	 * place both.
	 *
	 * \param table the listener table
	 * \param h the handle
	 * \param condition the condition
	 * \param add set to true if the socket is new to the backend
	 * \throw net::Error on failures
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
	 * \throw net::Error on failures
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
	 * \throw net::Error on failures
	 */
	std::vector<ListenerStatus> wait(const ListenerTable &, int ms)
	{
		int ret = epoll_wait(m_handle, m_events.data(), m_events.size(), ms);
		std::vector<ListenerStatus> result;

		if (ret == 0)
			throw Error(Error::Timeout, "epoll_wait", std::strerror(ETIMEDOUT));
		if (ret < 0)
			throw Error(Error::System, "epoll_wait");

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
			throw Error(Error::System, "kevent");
	}

public:
	/**
	 * Create kqueue.
	 *
	 * \throw net::Error on failures
	 */
	inline Kqueue()
		: m_handle(kqueue())
	{
		if (m_handle < 0)
			throw Error(Error::System, "kqueue");
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
	 * \throw net::Error on failures
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
	 * \throw net::Error on failures
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
	 * \throw net::Error on failures
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
			throw Error(Error::Timeout, "kevent", std::strerror(ETIMEDOUT));
		if (nevents < 0)
			throw Error(Error::System, "kevent");

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
		 * Poll implementations mark the socket differently regarding the disconnection of a socket.
		 *
		 * At least, even if POLLHUP or POLLIN is set, recv() always return 0 so we mark the socket as readable.
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
	 * \throw net::Error on failures
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
	 * \throw net::Error on failures
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
	 * \throw net::Error on failures
	 */
	std::vector<ListenerStatus> wait(const ListenerTable &, int ms)
	{
#if defined(_WIN32)
		auto result = WSAPoll(m_fds.data(), (ULONG)m_fds.size(), ms);
#else
		auto result = poll(m_fds.data(), m_fds.size(), ms);
#endif

		if (result == 0)
			throw Error(Error::Timeout, "select", std::strerror(ETIMEDOUT));
		if (result < 0)
			throw Error(Error::System, "poll");

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
 * This class is the fallback of any other method, it is not preferred at all for many reasons.
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
	 * \throw net::Error on failures
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
			throw Error(Error::System, "select");
		if (error == 0)
			throw Error(Error::Timeout, "select", std::strerror(ETIMEDOUT));

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
	 * It is a shorthand for unset(sc, Condition::Readable | Condition::Writable);
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
	 * Select a socket. Waits for a specific amount of time specified as the duration.
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
 * \ingroup net-module-tcp
 * \brief Helper to create TCP sockets.
 */
template <typename Address>
using SocketTcp = Socket<Address, protocol::Tcp>;

/**
 * \ingroup net-module-tcp
 * \brief Helper to create TCP/Ipv4 or TCP/Ipv6 sockets.
 */
using SocketTcpIp = Socket<address::Ip, protocol::Tcp>;

/**
 * \ingroup net-module-tcp
 * \brief Helper to create TCP/Ipv4 sockets.
 */
using SocketTcpIpv4 = Socket<address::Ipv4, protocol::Tcp>;

/**
 * \ingroup net-module-tcp
 * \brief Helper to create TCP/Ipv6 sockets.
 */
using SocketTcpIpv6 = Socket<address::Ipv6, protocol::Tcp>;

/**
 * \ingroup net-module-udp
 * \brief Helper to create UDP sockets.
 */
template <typename Address>
using SocketUdp = Socket<Address, protocol::Udp>;

/**
 * \ingroup net-module-udp
 * \brief Helper to create UDP/Ipv4 or UDP/Ipv6 sockets.
 */
using SocketUdpIp = Socket<address::Ip, protocol::Udp>;

/**
 * \ingroup net-module-udp
 * \brief Helper to create UDP/Ipv4 sockets.
 */
using SocketUdpIpv4 = Socket<address::Ipv4, protocol::Udp>;

/**
 * \ingroup net-module-udp
 * \brief Helper to create UDP/Ipv6 sockets.
 */
using SocketUdpIpv6 = Socket<address::Ipv6, protocol::Udp>;

#if !defined(_WIN32)

/**
 * \ingroup net-module-tcp
 * \brief Helper to create TCP/Local sockets.
 */
using SocketTcpLocal = Socket<address::Local, protocol::Tcp>;

/**
 * \ingroup net-module-udp
 * \brief Helper to create UDP/Local sockets.
 */
using SocketUdpLocal = Socket<address::Local, protocol::Udp>;

#endif

#if !defined(NET_NO_SSL)

/**
 * \ingroup net-module-tls
 * \brief Helper to create TLS sockets.
 */
template <typename Address>
using SocketTls = Socket<Address, protocol::Tls>;

/**
 * \ingroup net-module-tls
 * \brief Helper to create TLS/Ipv4 or TLS/Ipv6 sockets.
 */
using SocketTlsIp = Socket<address::Ip, protocol::Tls>;

/**
 * \ingroup net-module-tls
 * \brief Helper to create TLS/Ipv4 sockets.
 */
using SocketTlsIpv4 = Socket<address::Ip, protocol::Tls>;

/**
 * \ingroup net-module-tls
 * \brief Helper to create TLS/Ipv6 sockets.
 */
using SocketTlsIpv6 = Socket<address::Ip, protocol::Tls>;

#endif

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
 * \throw net::Error on failures
 */
inline address::AddressIterator resolve(const std::string &host, const std::string &service, int domain = AF_UNSPEC, int type = 0)
{
#if !defined(NET_NO_AUTO_INIT)
		net::init();
#endif

	struct addrinfo hints, *res, *p;

	std::memset(&hints, 0, sizeof (hints));
	hints.ai_family = domain;
	hints.ai_socktype = type;

	int e = getaddrinfo(host.c_str(), service.c_str(), &hints, &res);

	if (e != 0)
		throw Error(Error::System, "getaddrinfo", gai_strerror(e));

	std::vector<address::GenericAddress> addresses;

	for (p = res; p != nullptr; p = p->ai_next)
		addresses.push_back(address::GenericAddress(p->ai_addr, p->ai_addrlen));

	return address::AddressIterator(addresses, 0);
}

/**
 * Overloaded function.
 *
 * \param sc the parent socket
 * \param host the hostname
 * \param service the service name
 * \return the address iterator
 * \throw net::Error on failures
 */
template <typename Address, typename Protocol>
address::AddressIterator resolve(const Socket<Address, Protocol> &sc, const std::string &host, const std::string &service)
{
	return resolve(host, service, Address().domain(), sc.protocol().type());
}

/**
 * Resolve the first address.
 *
 * \param host the hostname
 * \param service the service name
 * \param domain the domain (e.g. AF_INET)
 * \param type the type (e.g. SOCK_STREAM)
 * \return the first generic address available
 * \throw net::Error on failures
 * \note do not use AF_UNSPEC and 0 as type for this function
 */
inline address::GenericAddress resolveOne(const std::string &host, const std::string &service, int domain, int type)
{
	address::AddressIterator end;
	address::AddressIterator it = resolve(host, service, domain, type);

	if (it == end)
		throw Error(Error::Other, "resolveOne", "no address available");

	return *it;
}

/**
 * Overloaded function
 *
 * \param sc the parent socket
 * \param host the hostname
 * \param service the service name
 * \return the first generic address available
 * \throw net::Error on failures
 */
template <typename Address, typename Protocol>
address::GenericAddress resolveOne(const Socket<Address, Protocol> &sc, const std::string &host, const std::string &service)
{
	return resolveOne(host, service, Address().domain(), sc.protocol().type());
}

} // !irccd

} // !net

#endif // !NET_HPP
