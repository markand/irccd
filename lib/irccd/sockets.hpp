/*
 * sockets.hpp -- portable C++ socket wrappers
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

#ifndef IRCCD_SOCKETS_HPP
#define IRCCD_SOCKETS_HPP

/**
 * \file sockets.hpp
 * \brief Portable socket abstraction
 *
 * # Introduction
 *
 * This file is a portable networking library.
 *
 * ## Options
 *
 * The user may set the following variables before compiling these files:
 *
 * ### General options
 *
 * - **SOCKET_NO_AUTO_INIT**: (bool) Set to 0 if you don't want Socket class to
 * automatically calls net::init function and net::finish functions.
 * - **SOCKET_NO_SSL**: (bool) Set to 0 if you don't have access to OpenSSL library.
 * - **SOCKET_NO_AUTO_SSL_INIT**: (bool) Set to 0 if you don't want Socket class with Tls to automatically init
 * the OpenSSL library. You will need to call net::ssl::init and net::ssl::finish.
 *
 * ### Options for Listener class
 *
 * Feature detection, multiple implementations may be avaible, for example, Linux has poll, select and epoll.
 *
 * We assume that `select(2)` is always available.
 *
 * Of course, you can set the variables yourself if you test it with your build system.
 *
 * - **SOCKET_HAVE_POLL**: Defined on all BSD, Linux. Also defined on Windows
 *   if _WIN32_WINNT is set to 0x0600 or greater.
 * - **SOCKET_HAVE_KQUEUE**: Defined on all BSD and Apple.
 * - **SOCKET_HAVE_EPOLL**: Defined on Linux only.
 * - **SOCKET_DEFAULT_BACKEND**: Which backend to use (e.g. `Select`).
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

#if defined(_WIN32)
#  if _WIN32_WINNT >= 0x0600 && !defined(SOCKET_HAVE_POLL)
#    define SOCKET_HAVE_POLL
#  endif
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__APPLE__)
#  if !defined(SOCKET_HAVE_KQUEUE)
#    define SOCKET_HAVE_KQUEUE
#  endif
#  if !defined(SOCKET_HAVE_POLL)
#    define SOCKET_HAVE_POLL
#  endif
#elif defined(__linux__)
#  if !defined(SOCKET_HAVE_EPOLL)
#    define SOCKET_HAVE_EPOLL
#  endif
#  if !defined(SOCKET_HAVE_POLL)
#    define SOCKET_HAVE_POLL
#  endif
#endif

/*
 * Define SOCKET_DEFAULT_BACKEND
 * ------------------------------------------------------------------
 */

/**
 * Defines the default Listener backend to use.
 *
 * \note Do not rely on the value shown in doxygen.
 */
#if defined(_WIN32)
#  if !defined(SOCKET_DEFAULT_BACKEND)
#    if defined(SOCKET_HAVE_POLL)
#      define SOCKET_DEFAULT_BACKEND Poll
#    else
#      define SOCKET_DEFAULT_BACKEND Select
#    endif
#  endif
#elif defined(__linux__)
#  include <sys/epoll.h>

#  if !defined(SOCKET_DEFAULT_BACKEND)
#    define SOCKET_DEFAULT_BACKEND Epoll
#  endif
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__) || defined(__APPLE__)
#  include <sys/types.h>
#  include <sys/event.h>
#  include <sys/time.h>

#  if !defined(SOCKET_DEFAULT_BACKEND)
#    define SOCKET_DEFAULT_BACKEND Kqueue
#  endif
#else
#  if !defined(SOCKET_DEFAULT_BACKEND)
#    define SOCKET_DEFAULT_BACKEND Select
#  endif
#endif

#if defined(SOCKET_HAVE_POLL) && !defined(_WIN32)
#  include <poll.h>
#endif

/*
 * Headers to include
 * ------------------------------------------------------------------
 */

#if defined(_WIN32)
#  include <cstdlib>

#  include <WinSock2.h>
#  include <WS2tcpip.h>
#else
#  include <cerrno>

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

#if !defined(SOCKET_NO_SSL)
#  include <openssl/err.h>
#  include <openssl/evp.h>
#  include <openssl/ssl.h>
#endif

#include <cassert>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace irccd {

/**
 * General network namespace.
 */
namespace net {

/*
 * Portables types
 * ------------------------------------------------------------------
 *
 * The following types are defined differently between Unix and Windows.
 */

/* {{{ Protocols */

#if defined(_WIN32)

/**
 * Socket type, SOCKET.
 */
using Handle	= SOCKET;

/**
 * Argument to pass to set.
 */
using ConstArg	= const char *;

/**
 * Argument to pass to get.
 */
using Arg	= char *;

#else

/**
 * Socket type, int.
 */
using Handle	= int;

/**
 * Argument to pass to set.
 */
using ConstArg	= const void *;

/**
 * Argument to pass to get.
 */
using Arg	= void *;

#endif

/* }}} */

/*
 * Portable constants
 * ------------------------------------------------------------------
 *
 * These constants are needed to check functions return codes, they are rarely needed in end user code.
 */

/* {{{ Constants */

/*
 * The following constants are defined differently from Unix
 * to Windows.
 */
#if defined(_WIN32)

/**
 * Socket creation failure or invalidation.
 */
extern const Handle Invalid;

/**
 * Socket operation failure.
 */
extern const int Failure;

#else

/**
 * Socket creation failure or invalidation.
 */
extern const int Invalid;

/**
 * Socket operation failure.
 */
extern const int Failure;

#endif

#if !defined(SOCKET_NO_SSL)

namespace ssl {

/**
 * \enum Method
 * \brief Which OpenSSL method to use.
 */
enum Method {
	Tlsv1,		//!< TLS v1.2 (recommended)
	Sslv3		//!< SSLv3
};

} // !ssl

#endif

/* }}} */

/*
 * Portable functions
 * ------------------------------------------------------------------
 *
 * The following free functions can be used to initialize the library or to get the last system error.
 */

/* {{{ Functions */

/**
 * Initialize the socket library. Except if you defined SOCKET_NO_AUTO_INIT, you don't need to call this
 * function manually.
 */
void init() noexcept;

/**
 * Close the socket library.
 */
void finish() noexcept;

#if !defined(SOCKET_NO_SSL)

/**
 * OpenSSL namespace.
 */
namespace ssl {

/**
 * Initialize the OpenSSL library. Except if you defined SOCKET_NO_AUTO_SSL_INIT, you don't need to call this function
 * manually.
 */
void init() noexcept;

/**
 * Close the OpenSSL library.
 */
void finish() noexcept;

} // !ssl

#endif // SOCKET_NO_SSL

/**
 * Get the last socket system error. The error is set from errno or from
 * WSAGetLastError on Windows.
 *
 * \return a string message
 */
std::string error();

/**
 * Get the last system error.
 *
 * \param errn the error number (errno or WSAGetLastError)
 * \return the error
 */
std::string error(int errn);

/* }}} */

/*
 * Error class
 * ------------------------------------------------------------------
 *
 * This is the main exception thrown on socket operations.
 */

/* {{{ Error */

/**
 * \class Error
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
	Error(Code code, std::string function);

	/**
	 * Constructor that use the system error set by the user.
	 *
	 * \param code which kind of error
	 * \param function the function name
	 * \param error the error
	 */
	Error(Code code, std::string function, int error);

	/**
	 * Constructor that set the error specified by the user.
	 *
	 * \param code which kind of error
	 * \param function the function name
	 * \param error the error
	 */
	Error(Code code, std::string function, std::string error);

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
	const char *what() const noexcept
	{
		return m_error.c_str();
	}
};

/* }}} */

/*
 * State class
 * ------------------------------------------------------------------
 *
 * To facilitate higher-level stuff, the socket has a state.
 */

/* {{{ State */

/**
 * \enum State
 * \brief Current socket state.
 */
enum class State {
	Open,			//!< Socket is open
	Bound,			//!< Socket is bound to an address
	Connecting,		//!< The connection is in progress
	Connected,		//!< Connection is complete
	Accepted,		//!< Socket has been accepted (client)
	Accepting,		//!< The client acceptation is in progress
	Closed,			//!< The socket has been closed
	Disconnected,		//!< The connection was lost
};

/* }}} */

/*
 * Action enum
 * ------------------------------------------------------------------
 *
 * Defines the pending operation.
 */

/* {{{ Action */

/**
 * \enum Action
 * \brief Define the current operation that must complete.
 *
 * Some operations like accept, connect, recv or send must sometimes do several round-trips to complete and the socket
 * action is set with that enumeration. The user is responsible of calling accept, connect send or recv until the
 * operation is complete.
 *
 * Note: the user must wait for the appropriate condition in Socket::condition to check if the required condition is
 * met.
 *
 * It is important to complete the operation in the correct order because protocols like Tls may require to continue
 * re-negociating when calling Socket::send or Socket::Recv.
 */
enum class Action {
	None,		//!< No action is required, socket is ready
	Accept,		//!< The socket is not yet accepted, caller must call accept() again
	Connect,	//!< The socket is not yet connected, caller must call connect() again
	Receive,	//!< The received operation has not succeeded yet, caller must call recv() or recvfrom() again
	Send		//!< The send operation has not succeded yet, caller must call send() or sendto() again
};

/* }}} */

/*
 * Condition enum
 * ------------------------------------------------------------------
 *
 * Defines if we must wait for reading or writing.
 */

/* {{{ Condition */

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
constexpr Condition operator^(Condition v1, Condition v2) noexcept
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
constexpr Condition operator&(Condition v1, Condition v2) noexcept
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
constexpr Condition operator|(Condition v1, Condition v2) noexcept
{
	return static_cast<Condition>(static_cast<int>(v1) | static_cast<int>(v2));
}

/**
 * Apply bitwise NOT.
 *
 * \param v the value
 * \return the complement
 */
constexpr Condition operator~(Condition v) noexcept
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

/* }}} */

/*
 * Base Socket class
 * ------------------------------------------------------------------
 *
 * This base class has operations that are common to all types of sockets but you usually instanciate
 * a SocketTcp or SocketUdp
 */

/* {{{ Socket */

/**
 * \class Socket
 * \brief Base socket class for socket operations.
 *
 * **Important:** When using non-blocking sockets, some considerations must be taken. See the implementation of the
 * underlying protocol for more details.
 *
 * \see protocol::Tls
 * \see protocol::Tcp
 * \see protocol::Udp
 */
template <typename Address, typename Protocol>
class Socket {
private:
	Protocol m_proto;
	State m_state{State::Closed};
	Action m_action{Action::None};
	Condition m_condition{Condition::None};

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
	 * \post state is set to Open
	 * \post handle is not set to Invalid
	 */
	Socket(int domain, int type, int protocol, Protocol iface = {})
		: m_proto(std::move(iface))
	{
#if !defined(SOCKET_NO_AUTO_INIT)
		init();
#endif
		m_handle = ::socket(domain, type, protocol);

		if (m_handle == Invalid) {
			throw Error{Error::System, "socket"};
		}

		m_proto.create(*this);
		m_state = State::Open;

		assert(m_handle != Invalid);
	}

	/**
	 * This tries to create a socket.
	 *
	 * Domain and type are determined by the Address and Protocol object.
	 *
	 * \param protocol the protocol
	 * \param address which type of address
	 * \throw net::Error on errors
	 */
	explicit inline Socket(Protocol protocol = {}, const Address &address = {})
		: Socket{address.domain(), protocol.type(), 0, std::move(protocol)}
	{
	}

	/**
	 * Construct a socket with an already created descriptor.
	 *
	 * \param handle the native descriptor
	 * \param state specify the socket state
	 * \param protocol the type of socket implementation
	 * \post action is set to None
	 * \post condition is set to None
	 */
	explicit inline Socket(Handle handle, State state = State::Closed, Protocol protocol = {}) noexcept
		: m_proto(std::move(protocol))
		, m_state{state}
		, m_handle{handle}
	{
		assert(m_action == Action::None);
		assert(m_condition == Condition::None);
	}

	/**
	 * Create an invalid socket. Can be used when you cannot instanciate the socket immediately.
	 */
	explicit inline Socket(std::nullptr_t) noexcept
		: m_handle{Invalid}
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
		, m_state{other.m_state}
		, m_action{other.m_action}
		, m_condition{other.m_condition}
		, m_handle{other.m_handle}
	{
		/* Invalidate other */
		other.m_handle = Invalid;
		other.m_state = State::Closed;
		other.m_action = Action::None;
		other.m_condition = Condition::None;
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
	 * Get the current socket state.
	 *
	 * \return the state
	 */
	inline State state() const noexcept
	{
		return m_state;
	}

	/**
	 * Change the current socket state.
	 *
	 * \param state the new state
	 * \warning only implementations should call this function
	 */
	inline void setState(State state) noexcept
	{
		m_state = state;
	}

	/**
	 * Get the pending operation.
	 *
	 * \return the action to complete before continuing
	 * \note usually only needed in non-blocking sockets
	 */
	inline Action action() const noexcept
	{
		return m_action;
	}

	/**
	 * Change the pending operation.
	 *
	 * \param action the action
	 * \warning you should not call this function yourself
	 */
	inline void setAction(Action action) noexcept
	{
		m_action = action;
	}

	/**
	 * Get the condition to wait for.
	 *
	 * \return the condition
	 */
	inline Condition condition() const noexcept
	{
		return m_condition;
	}

	/**
	 * Change the condition required.
	 *
	 * \param condition the condition
	 * \warning you should not call this function yourself
	 */
	inline void setCondition(Condition condition) noexcept
	{
		m_condition = condition;
	}

	/**
	 * Set an option for the socket. Wrapper of setsockopt(2).
	 *
	 * \param level the setting level
	 * \param name the name
	 * \param arg the value
	 * \throw net::Error on errors
	 */
	template <typename Argument>
	void set(int level, int name, const Argument &arg)
	{
		if (setsockopt(m_handle, level, name, (ConstArg)&arg, sizeof (arg)) == Failure) {
			throw Error{Error::System, "set"};
		}
	}

	/**
	 * Object-oriented option setter.
	 *
	 * The object must have `set(Socket<Address, Protocol> &) const`.
	 *
	 * \param option the option
	 * \throw net::Error on errors
	 */
	template <typename Option>
	inline void set(const Option &option)
	{
		option.set(*this);
	}

	/**
	 * Get an option for the socket. Wrapper of getsockopt(2).
	 *
	 * \param level the setting level
	 * \param name the name
	 * \throw net::Error on errors
	 */
	template <typename Argument>
	Argument get(int level, int name)
	{
		Argument desired, result{};
		socklen_t size = sizeof (result);

		if (getsockopt(m_handle, level, name, (Arg)&desired, &size) == Failure) {
			throw Error{Error::System, "get"};
		}

		std::memcpy(&result, &desired, size);

		return result;
	}

	/**
	 * Object-oriented option getter.
	 *
	 * The object must have `T get(Socket<Address, Protocol> &) const`, T can be any type and it is the value
	 * returned from this function.
	 *
	 * \return the same value as get() in the option
	 * \throw net::Error on errors
	 */
	template <typename Option>
	inline auto get() -> decltype(std::declval<Option>().get(*this))
	{
		return Option{}.get(*this);
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
	 * \param address the address
	 * \param length the size
	 * \pre state must not be Bound
	 * \throw net::Error on errors
	 */
	void bind(const sockaddr *address, socklen_t length)
	{
		assert(m_state != State::Bound);

		if (::bind(m_handle, address, length) == Failure) {
			throw Error{Error::System, "bind"};
		}

		m_state = State::Bound;
	}

	/**
	 * Overload that takes an address.
	 *
	 * \param address the address
	 * \throw net::Error on errors
	 */
	inline void bind(const Address &address)
	{
		bind(address.address(), address.length());
	}

	/**
	 * Listen for pending connection.
	 *
	 * \param max the maximum number
	 * \pre state must be Bound
	 * \throw net::Error on errors
	 */
	inline void listen(int max = 128)
	{
		assert(m_state == State::Bound);

		if (::listen(this->m_handle, max) == Failure) {
			throw Error{Error::System, "listen"};
		}
	}

	/**
	 * Connect to the address.
	 *
	 * If connection cannot be established immediately, connect with no argument must be called again. See
	 * the underlying protocol for more information.
	 *
	 * \pre state must be State::Open
	 * \param address the address
	 * \param length the the address length
	 * \throw net::Error on errors
	 * \post state is set to State::Connecting or State::Connected
	 * \note For non-blocking sockets, see the underlying protocol function for more details
	 */
	void connect(const sockaddr *address, socklen_t length)
	{
		assert(m_state == State::Open);

		m_action = Action::None;
		m_condition = Condition::None;

		m_proto.connect(*this, address, length);

		assert((m_state == State::Connected  && m_action == Action::None    && m_condition == Condition::None) ||
		       (m_state == State::Connecting && m_action == Action::Connect && m_condition != Condition::None));
	}

	/**
	 * Overloaded function.
	 *
	 * Effectively call connect(address.address(), address.length());
	 *
	 * \param address the address
	 */
	inline void connect(const Address &address)
	{
		connect(address.address(), address.length());
	}

	/**
	 * Continue the connection, only required with non-blocking sockets.
	 *
	 * \pre state must be State::Connecting
	 * \throw net::Error on errors
	 */
	void connect()
	{
		assert(m_state == State::Connecting);

		m_action = Action::None;
		m_condition = Condition::None;

		m_proto.connect(*this);

		assert((m_state == State::Connected  && m_action == Action::None    && m_condition == Condition::None) ||
		       (m_state == State::Connecting && m_action == Action::Connect && m_condition != Condition::None));
	}

	/**
	 * Accept a new client. If there are no pending connection, throws an error.
	 *
	 * If the client cannot be accepted immediately, the client is returned and accept with no arguments
	 * must be called on it. See the underlying protocol for more information.
	 *
	 * \pre state must be State::Bound
	 * \param info the address where to store client's information (optional)
	 * \return the new socket
	 * \throw Error on errors
	 * \post returned client's state is set to State::Accepting or State::Accepted
	 * \note For non-blocking sockets, see the underlying protocol function for more details
	 */
	Socket<Address, Protocol> accept(Address *info)
	{
		assert(m_state == State::Bound);

		m_action = Action::None;
		m_condition = Condition::None;

		sockaddr_storage storage;
		socklen_t length = sizeof (storage);

		Socket<Address, Protocol> sc = m_proto.accept(*this, reinterpret_cast<sockaddr *>(&storage), &length);

		if (info) {
			*info = Address{&storage, length};
		}

		/* Master do not change */
		assert(m_state == State::Bound);
		assert(m_action == Action::None);
		assert(m_condition == Condition::None);

		/* Client */
		assert(
			(sc.state() == State::Accepting && sc.action() == Action::Accept && sc.condition() != Condition::None) ||
			(sc.state() == State::Accepted  && sc.action() == Action::None   && sc.condition() == Condition::None)
		);

		return sc;
	}

	/**
	 * Continue the accept process on this client. This function must be called only when the socket is
	 * ready to be readable or writable! (see condition).
	 *
	 * \pre state must be State::Accepting
	 * \throw Error on errors
	 * \post if connection is complete, state is changed to State::Accepted, action and condition are unset
	 * \post if connection is still in progress, condition is set
	 */
	void accept()
	{
		assert(m_state == State::Accepting);

		m_action = Action::None;
		m_condition = Condition::None;

		m_proto.accept(*this);

		assert(
			(m_state == State::Accepting && m_action == Action::Accept && m_condition != Condition::None) ||
			(m_state == State::Accepted  && m_action == Action::None   && m_condition == Condition::None)
		);
	}

	/**
	 * Get the local name. This is a wrapper of getsockname().
	 *
	 * \return the address
	 * \throw Error on failures
	 * \pre state() must not be State::Closed
	 */
	Address address() const
	{
		assert(m_state != State::Closed);

		sockaddr_storage ss;
		socklen_t length = sizeof (sockaddr_storage);

		if (::getsockname(m_handle, (sockaddr *)&ss, &length) == Failure) {
			throw Error{Error::System, "getsockname"};
		}

		return Address(&ss, length);
	}

	/**
	 * Receive some data.
	 *
	 * If the operation cannot be complete immediately, 0 is returned and user must call the function
	 * again when ready. See the underlying protocol for more information.
	 *
	 * If action is set to Action::None and result is set to 0, disconnection occured.
	 *
	 * \param data the destination buffer
	 * \param length the buffer length
	 * \pre action must not be Action::Send
	 * \return the number of bytes received or 0
	 * \throw Error on error
	 * \note For non-blocking sockets, see the underlying protocol function for more details
	 */
	unsigned recv(void *data, unsigned length)
	{
		assert(m_action != Action::Send);

		m_action = Action::None;
		m_condition = Condition::None;

		unsigned nbread = m_proto.recv(*this, data, length);

		return nbread;
	}

	/**
	 * Overloaded function.
	 *
	 * \param count the number of bytes to receive
	 * \return the string
	 * \throw Error on error
	 */
	inline std::string recv(unsigned count)
	{
		std::string result;

		result.resize(count);
		auto n = recv(const_cast<char *>(result.data()), count);
		result.resize(n);

		return result;
	}

	/**
	 * Send some data.
	 *
	 * If the operation cannot be complete immediately, 0 is returned and user must call the function
	 * again when ready. See the underlying protocol for more information.
	 *
	 * \param data the data buffer
	 * \param length the buffer length
	 * \return the number of bytes sent or 0
	 * \pre action() must not be Flag::Receive
	 * \throw Error on error
	 * \note For non-blocking sockets, see the underlying protocol function for more details
	 */
	unsigned send(const void *data, unsigned length)
	{
		assert(m_action != Action::Receive);

		m_action = Action::None;
		m_condition = Condition::None;

		unsigned nbsent = m_proto.send(*this, data, length);

		assert((m_action == Action::None && m_condition == Condition::None) ||
		       (m_action == Action::Send && m_condition != Condition::None));

		return nbsent;
	}

	/**
	 * Overloaded function.
	 *
	 * \param data the string to send
	 * \return the number of bytes sent
	 * \throw Error on error
	 */
	inline unsigned send(const std::string &data)
	{
		return send(data.c_str(), data.size());
	}

	/**
	 * Send data to an end point.
	 *
	 * If the operation cannot be complete immediately, 0 is returned and user must call the function
	 * again when ready. See the underlying protocol for more information.
	 *
	 * \param data the buffer
	 * \param length the buffer length
	 * \param address the client address
	 * \param addrlen the address length
	 * \return the number of bytes sent
	 * \throw net::Error on errors
	 * \note For non-blocking sockets, see the underlying protocol function for more details
	 */
	inline unsigned sendto(const void *data, unsigned length, const sockaddr *address, socklen_t addrlen)
	{
		return m_proto.sendto(*this, data, length, address, addrlen);
	}

	/**
	 * Overloaded function.
	 *
	 * \param data the buffer
	 * \param length the buffer length
	 * \param address the destination
	 * \return the number of bytes sent
	 * \throw net::Error on errors
	 */
	inline unsigned sendto(const void *data, unsigned length, const Address &address)
	{
		return sendto(data, length, address.address(), address.length());
	}

	/**
	 * Overloaded function.
	 *
	 * \param data the data
	 * \param address the address
	 * \return the number of bytes sent
	 * \throw net:;Error on errors
	 */
	inline unsigned sendto(const std::string &data, const Address &address)
	{
		return sendto(data.c_str(), data.length(), address);
	}

	/**
	 * Receive data from an end point.
	 *
	 * If the operation cannot be complete immediately, 0 is returned and user must call the function
	 * again when ready. See the underlying protocol for more information.
	 *
	 * \param data the destination buffer
	 * \param length the buffer length
	 * \param address the address destination
	 * \param addrlen the address length (in/out)
	 * \return the number of bytes received
	 * \throw net::Error on errors
	 * \note For non-blocking sockets, see the underlying protocol function for more details
	 */
	inline unsigned recvfrom(void *data, unsigned length, sockaddr *address, socklen_t *addrlen)
	{
		return m_proto.recvfrom(*this, data, length, address, addrlen);
	}

	/**
	 * Overloaded function.
	 *
	 * \param data the destination buffer
	 * \param length the buffer length
	 * \param info the address destination
	 * \return the number of bytes received
	 * \throw net::Error on errors
	 */
	inline unsigned recvfrom(void *data, unsigned length, Address *info = nullptr)
	{
		sockaddr_storage storage;
		socklen_t addrlen = sizeof (sockaddr_storage);

		auto n = recvfrom(data, length, reinterpret_cast<sockaddr *>(&storage), &addrlen);

		if (info && n != 0) {
			*info = Address{&storage, addrlen};
		}

		return n;
	}

	/**
	 * Overloaded function.
	 *
	 * \param count the maximum number of bytes to receive
	 * \param info the client information
	 * \return the string
	 * \throw net::Error on errors
	 */
	std::string recvfrom(unsigned count, Address *info = nullptr)
	{
		std::string result;

		result.resize(count);
		auto n = recvfrom(const_cast<char *>(result.data()), count, info);
		result.resize(n);

		return result;
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

		m_state = State::Closed;
		m_action = Action::None;
		m_condition = Condition::None;
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
		m_state = other.m_state;
		m_action = other.m_action;
		m_condition = other.m_condition;

		/* Invalidate other */
		other.m_handle = Invalid;
		other.m_state = State::Closed;
		other.m_action = Action::None;
		other.m_condition = Condition::None;

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
bool operator==(const Socket<Address, Protocol> &s1, const Socket<Address, Protocol> &s2)
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
bool operator!=(const Socket<Address, Protocol> &s1, const Socket<Address, Protocol> &s2)
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
bool operator<(const Socket<Address, Protocol> &s1, const Socket<Address, Protocol> &s2)
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
bool operator>(const Socket<Address, Protocol> &s1, const Socket<Address, Protocol> &s2)
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
bool operator<=(const Socket<Address, Protocol> &s1, const Socket<Address, Protocol> &s2)
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
bool operator>=(const Socket<Address, Protocol> &s1, const Socket<Address, Protocol> &s2)
{
	return s1.handle() >= s2.handle();
}

/* }}} */

/*
 * Predefined options
 * ------------------------------------------------------------------
 */

/* {{{ Options */

/**
 * Namespace of predefined options.
 */
namespace option {

/*
 * Options for socket
 * ------------------------------------------------------------------
 */

/* {{{ Options for socket */

/**
 * \class SockBlockMode
 * \brief Set or get the blocking-mode for a socket.
 * \warning On Windows, it's not possible to check if the socket is blocking or not.
 */
class SockBlockMode {
public:
	/**
	 * Set to false if you want non-blocking socket.
	 */
	bool value{false};

	SockBlockMode(bool v)
		: value(v)
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

		if ((flags = fcntl(sc.handle(), F_GETFL, 0)) < 0) {
			flags = 0;
		}

		if (value) {
			flags &= ~(O_NONBLOCK);
		} else {
			flags |= O_NONBLOCK;
		}

		if (fcntl(sc.handle(), F_SETFL, flags) < 0) {
			throw Error{Error::System, "fcntl"};
		}
#else
		unsigned long flags = (value) ? 0 : 1;

		if (ioctlsocket(sc.handle(), FIONBIO, &flags) == Failure) {
			throw Error{Error::System, "fcntl"};
		}
#endif
	}

	/**
	 * Get the option.
	 *
	 * \return the value
	 * \throw Error on errors
	 */
	template <typename Address, typename Protocol>
	bool get(Socket<Address, Protocol> &sc) const
	{
#if defined(O_NONBLOCK) && !defined(_WIN32)
		int flags = fcntl(sc.handle(), F_GETFL, 0);

		if (flags < 0) {
			throw Error{Error::System, "fcntl"};
		}

		return !(flags & O_NONBLOCK);
#else
		throw Error{Error::Other, "get", "Windows API cannot let you get the blocking status of a socket"};
#endif
	}
};

/**
 * \class SockReuseAddress
 * \brief Reuse address, must be used before calling Socket::bind
 */
class SockReuseAddress {
public:
	/**
	 * Set to true if you want to set the SOL_SOCKET/SO_REUSEADDR option.
	 */
	bool value{true};

	SockReuseAddress(bool v)
		: value(v)
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
		sc.set(SOL_SOCKET, SO_REUSEADDR, value ? 1 : 0);
	}

	/**
	 * Get the option.
	 *
	 * \return the value
	 * \throw Error on errors
	 */
	template <typename Address, typename Protocol>
	inline bool get(Socket<Address, Protocol> &sc) const
	{
		return static_cast<bool>(sc.template get<int>(SOL_SOCKET, SO_REUSEADDR));
	}
};

/**
 * \class SockSendBuffer
 * \brief Set or get the output buffer.
 */
class SockSendBuffer {
public:
	/**
	 * Set to the buffer size.
	 */
	int value{2048};

	/**
	 * Set the option.
	 *
	 * \param sc the socket
	 * \throw Error on errors
	 */
	template <typename Address, typename Protocol>
	inline void set(Socket<Address, Protocol> &sc) const
	{
		sc.set(SOL_SOCKET, SO_SNDBUF, value);
	}

	/**
	 * Get the option.
	 *
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
 * \class SockReceiveBuffer
 * \brief Set or get the input buffer.
 */
class SockReceiveBuffer {
public:
	/**
	 * Set to the buffer size.
	 */
	int value{2048};

	/**
	 * Set the option.
	 *
	 * \param sc the socket
	 * \throw Error on errors
	 */
	template <typename Address, typename Protocol>
	inline void set(Socket<Address, Protocol> &sc) const
	{
		sc.set(SOL_SOCKET, SO_RCVBUF, value);
	}

	/**
	 * Get the option.
	 *
	 * \return the value
	 * \throw Error on errors
	 */
	template <typename Address, typename Protocol>
	inline int get(Socket<Address, Protocol> &sc) const
	{
		return sc.template get<int>(SOL_SOCKET, SO_RCVBUF);
	}
};

/* }}} */

/**
 * \class TcpNoDelay
 * \brief Set this option if you want to disable nagle's algorithm.
 */
class TcpNoDelay {
public:
	/**
	 * Set to true to set TCP_NODELAY option.
	 */
	bool value{true};

	/**
	 * Set the option.
	 *
	 * \param sc the socket
	 * \throw Error on errors
	 */
	template <typename Address, typename Protocol>
	inline void set(Socket<Address, Protocol> &sc) const
	{
		sc.set(IPPROTO_TCP, TCP_NODELAY, value ? 1 : 0);
	}

	/**
	 * Get the option.
	 *
	 * \return the value
	 * \throw Error on errors
	 */
	template <typename Address, typename Protocol>
	inline bool get(Socket<Address, Protocol> &sc) const
	{
		return static_cast<bool>(sc.template get<int>(IPPROTO_TCP, TCP_NODELAY));
	}
};

/**
 * \class Ipv6Only
 * \brief Control IPPROTO_IPV6/IPV6_V6ONLY
 *
 * Note: some systems may or not set this option by default so it's a good idea to set it in any case to either
 * false or true if portability is a concern.
 */
class Ipv6Only {
public:
	/**
	 * Set this to use only IPv6.
	 */
	bool value{true};

	Ipv6Only(bool v)
		: value(v)
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
		sc.set(IPPROTO_IPV6, IPV6_V6ONLY, value ? 1 : 0);
	}

	/**
	 * Get the option.
	 *
	 * \return the value
	 * \throw Error on errors
	 */
	template <typename Address, typename Protocol>
	inline bool get(Socket<Address, Protocol> &sc) const
	{
		return static_cast<bool>(sc.template get<int>(IPPROTO_IPV6, IPV6_V6ONLY));
	}
};

} // !option

/* }}} */

/*
 * Predefined addressed to be used
 * ------------------------------------------------------------------
 *
 * - Ip,
 * - Local.
 */

/* {{{ Addresses */

/**
 * Set of predefined addresses.
 */
namespace address {

/**
 * \class Ip
 * \brief Base class for IPv6 and IPv4, you can use it if you don't know in advance if you'll use IPv6 or IPv4.
 */
class Ip {
public:
	/**
	 * \enum Type
	 * \brief Type of ip address.
	 */
	enum Type {
		v4 = AF_INET,		//!< AF_INET
		v6 = AF_INET6		//!< AF_INET6
	};

private:
	/*
	 * Default domain when using default constructors.
	 *
	 * Note: AF_INET or AF_INET6, not
	 */
	static int m_default;

	union {
		sockaddr_in m_sin;
		sockaddr_in6 m_sin6;
	};

	socklen_t m_length{0};
	int m_domain{AF_INET};

public:
	/**
	 * Set the default domain to use when using default Ip constructor. By default, AF_INET is used.
	 *
	 * \pre domain must be Type::v4 or Type::v6
	 */
	static inline void setDefault(Type domain) noexcept
	{
		assert(domain == Type::v4 || domain == Type::v6);

		m_default = static_cast<int>(domain);
	}

	/**
	 * Construct using the default domain.
	 */
	inline Ip() noexcept
		: Ip(static_cast<Type>(m_default))
	{
	}

	/**
	 * Default initialize the Ip domain.
	 *
	 * \pre domain must be AF_INET or AF_INET6 only
	 * \param domain the domain (AF_INET or AF_INET6)
	 */
	Ip(Type domain) noexcept;

	/**
	 * Construct an address suitable for bind() or connect().
	 *
	 * \pre domain must be Type::v4 or Type::v6
	 * \param domain the domain (AF_INET or AF_INET6)
	 * \param host the host (* for any)
	 * \param port the port number
	 * \throw Error on errors
	 */
	Ip(const std::string &host, int port, Type domain = v4);

	/**
	 * Construct an address from a storage.
	 *
	 * \pre storage's domain must be AF_INET or AF_INET6 only
	 * \param ss the storage
	 * \param length the length
	 */
	Ip(const sockaddr_storage *ss, socklen_t length) noexcept;

	/**
	 * Get the domain (AF_INET or AF_INET6).
	 *
	 * \return the domain
	 */
	inline int domain() const noexcept
	{
		return m_domain;
	}

	/**
	 * Return the underlying address, either sockaddr_in6 or sockaddr_in.
	 *
	 * \return the address
	 */
	inline const sockaddr *address() const noexcept
	{
		if (m_domain == AF_INET6) {
			return reinterpret_cast<const sockaddr *>(&m_sin6);
		}

		return reinterpret_cast<const sockaddr *>(&m_sin);
	}

	/**
	 * Return the underlying address length.
	 *
	 * \return the length
	 */
	inline socklen_t length() const noexcept
	{
		return m_length;
	}

	/**
	 * Get the port.
	 *
	 * \return the port
	 */
	inline int port() const noexcept
	{
		if (m_domain == AF_INET6) {
			return ntohs(m_sin6.sin6_port);
		}

		return ntohs(m_sin.sin_port);
	}
};

#if !defined(_WIN32)

/**
 * \class Local
 * \brief unix family sockets
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
	Local() noexcept;

	/**
	 * Construct an address to a path.
	 *
	 * \param path the path
	 * \param rm remove the file before (default: false)
	 */
	Local(std::string path, bool rm = false) noexcept;

	/**
	 * Construct an unix address from a storage address.
	 *
	 * \pre storage's domain must be AF_LOCAL
	 * \param ss the storage
	 * \param length the length
	 */
	Local(const sockaddr_storage *ss, socklen_t length) noexcept;

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
#if defined(SOCKET_HAVE_SUN_LEN)
		return SUN_LEN(&m_sun);
#else
		return sizeof (m_sun);
#endif
	}
};

#endif // !_WIN32

} // !address

/* }}} */

/*
 * Predefined protocols
 * ------------------------------------------------------------------
 *
 * - Tcp, for standard stream connections,
 * - Udp, for standard datagram connections,
 * - Tls, for secure stream connections.
 */

/* {{{ Protocols */

/**
 * Set of predefined protocols.
 */
namespace protocol {

/* {{{ Tcp */

/**
 * \class Tcp
 * \brief Clear TCP implementation.
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
		/* No-op */
	}

	/**
	 * Standard connect.
	 *
	 * If the socket is marked non-blocking and the connection cannot be established immediately, then the
	 * following is true:
	 *
	 * - state is set to State::Connecting,
	 * - action is set to Action::Connect,
	 * - condition is set to Condition::Writable.
	 *
	 * Then the user must wait until the socket is writable and call connect() with 0 arguments.
	 *
	 * If the socket is blocking, this function blocks until the connection is complete or an error occurs, in
	 * that case state is either set to State::Connected or State::Disconnected but action and condition are
	 * not set.
	 *
	 * \param sc the socket
	 * \param address the address
	 * \param length the length
	 * \throw net::Error on errors
	 * \note Wrapper of connect(2)
	 */
	template <typename Address, typename Protocol>
	void connect(Socket<Address, Protocol> &sc, const sockaddr *address, socklen_t length)
	{
		if (::connect(sc.handle(), address, length) == Failure) {
			/*
			 * Determine if the error comes from a non-blocking connect that cannot be
			 * accomplished yet.
			 */
#if defined(_WIN32)
			int error = WSAGetLastError();

			if (error == WSAEWOULDBLOCK) {
				sc.setState(State::Connecting);
				sc.setAction(Action::Connect);
				sc.setCondition(Condition::Writable);
			} else {
				sc.setState(State::Disconnected);
				throw Error{Error::System, "connect", error};
			}
#else
			if (errno == EINPROGRESS) {
				sc.setState(State::Connecting);
				sc.setAction(Action::Connect);
				sc.setCondition(Condition::Writable);
			} else {
				sc.setState(State::Disconnected);
				throw Error{Error::System, "connect"};
			}
#endif
		} else {
			sc.setState(State::Connected);
		}
	}

	/**
	 * Continue the connection. This function must only be called when the socket is ready for writing,
	 * the user is responsible of waiting for that condition.
	 *
	 * This function check for SOL_SOCKET/SO_ERROR status.
	 *
	 * If the connection is complete, status is set to State::Connected, otherwise it is set to
	 * State::Disconnected. In both cases, action and condition are not set.
	 *
	 * \param sc the socket
	 * \throw net::Error on errors
	 */
	template <typename Address, typename Protocol>
	void connect(Socket<Address, Protocol> &sc)
	{
		int error = sc.template get<int>(SOL_SOCKET, SO_ERROR);

		if (error == Failure) {
			sc.setState(State::Disconnected);
			throw Error{Error::System, "connect", error};
		}

		sc.setState(State::Connected);
	}

	/**
	 * Accept a clear client.
	 *
	 * If the socket is marked non-blocking and there are no pending connection, this function throws an
	 * error. The user must wait that the socket is readable before calling this function.
	 *
	 * If the socket is blocking, this function blocks until a new client is connected or throws an error on
	 * errors.
	 *
	 * If the socket is correctly returned, its state is set to State::Accepted and its action and condition
	 * are not set.
	 *
	 * In any case, action and condition of this socket are not set.
	 *
	 * \param sc the socket
	 * \param address the address destination
	 * \param length the address length
	 * \return the socket
	 * \throw net::Error on errors
	 * \note Wrapper of accept(2)
	 */
	template <typename Address, typename Protocol>
	Socket<Address, Protocol> accept(Socket<Address, Protocol> &sc, sockaddr *address, socklen_t *length)
	{
		Handle handle = ::accept(sc.handle(), address, length);

		if (handle == Invalid) {
			throw Error{Error::System, "accept"};
		}

		return Socket<Address, Protocol>{handle, State::Accepted};
	}

	/**
	 * Continue accept.
	 *
	 * This function is just present for compatibility, it should never be called.
	 */
	template <typename Address, typename Protocol>
	inline void accept(Socket<Address, Protocol> &) const noexcept
	{
		/* no-op */
	}

	/**
	 * Receive data.
	 *
	 * If the socket is marked non-blocking and no data is available, 0 is returned and condition is set to
	 * Condition::Readable. If 0 is returned and condition is not set, then the state is set to
	 * State::Disconnected.
	 *
	 * If the socket is blocking, this function blocks until some data is available or if an error occurs.
	 *
	 * In any case, action is never set.
	 *
	 * \param sc the socket
	 * \param data the destination
	 * \param length the destination length
	 * \return the number of bytes read
	 * \throw Error on errors
	 * \note Wrapper of recv(2)
	 */
	template <typename Address>
	unsigned recv(Socket<Address, Tcp> &sc, void *data, unsigned length)
	{
		int nbread = ::recv(sc.handle(), (Arg)data, length, 0);

		if (nbread == Failure) {
#if defined(_WIN32)
			int error = WSAGetLastError();

			if (error == WSAEWOULDBLOCK) {
				nbread = 0;
				sc.setCondition(Condition::Readable);
			} else {
				sc.setState(State::Disconnected);
				throw Error{Error::System, "recv", error};
			}
#else
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				sc.setCondition(Condition::Readable);
			} else {
				sc.setState(State::Disconnected);
				throw Error{Error::System, "recv"};
			}
#endif
		} else if (nbread == 0) {
			sc.setState(State::Disconnected);
		}

		return static_cast<unsigned>(nbread);
	}

	/**
	 * Send some data.
	 *
	 * If the socket is marked non-blocking and the operation would block, then 0 is returned and condition is set to
	 * Condition::Writable.
	 *
	 * If the socket is blocking, this function blocks until the data has been sent.
	 *
	 * On any other errors, this function throw net::Error.
	 *
	 * \param sc the socket
	 * \param data the buffer to send
	 * \param length the buffer length
	 * \return the number of bytes sent
	 * \throw net::Error on errors
	 * \note Wrapper of send(2)
	 */
	template <typename Address>
	unsigned send(Socket<Address, Tcp> &sc, const void *data, unsigned length)
	{
		int nbsent = ::send(sc.handle(), (ConstArg)data, length, 0);

		if (nbsent == Failure) {
#if defined(_WIN32)
			int error = WSAGetLastError();

			if (error == WSAEWOULDBLOCK) {
				nbsent = 0;
				sc.setCondition(Condition::Writable);
			} else {
				sc.setState(State::Disconnected);
				throw Error{Error::System, "send", error};
			}
#else
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				nbsent = 0;
				sc.setCondition(Condition::Writable);
			} else {
				sc.setState(State::Disconnected);
				throw Error{Error::System, "send"};
			}
#endif
		}

		return static_cast<unsigned>(nbsent);
	}
};

/* }}} */

/* {{{ Udp */

/**
 * \class Udp
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
	template <typename Address>
	inline void create(Socket<Address, Udp> &) noexcept
	{
		/* No-op */
	}

	/**
	 * Receive data from an end point.
	 *
	 * If the socket is marked non-blocking and no data is available, 0 is returned and condition is set to
	 * Condition::Readable.
	 *
	 * If the socket is blocking, this functions blocks until some data is available or if an error occurs.
	 *
	 * \param sc the socket
	 * \param data the destination buffer
	 * \param length the buffer length
	 * \param address the address
	 * \param addrlen the initial address length
	 * \return the number of bytes received
	 * \throw Error on error
	 */
	template <typename Address>
	unsigned recvfrom(Socket<Address, Udp> &sc, void *data, unsigned length, sockaddr *address, socklen_t *addrlen)
	{
		int nbread;

		nbread = ::recvfrom(sc.handle(), (Arg)data, length, 0, address, addrlen);

		if (nbread == Failure) {
#if defined(_WIN32)
			int error = WSAGetLastError();

			if (error == WSAEWOULDBLOCK) {
				nbread = 0;
				sc.setCondition(Condition::Readable);
			} else {
				throw Error{Error::System, "recvfrom"};
			}
#else
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				nbread = 0;
				sc.setCondition(Condition::Readable);
			} else {
				throw Error{Error::System, "recvfrom"};
			}
#endif
		}

		return static_cast<unsigned>(nbread);
	}

	/**
	 * Send data to an end point.
	 *
	 * If the socket is marked non-blocking and the operation would block, then 0 is returned and condition is set to
	 * Condition::Writable.
	 *
	 * If the socket is blocking, this functions blocks until the data has been sent.
	 *
	 * \param sc the socket
	 * \param data the buffer
	 * \param length the buffer length
	 * \param address the client address
	 * \param addrlen the adderss length
	 * \return the number of bytes sent
	 * \throw Error on error
	 */
	template <typename Address>
	unsigned sendto(Socket<Address, Udp> &sc, const void *data, unsigned length, const sockaddr *address, socklen_t addrlen)
	{
		int nbsent;

		nbsent = ::sendto(sc.handle(), (ConstArg)data, length, 0, address, addrlen);
		if (nbsent == Failure) {
#if defined(_WIN32)
			int error = WSAGetLastError();

			if (error == WSAEWOULDBLOCK) {
				nbsent = 0;
				sc.setCondition(Condition::Writable);
			} else {
				throw Error{Error::System, "sendto", error};
			}
#else
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				nbsent = 0;
				sc.setCondition(Condition::Writable);
			} else {
				throw Error{Error::System, "sendto"};
			}
#endif
		}

		return static_cast<unsigned>(nbsent);
	}
};

/* }}} */

/* {{{ Tls */

#if !defined(SOCKET_NO_SSL)

/**
 * \class Tls
 * \brief OpenSSL secure layer for TCP.
 *
 * **Note:** This protocol is much more difficult to use with non-blocking sockets, if some operations would block, the
 * user is responsible of calling the function again by waiting for the appropriate condition. See the functions for
 * more details.
 *
 * \see Tls::accept
 * \see Tls::connect
 * \see Tls::recv
 * \see Tls::send
 */
class Tls : private Tcp {
private:
	using Context = std::shared_ptr<SSL_CTX>;
	using Ssl = std::unique_ptr<SSL, void (*)(SSL *)>;

	/* OpenSSL objects */
	Context m_context;
	Ssl m_ssl{nullptr, nullptr};

	/* Status */
	bool m_tcpconnected{false};

	/*
	 * User definable parameters
	 */
	ssl::Method m_method{ssl::Tlsv1};
	std::string m_key;
	std::string m_certificate;
	bool m_verify{false};

	/*
	 * Construct with a context and ssl, for Tls::accept.
	 */
	Tls(Context context, Ssl ssl)
		: m_context{std::move(context)}
		, m_ssl{std::move(ssl)}
	{
	}

	/*
	 * Get the OpenSSL error message.
	 */
	inline std::string error(int error)
	{
		auto msg = ERR_reason_error_string(error);

		return msg == nullptr ? "" : msg;
	}

	/*
	 * Update the states after an uncompleted operation.
	 */
	template <typename Address, typename Protocol>
	inline void updateStates(Socket<Address, Protocol> &sc, State state, Action action, int code)
	{
		assert(code == SSL_ERROR_WANT_READ || code == SSL_ERROR_WANT_WRITE);

		sc.setState(state);
		sc.setAction(action);

		if (code == SSL_ERROR_WANT_READ) {
			sc.setCondition(Condition::Readable);
		} else {
			sc.setCondition(Condition::Writable);
		}
	}

	/*
	 * Continue the connect operation.
	 */
	template <typename Address, typename Protocol>
	void processConnect(Socket<Address, Protocol> &sc)
	{
		int ret = SSL_connect(m_ssl.get());

		if (ret <= 0) {
			int no = SSL_get_error(m_ssl.get(), ret);

			if (no == SSL_ERROR_WANT_READ || no == SSL_ERROR_WANT_WRITE) {
				updateStates(sc, State::Connecting, Action::Connect, no);
			} else {
				sc.setState(State::Disconnected);
				throw Error{Error::System, "connect", error(no)};
			}
		} else {
			sc.setState(State::Connected);
		}
	}

	/*
	 * Continue accept.
	 */
	template <typename Address, typename Protocol>
	void processAccept(Socket<Address, Protocol> &sc)
	{
		int ret = SSL_accept(m_ssl.get());

		if (ret <= 0) {
			int no = SSL_get_error(m_ssl.get(), ret);

			if (no == SSL_ERROR_WANT_READ || no == SSL_ERROR_WANT_WRITE) {
				updateStates(sc, State::Accepting, Action::Accept, no);
			} else {
				sc.setState(State::Disconnected);
				throw Error(Error::System, "accept", error(no));
			}
		} else {
			sc.setState(State::Accepted);
		}
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
	Tls()
	{
#if !defined(SOCKET_NO_SSL_AUTO_INIT)
		net::ssl::init();
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
	inline void create(Socket<Address, Tls> &sc)
	{
		auto method = (m_method == ssl::Tlsv1) ? TLSv1_method() : SSLv23_method();

		m_context = {SSL_CTX_new(method), SSL_CTX_free};
		m_ssl = {SSL_new(m_context.get()), SSL_free};

		SSL_set_fd(m_ssl.get(), sc.handle());

		/* Load certificates */
		if (m_certificate.size() > 0) {
			SSL_CTX_use_certificate_file(m_context.get(), m_certificate.c_str(), SSL_FILETYPE_PEM);
		}
		if (m_key.size() > 0) {
			SSL_CTX_use_PrivateKey_file(m_context.get(), m_key.c_str(), SSL_FILETYPE_PEM);
		}
		if (m_verify && !SSL_CTX_check_private_key(m_context.get())) {
			throw Error{Error::System, "(openssl)", "unable to verify key"};
		}
	}

	/**
	 * Connect to a secure host.
	 *
	 * If the socket is marked non-blocking and the connection cannot be established yet, then the state is set
	 * to State::Connecting, the condition is set to Condition::Readable or Condition::Writable, the user must
	 * wait for the appropriate condition before calling the overload connect which takes 0 argument.
	 *
	 * If the socket is blocking, this functions blocks until the connection is complete.
	 *
	 * If the connection was completed correctly the state is set to State::Connected.
	 *
	 * \param sc the socket
	 * \param address the address
	 * \param length the address length
	 * \throw net::Error on errors
	 */
	template <typename Address, typename Protocol>
	void connect(Socket<Address, Protocol> &sc, const sockaddr *address, socklen_t length)
	{
		/* 1. Connect using raw TCP */
		Tcp::connect(sc, address, length);

		/* 2. If the connection is complete (e.g. non-blocking), try handshake */
		if (sc.state() == State::Connected) {
			m_tcpconnected = true;
			processConnect(sc);
		}
	}

	/**
	 * Continue the connection.
	 *
	 * This function must be called when the socket is ready for reading or writing (check with Socket::condition),
	 * the state may change exactly like the initial connect call.
	 *
	 * \param sc the socket
	 * \throw net::Error on errors
	 */
	template <typename Address, typename Protocol>
	void connect(Socket<Address, Protocol> &sc)
	{
		/* 1. Be sure to complete standard connect before */
		if (!m_tcpconnected) {
			Tcp::connect(sc);
			m_tcpconnected = sc.state() == State::Connected;
		}

		if (m_tcpconnected) {
			processConnect(sc);
		}
	}

	/**
	 * Accept a secure client.
	 *
	 * Because SSL needs several round-trips, if the socket is marked non-blocking and the connection is not
	 * completed yet, a new socket is returned but with the State::Accepting state. Its condition is set to
	 * Condition::Readable or Condition::Writable, the user is responsible of calling accept overload which takes
	 * 0 arguments on the returned socket when the condition is met.
	 *
	 * If the socket is blocking, this function blocks until the client is accepted and returned.
	 *
	 * If the client is accepted correctly, its state is set to State::Accepted. This instance does not change.
	 *
	 * \param sc the socket
	 * \param address the address destination
	 * \param length the address length
	 * \return the client
	 * \throw net::Error on errors
	 */
	template <typename Address>
	Socket<Address, Tls> accept(Socket<Address, Tls> &sc, sockaddr *address, socklen_t *length)
	{
		Socket<Address, Tls> client = Tcp::accept(sc, address, length);
		Tls &proto = client.protocol();

		/* 1. Share the context */
		proto.m_context = m_context;

		/* 2. Create new SSL instance */
		proto.m_ssl = Ssl{SSL_new(m_context.get()), SSL_free};
		SSL_set_fd(proto.m_ssl.get(), client.handle());

		/* 3. Try accept process on the **new** client */
		proto.processAccept(client);

		return client;
	}

	/**
	 * Continue accept.
	 *
	 * This function must be called on the client that is being accepted.
	 *
	 * Like accept or connect, user is responsible of calling this function until the connection is complete.
	 *
	 * \param sc the socket
	 * \throw net::Error on errors
	 */
	template <typename Address, typename Protocol>
	inline void accept(Socket<Address, Protocol> &sc)
	{
		processAccept(sc);
	}

	/**
	 * Receive some secure data.
	 *
	 * If the socket is marked non-blocking, 0 is returned if no data is available yet or if the connection
	 * needs renegociation. If renegociation is required case, the action is set to Action::Receive and condition
	 * is set to Condition::Readable or Condition::Writable. The user must wait that the condition is met and
	 * call this function again.
	 *
	 * \param sc the socket
	 * \param data the destination
	 * \param len the buffer length
	 * \return the number of bytes read
	 * \throw net::Error on errors
	 */
	template <typename Address>
	unsigned recv(Socket<Address, Tls> &sc, void *data, unsigned len)
	{
		auto nbread = SSL_read(m_ssl.get(), data, len);

		if (nbread <= 0) {
			auto no = SSL_get_error(m_ssl.get(), nbread);

			if (no == SSL_ERROR_WANT_READ || no == SSL_ERROR_WANT_WRITE) {
				nbread = 0;
				updateStates(sc, sc.state(), Action::Receive, no);
			} else {
				throw Error{Error::System, "recv", error(no)};
			}
		}

		return nbread;
	}

	/**
	 * Send some data.
	 *
	 * Like recv, if the socket is marked non-blocking and no data can be sent or a negociation is required,
	 * condition and action are set. See receive for more details
	 *
	 * \param sc the socket
	 * \param data the data to send
	 * \param len the buffer length
	 * \return the number of bytes sent
	 * \throw net::Error on errors
	 */
	template <typename Address>
	unsigned send(Socket<Address, Tls> &sc, const void *data, unsigned len)
	{
		auto nbsent = SSL_write(m_ssl.get(), data, len);

		if (nbsent <= 0) {
			auto no = SSL_get_error(m_ssl.get(), nbsent);

			if (no == SSL_ERROR_WANT_READ || no == SSL_ERROR_WANT_WRITE) {
				nbsent = 0;
				updateStates(sc, sc.state(), Action::Send, no);
			} else {
				throw Error{Error::System, "send", error(no)};
			}
		}

		return nbsent;
	}
};

#endif // !SOCKET_NO_SSL

/* }}} */

} // !protocol

/* }}} */

/*
 * Convenient helpers
 * ------------------------------------------------------------------
 *
 * - SocketTcp<Address>, for TCP sockets,
 * - SocketUdp<Address>, for UDP sockets,
 * - SocketTls<Address>, for secure TCP sockets.
 */

/* {{{ Helpers */

/**
 * Helper to create TCP sockets.
 */
template <typename Address>
using SocketTcp = Socket<Address, protocol::Tcp>;

/**
 * Helper to create TCP/IP sockets.
 */
using SocketTcpIp = Socket<address::Ip, protocol::Tcp>;

#if !defined(_WIN32)

/**
 * Helper to create TCP/Local sockets.
 */
using SocketTcpLocal = Socket<address::Local, protocol::Tcp>;

#endif

/**
 * Helper to create UDP sockets.
 */
template <typename Address>
using SocketUdp = Socket<Address, protocol::Udp>;

/**
 * Helper to create UDP/IP sockets.
 */
using SocketUdpIp = Socket<address::Ip, protocol::Udp>;

#if !defined(SOCKET_NO_SSL)

/**
 * Helper to create OpenSSL TCP sockets.
 */
template <typename Address>
using SocketTls = Socket<Address, protocol::Tls>;

/**
 * Helper to create OpenSSL TCP/Ip sockets.
 */
using SocketTlsIp = Socket<address::Ip, protocol::Tls>;

#endif // !SOCKET_NO_SSL

/* }}} */

/*
 * Select wrapper
 * ------------------------------------------------------------------
 *
 * Wrapper for select(2) and other various implementations.
 */

/* {{{ Listener */

/**
 * \class ListenerStatus
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
using ListenerTable = std::map<Handle, Condition>;

/**
 * \class Select
 * \brief Implements select(2)
 *
 * This class is the fallback of any other method, it is not preferred at all for many reasons.
 */
class Select {
public:
	/**
	 * No-op, uses the ListenerTable directly.
	 */
	inline void set(const ListenerTable &, Handle, Condition, bool) noexcept {}

	/**
	 * No-op, uses the ListenerTable directly.
	 */
	inline void unset(const ListenerTable &, Handle, Condition, bool) noexcept {}

	/**
	 * Return the sockets
	 */
	std::vector<ListenerStatus> wait(const ListenerTable &table, int ms);

	/**
	 * Backend identifier
	 */
	inline const char *name() const noexcept
	{
		return "select";
	}
};

#if defined(SOCKET_HAVE_POLL)

/**
 * \class Poll
 * \brief Implements poll(2).
 *
 * Poll is widely supported and is better than select(2). It is still not the
 * best option as selecting the sockets is O(n).
 */
class Poll {
private:
	std::vector<pollfd> m_fds;

	short toPoll(Condition flags) const noexcept;
	Condition toCondition(short &event) const noexcept;

public:
	/**
	 * Set the handle.
	 */
	void set(const ListenerTable &, Handle, Condition, bool);

	/**
	 * Unset the handle.
	 */
	void unset(const ListenerTable &, Handle, Condition, bool);

	/**
	 * Wait for events.
	 */
	std::vector<ListenerStatus> wait(const ListenerTable &, int ms);

	/**
	 * Backend identifier
	 */
	inline const char *name() const noexcept
	{
		return "poll";
	}
};

#endif

#if defined(SOCKET_HAVE_EPOLL)

/**
 * \class Epoll
 * \brief Linux's epoll.
 */
class Epoll {
private:
	int m_handle;
	std::vector<epoll_event> m_events;

	Epoll(const Epoll &) = delete;
	Epoll &operator=(const Epoll &) = delete;
	Epoll(const Epoll &&) = delete;
	Epoll &operator=(const Epoll &&) = delete;

	uint32_t toEpoll(Condition flags) const noexcept;
	Condition toCondition(uint32_t events) const noexcept;
	void update(Handle sc, int op, int eflags);

public:
	/**
	 * Construct the epoll instance.
	 */
	Epoll();

	/**
	 * Close the epoll instance.
	 */
	~Epoll();

	/**
	 * Set the handle.
	 */
	void set(const ListenerTable &, Handle, Condition, bool);

	/**
	 * Unset the handle.
	 */
	void unset(const ListenerTable &, Handle, Condition, bool);

	/**
	 * Wait for events.
	 */
	std::vector<ListenerStatus> wait(const ListenerTable &, int);

	/**
	 * Backend identifier
	 */
	inline const char *name() const noexcept
	{
		return "epoll";
	}
};

#endif

#if defined(SOCKET_HAVE_KQUEUE)

/**
 * \class Kqueue
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
	Kqueue(Kqueue &&) = delete;
	Kqueue &operator=(Kqueue &&) = delete;

	void update(Handle sc, int filter, int kflags);

public:
	/**
	 * Construct the kqueue instance.
	 */
	Kqueue();

	/**
	 * Destroy the kqueue instance.
	 */
	~Kqueue();

	/**
	 * Set the handle.
	 */
	void set(const ListenerTable &, Handle, Condition, bool);

	/**
	 * Unset the handle.
	 */
	void unset(const ListenerTable &, Handle, Condition, bool);

	/**
	 * Wait for events.
	 */
	std::vector<ListenerStatus> wait(const ListenerTable &, int);

	/**
	 * Backend identifier
	 */
	inline const char *name() const noexcept
	{
		return "kqueue";
	}
};

#endif

/**
 * \class Listener
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
 *
 * To implement the backend, the following functions must be available:
 *
 * ### Set
 *
 * \code
 * void set(const ListenerTable &, Handle sc, Condition condition, bool add);
 * \endcode
 *
 * This function, takes the socket to be added and the flags. The condition is
 * always guaranteed to be correct and the function will never be called twice
 * even if the user tries to set the same flag again.
 *
 * An optional add argument is added for backends which needs to do different
 * operation depending if the socket was already set before or if it is the
 * first time (e.g EPOLL_CTL_ADD vs EPOLL_CTL_MOD for epoll(7).
 *
 * ### Unset
 *
 * \code
 * void unset(const ListenerTable &, Handle sc, Condition condition, bool remove);
 * \endcode
 *
 * Like set, this function is only called if the condition is actually set and will
 * not be called multiple times.
 *
 * Also like set, an optional remove argument is set if the socket is being
 * completely removed (e.g no more flags are set for this socket).
 *
 * ### Wait
 *
 * \code
 * std::vector<ListenerStatus> wait(const ListenerTable &, int ms);
 * \endcode
 *
 * Wait for the sockets to be ready with the specified milliseconds. Must return a list of ListenerStatus,
 * may throw any exceptions.
 *
 * ### Name
 *
 * \code
 * inline const char *name() const noexcept
 * \endcode
 *
 * Returns the backend name. Usually the class in lower case.
 */
template <typename Backend = SOCKET_DEFAULT_BACKEND>
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
		/* Invalid or useless flags */
		if (condition == Condition::None || static_cast<int>(condition) > 0x3)
			return;

		auto it = m_table.find(sc);

		/*
		 * Do not update the table if the backend failed to add
		 * or update.
		 */
		if (it == m_table.end()) {
			m_backend.set(m_table, sc, condition, true);
			m_table.emplace(sc, condition);
		} else {
			/* Remove flag if already present */
			if ((condition & Condition::Readable) == Condition::Readable &&
			    (it->second & Condition::Readable) == Condition::Readable) {
				condition &= ~(Condition::Readable);
			}
			if ((condition & Condition::Writable) == Condition::Writable &&
			    (it->second & Condition::Writable) == Condition::Writable) {
				condition &= ~(Condition::Writable);
			}

			/* Still need a call? */
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

		/* Invalid or useless flags */
		if (condition == Condition::None || static_cast<int>(condition) > 0x3 || it == m_table.end())
			return;

		/*
		 * Like set, do not update if the socket is already at the appropriate
		 * state.
		 */
		if ((condition & Condition::Readable) == Condition::Readable &&
		    (it->second & Condition::Readable) != Condition::Readable) {
			condition &= ~(Condition::Readable);
		}
		if ((condition & Condition::Writable) == Condition::Writable &&
		    (it->second & Condition::Writable) != Condition::Writable) {
			condition &= ~(Condition::Writable);
		}

		if (condition != Condition::None) {
			/* Determine if it's a complete removal */
			bool removal = ((it->second) & ~(condition)) == Condition::None;

			m_backend.unset(m_table, sc, condition, removal);

			if (removal) {
				m_table.erase(it);
			} else {
				it->second &= ~(condition);
			}
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
		while (!m_table.empty()) {
			remove(m_table.begin()->first);
		}
	}

	/**
	 * Get the number of sockets in the listener.
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
		assert(!m_table.empty());

		auto cvt = std::chrono::duration_cast<std::chrono::milliseconds>(duration);

		return m_backend.wait(m_table, cvt.count())[0];
	}

	/**
	 * Overload with milliseconds.
	 *
	 * \param timeout the optional timeout in milliseconds
	 * \return the socket ready
	 */
	inline ListenerStatus wait(int timeout = -1)
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
		assert(!m_table.empty());

		auto cvt = std::chrono::duration_cast<std::chrono::milliseconds>(duration);

		return m_backend.wait(m_table, cvt.count());
	}

	/**
	 * Overload with milliseconds.
	 *
	 * \return the socket ready
	 */
	inline std::vector<ListenerStatus> waitMultiple(int timeout = -1)
	{
		return waitMultiple(std::chrono::milliseconds(timeout));
	}
};

/* }}} */

/*
 * Callback
 * ------------------------------------------------------------------
 *
 * Function owner with tests.
 */

/* {{{ Callback */

/**
 * \class Callback
 * \brief Convenient signal owner that checks if the target is valid.
 *
 * This class also catch all errors thrown from signals to avoid interfering with our process.
 */
template <typename... Args>
class Callback : public std::function<void (Args...)> {
public:
	/**
	 * Inherited constructors.
	 */
	using std::function<void (Args...)>::function;

	/**
	 * Execute the callback only if a target is set.
	 */
	void operator()(Args... args) const
	{
		if (*this) {
			try {
				std::function<void (Args...)>::operator()(args...);
			} catch (...) {
			}
		}
	}
};

/* }}} */

/*
 * StreamConnection
 * ------------------------------------------------------------------
 *
 * Client connected on the server side.
 */

/* {{{ StreamConnection */

/**
 * \class StreamConnection
 * \brief Connected client on the server side.
 *
 * This object is created from StreamServer when a new client is connected, it is the higher
 * level object of sockets and completely asynchronous.
 */
template <typename Address, typename Protocol>
class StreamConnection {
public:
	/**
	 * Called when the output has changed.
	 */
	using WriteHandler = Callback<>;

private:
	/* Signals */
	WriteHandler m_onWrite;

	/* Sockets and output buffer */
	Socket<Address, Protocol> m_socket;
	std::string m_output;

public:
	/**
	 * Create the connection.
	 *
	 * \param s the socket
	 */
	StreamConnection(Socket<Address, Protocol> s)
		: m_socket{std::move(s)}
	{
		m_socket.set(net::option::SockBlockMode{false});
	}

	/**
	 * Access the underlying socket.
	 *
	 * \return the socket
	 * \warning use with care
	 */
	inline Socket<Address, Protocol> &socket() noexcept
	{
		return m_socket;
	}

	/**
	 * Access the current output.
	 *
	 * \return the output
	 */
	inline const std::string &output() const noexcept
	{
		return m_output;
	}

	/**
	 * Overloaded function
	 *
	 * \return the output
	 * \warning use with care, avoid modifying the output if you don't know what you're doing
	 */
	inline std::string &output() noexcept
	{
		return m_output;
	}

	/**
	 * Post some data to be sent asynchronously.
	 *
	 * \param str the data to append
	 */
	inline void send(std::string str)
	{
		m_output += str;
		m_onWrite();
	}

	/**
	 * Kill the client.
	 */
	inline void close()
	{
		m_socket.close();
	}

	/**
	 * Set the write handler, the signal is emitted when the output has changed so that the StreamServer owner
	 * knows that there are some data to send.
	 *
	 * \param handler the handler
	 * \warning you usually never need to set this yourself
	 */
	inline void setWriteHandler(WriteHandler handler)
	{
		m_onWrite = std::move(handler);
	}
};

/* }}} */

/*
 * StreamServer
 * ------------------------------------------------------------------
 *
 * Convenient stream oriented server.
 */

/* {{{ StreamServer */

/**
 * \class StreamServer
 * \brief Convenient stream server for TCP and TLS.
 *
 * This class does all the things for you as accepting new clients, listening for it and sending data. It works
 * asynchronously without blocking to let you control your process workflow.
 *
 * This class is not thread safe and you must not call any of the functions from different threads.
 */
template <typename Address, typename Protocol>
class StreamServer {
public:
	/**
	 * Handler when a new client is connected.
	 */
	using ConnectionHandler = Callback<const std::shared_ptr<StreamConnection<Address, Protocol>> &>;

	/**
	 * Handler when a client is disconnected.
	 */
	using DisconnectionHandler = Callback<const std::shared_ptr<StreamConnection<Address, Protocol>> &>;

	/**
	 * Handler when data has been received from a client.
	 */
	using ReadHandler = Callback<const std::shared_ptr<StreamConnection<Address, Protocol>> &, const std::string &>;

	/**
	 * Handler when data has been correctly sent to a client.
	 */
	using WriteHandler = Callback<const std::shared_ptr<StreamConnection<Address, Protocol>> &, const std::string &>;

	/**
	 * Handler when an error occured.
	 */
	using ErrorHandler = Callback<const Error &>;

	/**
	 * Handler when there was a timeout.
	 */
	using TimeoutHandler = Callback<>;

private:
	using ClientMap = std::map<Handle, std::shared_ptr<StreamConnection<Address, Protocol>>>;

	/* Signals */
	ConnectionHandler m_onConnection;
	DisconnectionHandler m_onDisconnection;
	ReadHandler m_onRead;
	WriteHandler m_onWrite;
	ErrorHandler m_onError;
	TimeoutHandler m_onTimeout;

	/* Sockets */
	Socket<Address, Protocol> m_master;
	Listener<> m_listener;
	ClientMap m_clients;

	/*
	 * Update flags depending on the required condition.
	 */
	void updateFlags(std::shared_ptr<StreamConnection<Address, Protocol>> &client)
	{
		assert(client->socket().action() != Action::None);

		m_listener.remove(client->socket().handle());
		m_listener.set(client->socket().handle(), client->socket().condition());
	}

	/*
	 * Continue accept process.
	 */
	template <typename AcceptCall>
	void processAccept(std::shared_ptr<StreamConnection<Address, Protocol>> &client, const AcceptCall &acceptFunc)
	{
		try {
			/* Do the accept */
			acceptFunc();

			/* 1. First remove completely the client */
			m_listener.remove(client->socket().handle());

			/* 2. If accept is not finished, wait for the appropriate condition */
			if (client->socket().state() == State::Accepted) {
				/* 3. Client is accepted, notify the user */
				m_listener.set(client->socket().handle(), Condition::Readable);
				m_onConnection(client);
			} else {
				/* Operation still in progress */
				updateFlags(client);
			}
		} catch (const Error &error) {
			m_clients.erase(client->socket().handle());
			m_listener.remove(client->socket().handle());
			m_onError(error);
		}
	}

	/*
	 * Process initial accept of master socket, this is the initial accepting process. Except on errors, the
	 * socket is stored but the user will be notified only once the socket is completely accepted.
	 */
	void processInitialAccept()
	{
		// TODO: store address too.
		std::shared_ptr<StreamConnection<Address, Protocol>> client = std::make_shared<StreamConnection<Address, Protocol>>(m_master.accept(nullptr));
		std::weak_ptr<StreamConnection<Address, Protocol>> ptr{client};

		/* 1. Register output changed to update listener */
		client->setWriteHandler([this, ptr] () {
			auto client = ptr.lock();

			/* Do not update the listener immediately if an action is pending */
			if (client && client->socket().action() == Action::None && !client->output().empty()) {
				m_listener.set(client->socket().handle(), Condition::Writable);
			}
		});

		/* 2. Add the client */
		m_clients.insert(std::make_pair(client->socket().handle(), client));

		/*
		 * 2. Do an initial check to set the listener flags, at this moment the socket may or not be
		 *    completely accepted.
		 */
		processAccept(client, [&] () {});
	}

	/*
	 * Read or complete the read operation.
	 */
	void processRead(std::shared_ptr<StreamConnection<Address, Protocol>> &client)
	{
		/*
		 * Read because there is something to read or because the pending operation is
		 * read and must complete.
		 */
		auto buffer = client->socket().recv(512);

		/*
		 * Now the receive operation may be completed, in that case, two possibilities:
		 *
		 * 1. The action is set to None (completed)
		 * 2. The action is still not complete, update the flags
		 */
		if (client->socket().action() == Action::None) {
			/* Empty mean normal disconnection */
			if (buffer.empty()) {
				m_listener.remove(client->socket().handle());
				m_clients.erase(client->socket().handle());
				m_onDisconnection(client);
			} else {
				/*
				 * At this step, it is possible that we were completing a receive operation, in this
				 * case the write flag may be removed, add it if required.
				 */
				if (!client->output().empty()) {
					m_listener.set(client->socket().handle(), Condition::Writable);
				}

				m_onRead(client, buffer);
			}
		} else {
			/* Operation in progress */
			updateFlags(client);
		}
	}

	/*
	 * Flush the output buffer.
	 */
	void processWrite(std::shared_ptr<StreamConnection<Address, Protocol>> &client)
	{
		auto &output = client->output();
		auto nsent = client->socket().send(output);

		if (client->socket().action() == Action::None) {
			/* 1. Create a copy of content that has been sent */
			auto sent = output.substr(0, nsent);

			/* 2. Erase the content sent */
			output.erase(0, nsent);

			/* 3. Update listener */
			if (output.empty()) {
				m_listener.unset(client->socket().handle(), Condition::Writable);
			}

			/* 4. Notify user */
			m_onWrite(client, sent);
		} else {
			updateFlags(client);
		}
	}

	void processSync(std::shared_ptr<StreamConnection<Address, Protocol>> &client, Condition flags)
	{
		try {
			auto action = client->socket().action();

			if (action == Action::Receive ||
			    (action == Action::None && (flags & Condition::Readable) == Condition::Readable)) {
				processRead(client);
			} else if ((flags & Condition::Writable) == Condition::Writable) {
				processWrite(client);
			}
		} catch (const Error &error) {
			m_onDisconnection(client);
			m_listener.remove(client->socket().handle());
			m_clients.erase(client->socket().handle());
		}
	}

public:
	/**
	 * Create a stream server with the specified address to bind.
	 *
	 * \param protocol the protocol (Tcp or Tls)
	 * \param address the address to bind
	 * \param max the max number to listen
	 * \throw Error on errors
	 */
	StreamServer(Protocol protocol, const Address &address, int max = 128)
		: m_master{std::move(protocol), address}
	{
		// TODO: m_onError
		m_master.set(SOL_SOCKET, SO_REUSEADDR, 1);
		m_master.bind(address);
		m_master.listen(max);
		m_listener.set(m_master.handle(), Condition::Readable);
	}

	/**
	 * Set the connection handler, called when a new client is connected.
	 *
	 * \param handler the handler
	 */
	inline void setConnectionHandler(ConnectionHandler handler)
	{
		m_onConnection = std::move(handler);
	}

	/**
	 * Set the disconnection handler, called when a client died.
	 *
	 * \param handler the handler
	 */
	inline void setDisconnectionHandler(DisconnectionHandler handler)
	{
		m_onDisconnection = std::move(handler);
	}

	/**
	 * Set the receive handler, called when a client has sent something.
	 *
	 * \param handler the handler
	 */
	inline void setReadHandler(ReadHandler handler)
	{
		m_onRead = std::move(handler);
	}

	/**
	 * Set the writing handler, called when some data has been sent to a client.
	 *
	 * \param handler the handler
	 */
	inline void setWriteHandler(WriteHandler handler)
	{
		m_onWrite = std::move(handler);
	}

	/**
	 * Set the error handler, called when unrecoverable error has occured.
	 *
	 * \param handler the handler
	 */
	inline void setErrorHandler(ErrorHandler handler)
	{
		m_onError = std::move(handler);
	}

	/**
	 * Set the timeout handler, called when the selection has timeout.
	 *
	 * \param handler the handler
	 */
	inline void setTimeoutHandler(TimeoutHandler handler)
	{
		m_onTimeout = std::move(handler);
	}

	/**
	 * Poll for the next event.
	 *
	 * \param timeout the timeout (-1 for indefinitely)
	 * \throw Error on errors
	 */
	void poll(int timeout = -1)
	{
		try {
			auto st = m_listener.wait(timeout);

			if (st.socket == m_master.handle()) {
				/* New client */
				processInitialAccept();
			} else {
				/* Recv / Send / Accept on a client */
				auto client = m_clients[st.socket];

				if (client->socket().state() == State::Accepted) {
					processSync(client, st.flags);
				} else {
					processAccept(client, [&] () { client->socket().accept(); });
				}
			}
		} catch (const Error &error) {
			if (error.code() == Error::Timeout) {
				m_onTimeout();
			} else {
				m_onError(error);
			}
		}
	}
};

/* }}} */

/*
 * StreamClient
 * ------------------------------------------------------------------
 */

/* {{{ StreamClient */

/**
 * \class StreamClient
 * \brief Client side connection to a server.
 *
 * This class is not thread safe and you must not call any of the functions from different threads.
 */
template <typename Address, typename Protocol>
class StreamClient {
public:
	/**
	 * Handler when connection is complete.
	 */
	using ConnectionHandler = Callback<>;

	/**
	 * Handler when data has been received.
	 */
	using ReadHandler = Callback<const std::string &>;

	/**
	 * Handler when data has been sent correctly.
	 */
	using WriteHandler = Callback<const std::string &>;

	/**
	 * Handler when disconnected.
	 */
	using DisconnectionHandler = Callback<>;

	/**
	 * Handler on unrecoverable error.
	 */
	using ErrorHandler = Callback<const Error &>;

	/**
	 * Handler when timeout occured.
	 */
	using TimeoutHandler = Callback<>;

private:
	/* Signals */
	ConnectionHandler m_onConnection;
	ReadHandler m_onRead;
	WriteHandler m_onWrite;
	DisconnectionHandler m_onDisconnection;
	ErrorHandler m_onError;
	TimeoutHandler m_onTimeout;

	/* Socket */
	Socket<Address, Protocol> m_socket;
	Listener<> m_listener;

	/* Output buffer */
	std::string m_output;

	/*
	 * Update the flags after an uncompleted operation. This function must only be called when the operation
	 * has not complete (e.g. connect, recv, send).
	 */
	void updateFlags()
	{
		assert(m_socket.action() != Action::None);

		m_listener.remove(m_socket.handle());
		m_listener.set(m_socket.handle(), m_socket.condition());
	}

	/*
	 * This is the generic connect helper, it will be used to both initiate the connection or to continue the
	 * connection process if needed.
	 *
	 * Thus the template parameter is the appropriate function to call either, m_socket.connect(address) or
	 * m_socket.connect().
	 *
	 * See poll() and connect() to understand.
	 */
	template <typename ConnectCall>
	void processConnect(const ConnectCall &connectFunc)
	{
		/* Call m_socket.connect() or m_socket.connect(address) */
		connectFunc();

		/* Remove entirely */
		m_listener.remove(m_socket.handle());

		if (m_socket.state() == State::Connected) {
			m_onConnection();
			m_listener.set(m_socket.handle(), Condition::Readable);
		} else {
			/* Connection still in progress */
			updateFlags();
		}
	}

	/*
	 * Receive or complete the receive command, if the command is not complete, the listener is updated
	 * accordingly.
	 */
	void processRead()
	{
		auto received = m_socket.recv(512);

		if (m_socket.action() == Action::None) {
			/* 0 means disconnection */
			if (received.empty()) {
				m_onDisconnection();
			} else {
				/*
				 * At this step, it is possible that we were completing a receive operation, in this
				 * case the write flag may be removed, add it if required.
				 */
				if (m_output.empty()) {
					m_listener.unset(m_socket.handle(), Condition::Writable);
				}

				m_onRead(received);
			}
		} else {
			/* Receive operation in progress */
			updateFlags();
		}
	}

	/*
	 * Send or complete the send command, if the command is not complete, the listener is updated
	 * accordingly.
	 */
	void processWrite()
	{
		auto nsent = m_socket.send(m_output);

		if (m_socket.action() == Action::None) {
			/* 1. Make a copy of what has been sent */
			auto sent = m_output.substr(0, nsent);

			/* 2. Erase sent content */
			m_output.erase(0, nsent);

			/* 3. Update flags if needed */
			if (m_output.empty()) {
				m_listener.unset(m_socket.handle(), Condition::Writable);
			}

			/* 4. Notify user */
			m_onWrite(sent);
		} else {
			/* Send operation in progress */
			updateFlags();
		}
	}

	/*
	 * Receive or send.
	 */
	void processSync(Condition condition)
	{
		if ((m_socket.action() == Action::Receive) ||
		    (m_socket.action() == Action::None && (condition & Condition::Readable) == Condition::Readable)) {
			processRead();
		} else {
			processWrite();
		}
	}

public:
	/**
	 * Create a client. The client is automatically marked as non-blocking.
	 *
	 * \param protocol the protocol (Tcp or Tls)
	 * \param address the optional address
	 * \throw net::Error on failures
	 */
	StreamClient(Protocol protocol = {}, const Address &address = {})
		: m_socket{std::move(protocol), address}
	{
		m_socket.set(net::option::SockBlockMode{false});
		m_listener.set(m_socket.handle(), Condition::Readable);
	}

	/**
	 * Set the connection handler, called when the connection succeed.
	 *
	 * \param handler the handler
	 */
	inline void setConnectionHandler(ConnectionHandler handler)
	{
		m_onConnection = std::move(handler);
	}

	/**
	 * Set the disconnection handler, called when the server closed the connection.
	 *
	 * \param handler the handler
	 */
	inline void setDisconnectionHandler(DisconnectionHandler handler)
	{
		m_onDisconnection = std::move(handler);
	}

	/**
	 * Set the read handler, called when we received something.
	 *
	 * \param handler the handler
	 */
	inline void setReadHandler(ReadHandler handler)
	{
		m_onRead = std::move(handler);
	}

	/**
	 * Set the write handler, called when we successfully sent data.
	 *
	 * \param handler the handler
	 */
	inline void setWriteHandler(WriteHandler handler)
	{
		m_onWrite = std::move(handler);
	}

	/**
	 * Set the error handler, called when unexpected error occurs.
	 *
	 * \param handler the handler
	 */
	inline void setErrorHandler(ErrorHandler handler)
	{
		m_onError = std::move(handler);
	}

	/**
	 * Connect to a server, this function may connect immediately or not in any case the connection handler
	 * will be called when the connection completed.
	 *
	 * \param address the address to connect to
	 */
	void connect(const Address &address) noexcept
	{
		assert(m_socket.state() == State::Open);

		processConnect([&] () { m_socket.connect(address); });
	}

	/**
	 * Asynchronously send data to the server.
	 *
	 * \param str the data to append
	 */
	void send(std::string str)
	{
		m_output += str;

		/* Don't update the listener if there is a pending operation */
		if (m_socket.state() == State::Connected && m_socket.action() == Action::None && !m_output.empty()) {
			m_listener.set(m_socket.handle(), Condition::Writable);
		}
	}

	/**
	 * Wait for the next event.
	 *
	 * \param timeout the time to wait in milliseconds
	 * \throw Error on errors
	 */
	void poll(int timeout = -1) noexcept
	{
		try {
			auto st = m_listener.wait(timeout);

			if (m_socket.state() != State::Connected) {
				/* Continue the connection */
				processConnect([&] () { m_socket.connect(); });
			} else {
				/* Read / Write */
				processSync(st.flags);
			}
		} catch (const Error &error) {
			if (error.code() == Error::Timeout) {
				m_onTimeout();
			} else {
				m_listener.remove(m_socket.handle());
				m_onError(error);
			}
		}
	}
};

/* }}} */

} // !net

} // !irccd

#endif // !IRCCD_SOCKETS_HPP
