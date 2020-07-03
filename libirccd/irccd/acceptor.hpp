/*
 * acceptor.hpp -- abstract stream acceptor interface
 *
 * Copyright (c) 2013-2020 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_ACCEPTOR_HPP
#define IRCCD_ACCEPTOR_HPP

/**
 * \file acceptor.hpp
 * \brief Abstract stream acceptor interface.
 */

#include <irccd/sysconfig.hpp>

#include <cassert>
#include <functional>
#include <memory>
#include <system_error>

#include <boost/asio.hpp>
#include <boost/filesystem/path.hpp>

#if defined(IRCCD_HAVE_SSL)
#	include <boost/asio/ssl.hpp>
#endif

#include "stream.hpp"

namespace irccd {

/**
 * \brief Abstract stream acceptor interface.
 * \ingroup core-acceptors
 *
 * This class is used to wait a new client in an asynchronous manner. Derived
 * classes must implement a non-blocking accept function.
 */
class acceptor {
public:
	/**
	 * \brief Accept completion handler.
	 */
	using handler = std::function<void (std::error_code, std::shared_ptr<stream>)>;

public:
	/**
	 * Default constructor.
	 */
	acceptor() = default;

	/**
	 * Virtual destructor defaulted.
	 */
	virtual ~acceptor() = default;

	/**
	 * Start asynchronous accept.
	 *
	 * Once the client is accepted, the original acceptor must be kept until it
	 * is destroyed.
	 *
	 * \pre another accept operation must not be running
	 * \pre handler != nullptr
	 * \param handler the handler
	 */
	virtual void accept(handler handler) = 0;
};

// {{{ basic_socket_acceptor

/**
 * \brief Convenient acceptor owner.
 * \ingroup core-acceptors
 */
template <typename Acceptor>
class basic_socket_acceptor : public acceptor {
public:
	/**
	 * Underlying socket type.
	 */
	using socket_type = typename Acceptor::protocol_type::socket;

private:
#if !defined(NDEBUG)
	bool is_accepting_{false};
#endif

protected:
	/**
	 * \brief The I/O context.
	 */
	boost::asio::io_context& service_;

	/**
	 * \brief The underlying acceptor.
	 */
	Acceptor acceptor_;

public:
	/**
	 * Construct a basic_socket_acceptor.
	 *
	 * \param service the I/O context
	 */
	basic_socket_acceptor(boost::asio::io_context& service);

	/**
	 * Construct a basic_socket_acceptor with a already bound native
	 * acceptor.
	 *
	 * \param service the I/O context
	 * \param acceptor the acceptor
	 */
	basic_socket_acceptor(boost::asio::io_context& service, Acceptor acceptor) noexcept;

	/**
	 * Get the I/O context.
	 *
	 * \return the context
	 */
	auto get_service() const noexcept -> const boost::asio::io_context&;

	/**
	 * Overloaded function.
	 *
	 * \return the context
	 */
	auto get_service() noexcept -> boost::asio::io_context&;

	/**
	 * Get the underlying acceptor.
	 *
	 * \return the acceptor
	 */
	auto get_acceptor() const noexcept -> const Acceptor&;

	/**
	 * Overloaded function.
	 *
	 * \return the acceptor
	 */
	auto get_acceptor() noexcept -> Acceptor&;

	/**
	 * Accept a new client.
	 *
	 * \pre another accept call must not be running
	 * \param sc the socket type
	 * \param handler the handler
	 * \note implemented for SocketAcceptor concept
	 */
	template <typename Socket, typename Handler>
	void accept(Socket& sc, Handler handler);
};

template <typename Acceptor>
inline basic_socket_acceptor<Acceptor>::basic_socket_acceptor(boost::asio::io_context& service)
	: service_(service)
	, acceptor_(service)
{
}

template <typename Acceptor>
inline basic_socket_acceptor<Acceptor>::basic_socket_acceptor(boost::asio::io_context& service, Acceptor acceptor) noexcept
	: service_(service)
	, acceptor_(std::move(acceptor))
{
}

template <typename Acceptor>
inline auto basic_socket_acceptor<Acceptor>::get_service() const noexcept -> const boost::asio::io_context&
{
	return service_;
}

template <typename Acceptor>
inline auto basic_socket_acceptor<Acceptor>::get_service() noexcept -> boost::asio::io_context&
{
	return service_;
}

template <typename Acceptor>
inline auto basic_socket_acceptor<Acceptor>::get_acceptor() const noexcept -> const Acceptor&
{
	return acceptor_;
}

template <typename Acceptor>
inline auto basic_socket_acceptor<Acceptor>::get_acceptor() noexcept -> Acceptor&
{
	return acceptor_;
}

template <typename Acceptor>
template <typename Socket, typename Handler>
inline void basic_socket_acceptor<Acceptor>::accept(Socket& sc, Handler handler)
{
#if !defined(NDEBUG)
	assert(!is_accepting_);

	is_accepting_ = true;
#endif

	assert(acceptor_.is_open());

	acceptor_.async_accept(sc, [this, handler] (auto code) {
#if !defined(NDEBUG)
		is_accepting_ = false;
#endif
		(void)this;
		handler(std::move(code));
	});
}

// }}}

// {{{ ip_acceptor

/**
 * \brief TCP/IP acceptor.
 * \ingroup core-acceptors
 */
class ip_acceptor : public basic_socket_acceptor<boost::asio::ip::tcp::acceptor> {
private:
	void open(bool ipv6);
	void set(bool ipv4, bool ipv6);
	void bind(const std::string& address, std::uint16_t port, bool ipv6);

public:
	/**
	 * Construct a TCP/IP acceptor.
	 *
	 * If both ipv4 and ipv6 are set, the acceptor will listen on the two
	 * protocols.
	 *
	 * To listen to any address, you can use "*" as address argument.
	 *
	 * \pre at least ipv4 or ipv6 must be true
	 * \param service the I/O service
	 * \param address the address to bind or * for any
	 * \param port the port number
	 * \param ipv4 enable ipv4
	 * \param ipv6 enable ipv6
	 */
	ip_acceptor(boost::asio::io_context& service,
	            std::string address,
	            std::uint16_t port,
	            bool ipv4 = true,
	            bool ipv6 = true);

	/**
	 * Inherited constructors.
	 */
	using basic_socket_acceptor::basic_socket_acceptor;

	/**
	 * Inherited functions.
	 */
	using basic_socket_acceptor::accept;

	/**
	 * \copydoc acceptor::accept
	 */
	void accept(handler handler) override;
};

inline void ip_acceptor::open(bool ipv6)
{
	using boost::asio::ip::tcp;

	if (ipv6)
		acceptor_.open(tcp::v6());
	else
		acceptor_.open(tcp::v4());
}

inline void ip_acceptor::set(bool ipv4, bool ipv6)
{
	using boost::asio::socket_base;
	using boost::asio::ip::v6_only;

	if (ipv6)
		acceptor_.set_option(v6_only(!ipv4));

	acceptor_.set_option(socket_base::reuse_address(true));
}

inline void ip_acceptor::bind(const std::string& address, std::uint16_t port, bool ipv6)
{
	using boost::asio::ip::make_address_v4;
	using boost::asio::ip::make_address_v6;
	using boost::asio::ip::tcp;

	tcp::endpoint ep;

	if (address == "*")
		ep = tcp::endpoint(ipv6 ? tcp::v6() : tcp::v4(), port);
	else if (ipv6)
		ep = tcp::endpoint(make_address_v6(address), port);
	else
		ep = tcp::endpoint(make_address_v4(address), port);

	acceptor_.bind(ep);
	acceptor_.listen();
}

inline ip_acceptor::ip_acceptor(boost::asio::io_context& service,
                                std::string address,
                                std::uint16_t port,
                                bool ipv4,
                                bool ipv6)
	: basic_socket_acceptor(service)
{
	assert(ipv4 || ipv6);

	open(ipv6);
	set(ipv4, ipv6);
	bind(address, port, ipv6);
}

inline void ip_acceptor::accept(handler handler)
{
	auto stream = std::make_shared<ip_stream>(service_);

	basic_socket_acceptor::accept(stream->get_socket(), [handler, stream] (auto code) {
		if (code)
			handler(std::move(code), nullptr);
		else
			handler(std::move(code), std::move(stream));
	});
}

// }}}

// {{{ local_acceptor

#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)

/**
 * \brief Local acceptor.
 * \ingroup core-acceptors
 * \note Only available if BOOST_ASIO_HAS_LOCAL_SOCKETS is defined
 */
class local_acceptor : public basic_socket_acceptor<boost::asio::local::stream_protocol::acceptor> {
public:
	/**
	 * Construct a local acceptor.
	 *
	 * \param service the I/O service
	 * \param path the unix socket file
	 */
	local_acceptor(boost::asio::io_context& service,
	               const boost::filesystem::path& path);

	/**
	 * Inherited constructors.
	 */
	using basic_socket_acceptor::basic_socket_acceptor;

	/**
	 * Inherited functions.
	 */
	using basic_socket_acceptor::accept;

	/**
	 * \copydoc acceptor::accept
	 */
	void accept(handler handler) override;
};

inline local_acceptor::local_acceptor(boost::asio::io_context& service,
                                      const boost::filesystem::path& path)
	: basic_socket_acceptor(service)
{
	using boost::asio::socket_base;

	std::remove(path.string().c_str());

	acceptor_.open();
	acceptor_.set_option(socket_base::reuse_address(true));
	acceptor_.bind({ path.string() });
	acceptor_.listen();
}

inline void local_acceptor::accept(handler handler)
{
	auto stream = std::make_shared<local_stream>(service_);

	basic_socket_acceptor::accept(stream->get_socket(), [handler, stream] (auto code) {
		if (code)
			handler(std::move(code), nullptr);
		else
			handler(std::move(code), std::move(stream));
	});
}

#endif

// }}}

// {{{ tls_acceptor

#if defined(IRCCD_HAVE_SSL)

/**
 * \brief TLS/SSL acceptors.
 * \ingroup core-acceptors
 * \tparam SocketAcceptor the socket connector (e.g. ip_acceptor)
 *
 * Wrap a SocketAcceptor object.
 *
 * The SocketAcceptor object must have the following types:
 *
 * ```cpp
 * using socket_type = implementation-defined
 * ```
 *
 * The following function:
 *
 * ```cpp
 * template <typename Handler>
 * void accept(socket_type& sc, Handler handler);
 *
 * auto get_context() -> boost::asio::io_context&
 * ```
 *
 * The Handler callback must have the signature
 * `void f(const std::error_code&)`.
 */
template <typename SocketAcceptor>
class tls_acceptor : public acceptor {
private:
	using socket_type = typename SocketAcceptor::socket_type;

	std::shared_ptr<boost::asio::ssl::context> context_;
	SocketAcceptor acceptor_;

public:
	/**
	 * Construct a secure layer transport server.
	 *
	 * \param context the SSL context
	 * \param args the arguments to SocketAcceptor constructor
	 */
	template <typename... Args>
	tls_acceptor(boost::asio::ssl::context context, Args&&... args);

	/**
	 * \copydoc acceptor::accept
	 */
	void accept(handler handler) override;
};

template <typename SocketAcceptor>
template <typename... Args>
inline tls_acceptor<SocketAcceptor>::tls_acceptor(boost::asio::ssl::context context, Args&&... args)
	: context_(std::make_shared<boost::asio::ssl::context>(std::move(context)))
	, acceptor_(std::forward<Args>(args)...)
{
}

template <typename SocketAcceptor>
inline void tls_acceptor<SocketAcceptor>::accept(handler handler)
{
	auto client = std::make_shared<tls_stream<socket_type>>(acceptor_.get_service(), context_);

	acceptor_.accept(client->get_socket().lowest_layer(), [handler, client] (auto code) {
		using boost::asio::ssl::stream_base;

		if (code) {
			handler(std::move(code), nullptr);
			return;
		}

		client->get_socket().async_handshake(stream_base::server, [handler, client] (auto code) {
			if (code)
				handler(std::move(code), nullptr);
			else
				handler(std::move(code), std::move(client));
		});
	});
}

/**
 * \brief Convenient alias.
 */
using tls_ip_acceptor = tls_acceptor<ip_acceptor>;

#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)

/**
 * \brief Convenient alias.
 */
using tls_local_acceptor = tls_acceptor<local_acceptor>;

#endif // !BOOST_ASIO_HAS_LOCAL_SOCKETS

#endif // !IRCCD_HAVE_SSL

// }}}

} // !irccd

#endif // !IRCCD_ACCEPTOR_HPP
