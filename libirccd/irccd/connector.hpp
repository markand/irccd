/*
 * connector.hpp -- abstract connection interface
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

#ifndef IRCCD_CONNECTOR_HPP
#define IRCCD_CONNECTOR_HPP

/**
 * \file connector.hpp
 * \brief Abstract connection interface.
 */

#include <cassert>
#include <functional>
#include <memory>
#include <system_error>

#include <boost/asio.hpp>

#if defined(IRCCD_HAVE_SSL)
#	include <boost/asio/ssl.hpp>
#endif

#include <boost/filesystem/path.hpp>

#include "stream.hpp"

namespace irccd {

/**
 * \brief Abstract connection interface.
 * \ingroup core-connectors
 *
 * This class is used to connect to a stream end point (usually sockets) in an
 * asynchronous manner.
 *
 * Derived class must implement non-blocking connect function.
 */
class connector {
public:
	/**
	 * \brief Connect completion handler.
	 */
	using handler = std::function<void (std::error_code, std::shared_ptr<stream>)>;

	/**
	 * Default constructor.
	 */
	connector() = default;

	/**
	 * Virtual destructor defaulted.
	 */
	virtual ~connector() = default;

	/**
	 * Start asynchronous connect.
	 *
	 * Once the client is connected, the original acceptor must be kept until it
	 * is destroyed.
	 *
	 * \pre another connect operation must not be running
	 * \pre handler != nullptr
	 * \param handler the handler
	 */
	virtual void connect(handler handler) = 0;
};

// {{{ basic_socket_connector

/**
 * \brief Provide convenient functions for connectors.
 * \ingroup core-connectors
 */
class basic_socket_connector : public connector {
protected:
	/**
	 * \brief The I/O service.
	 */
	boost::asio::io_context& service_;

public:
	/**
	 * Construct the connector
	 *
	 * \param service the service
	 */
	basic_socket_connector(boost::asio::io_context& service);

	/**
	 * Get the I/O service.
	 *
	 * \return the service
	 */
	auto get_service() const noexcept -> const boost::asio::io_context&;

	/**
	 * Overloaded function.
	 *
	 * \return the service
	 */
	auto get_service() noexcept -> boost::asio::io_context&;
};

inline basic_socket_connector::basic_socket_connector(boost::asio::io_context& service)
	: service_(service)
{
}

inline auto basic_socket_connector::get_service() const noexcept -> const boost::asio::io_context&
{
	return service_;
}

inline auto basic_socket_connector::get_service() noexcept -> boost::asio::io_context&
{
	return service_;
}

// }}}

// {{{ ip_connector

/**
 * \brief TCP/IP connector.
 * \ingroup core-connectors
 */
class ip_connector : public basic_socket_connector {
public:
	/**
	 * Underlying socket type.
	 */
	using socket_type = boost::asio::ip::tcp::socket;

private:
	boost::asio::ip::tcp::resolver resolver_;

	std::string hostname_;
	std::string port_;

	bool ipv4_;
	bool ipv6_;

#if !defined(NDEBUG)
	bool is_connecting_{false};
#endif

	template <typename Handler>
	void resolve(Handler handler);

public:
	/**
	 * Construct the TCP/IP connector.
	 *
	 * \pre at least ipv4 or ipv6 must be true
	 * \param service the I/O context
	 * \param hostname the hostname
	 * \param port the port or service name
	 * \param ipv4 enable IPv4
	 * \param ipv6 enable IPv6
	 */
	ip_connector(boost::asio::io_context& service,
	             std::string hostname,
	             std::string port,
	             bool ipv4 = true,
	             bool ipv6 = true) noexcept;

	/**
	 * Connect to the given socket.
	 *
	 * \param sc the socket type
	 * \param handler the handler
	 * \note implemented for SocketConnector concept
	 */
	template <typename Socket, typename Handler>
	void connect(Socket& sc, Handler handler);

	/**
	 * \copydoc connector::connect
	 */
	void connect(handler handler);
};

template <typename Handler>
inline void ip_connector::resolve(Handler handler)
{
	using boost::asio::ip::tcp;

	if (ipv6_ && ipv4_)
		resolver_.async_resolve(hostname_, port_, handler);
	else if (ipv6_)
		resolver_.async_resolve(tcp::v6(), hostname_, port_, handler);
	else
		resolver_.async_resolve(tcp::v4(), hostname_, port_, handler);
}

inline ip_connector::ip_connector(boost::asio::io_context& service,
                                  std::string hostname,
                                  std::string port,
                                  bool ipv4,
                                  bool ipv6) noexcept
	: basic_socket_connector(service)
	, resolver_(service)
	, hostname_(std::move(hostname))
	, port_(std::move(port))
	, ipv4_(ipv4)
	, ipv6_(ipv6)
{
	assert(!hostname_.empty());
	assert(!port_.empty());
	assert(ipv4 || ipv6);
}

template <typename Socket, typename Handler>
inline void ip_connector::connect(Socket& sc, Handler handler)
{
#if !defined(NDEBUG)
	assert(!is_connecting_);

	is_connecting_ = true;
#endif

	resolve([this, &sc, handler] (auto code, auto res) {
#if !defined(NDEBUG)
		is_connecting_ = false;
#endif
		(void)this;

		if (code) {
			handler(std::move(code));
			return;
		}

		async_connect(sc, res, [handler] (auto code, auto) {
			handler(std::move(code));
		});
	});
}

inline void ip_connector::connect(handler handler)
{
	auto stream = std::make_shared<ip_stream>(service_);

	connect(stream->get_socket(), [handler, stream] (auto code) {
		if (code)
			handler(std::move(code), nullptr);
		else
			handler(std::move(code), std::move(stream));
	});
}

// }}}

// {{{ local_connector

#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)

/**
 * \brief Unix domain connector.
 * \ingroup core-connectors
 */
class local_connector : public basic_socket_connector {
public:
	/**
	 * Underlying socket type.
	 */
	using socket_type = boost::asio::local::stream_protocol::socket;

private:
	boost::filesystem::path path_;

#if !defined(NDEBUG)
	bool is_connecting_{false};
#endif

public:
	/**
	 * Construct a local connector.
	 *
	 * \param service the service
	 * \param path the path to the file
	 */
	local_connector(boost::asio::io_context& service,
	                boost::filesystem::path path) noexcept;

	/**
	 * Connect to the given socket.
	 *
	 * \param sc the socket type
	 * \param handler the handler
	 * \note implemented for SocketConnector concept
	 */
	template <typename Socket, typename Handler>
	void connect(Socket& sc, Handler handler) noexcept;

	/**
	 * \copydoc connector::connect
	 */
	void connect(handler handler);
};

inline local_connector::local_connector(boost::asio::io_context& service,
                                        boost::filesystem::path path) noexcept
	: basic_socket_connector(service)
	, path_(std::move(path))
{
}

template <typename Socket, typename Handler>
inline void local_connector::connect(Socket& sc, Handler handler) noexcept
{
#if !defined(NDEBUG)
	assert(!is_connecting_);

	is_connecting_ = true;
#endif

	sc.async_connect({ path_.string() }, [this, handler] (auto code) {
#if !defined(NDEBUG)
		is_connecting_ = false;
#endif
		(void)this;
		handler(std::move(code));
	});
}

inline void local_connector::connect(handler handler)
{
	auto stream = std::make_shared<local_stream>(service_);

	connect(stream->get_socket(), [handler, stream] (auto code) {
		if (code)
			handler(std::move(code), nullptr);
		else
			handler(std::move(code), std::move(stream));
	});
}

#endif // !BOOST_ASIO_HAS_LOCAL_SOCKETS

// }}}

// {{{ tls_connector

#if defined(IRCCD_HAVE_SSL)

/**
 * \brief TLS/SSL connectors.
 * \ingroup core-connectors
 * \tparam SocketConnector the socket connector (e.g. ip_connector)
 */
template <typename SocketConnector>
class tls_connector : public connector {
public:
	/**
	 * \brief the underlying socket type.
	 */
	using socket_type = typename SocketConnector::socket_type;

private:
	std::shared_ptr<boost::asio::ssl::context> context_;
	SocketConnector connector_;

public:
	/**
	 * Construct a secure layer transport server.
	 *
	 * \param context the SSL context
	 * \param args the arguments to SocketConnector constructor
	 */
	template <typename... Args>
	tls_connector(boost::asio::ssl::context context, Args&&... args);

	/**
	 * \copydoc connector::connect
	 */
	void connect(handler handler) override;
};

template <typename SocketConnector>
template <typename... Args>
inline tls_connector<SocketConnector>::tls_connector(boost::asio::ssl::context context, Args&&... args)
	: context_(std::make_shared<boost::asio::ssl::context>(std::move(context)))
	, connector_(std::forward<Args>(args)...)
{
}

template <typename SocketConnector>
inline void tls_connector<SocketConnector>::connect(handler handler)
{
	using boost::asio::ssl::stream_base;

	assert(handler);

	auto stream = std::make_shared<tls_stream<socket_type>>(connector_.get_service(), context_);

	connector_.connect(stream->get_socket().lowest_layer(), [handler, stream] (auto code) {
		if (code) {
			handler(code, nullptr);
			return;
		}

		stream->get_socket().async_handshake(stream_base::client, [handler, stream] (auto code) {
			if (code)
				handler(std::move(code), nullptr);
			else
				handler(std::move(code), std::move(stream));
		});
	});
}

/**
 * \brief Convenient alias.
 */
using tls_ip_connector = tls_connector<ip_connector>;

#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)

/**
 * \brief Convenient alias.
 */
using tls_local_connector = tls_connector<local_connector>;

#endif // !BOOST_ASIO_HAS_LOCAL_SOCKETS

#endif // !IRCCD_HAVE_SSL

// }}}

} // !irccd

#endif // !IRCCD_CONNECTOR_HPP
