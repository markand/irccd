/*
 * stream.hpp -- abstract stream interface
 *
 * Copyright (c) 2013-2018 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_STREAM_HPP
#define IRCCD_STREAM_HPP

/**
 * \file stream.hpp
 * \brief Abstract stream interface.
 */

#include <irccd/sysconfig.hpp>

#include <cassert>
#include <cstddef>
#include <functional>
#include <ostream>
#include <string>
#include <system_error>
#include <utility>

#include <boost/asio.hpp>

#if defined(IRCCD_HAVE_SSL)
#	include <boost/asio/ssl.hpp>
#endif

#include "json.hpp"

namespace irccd {

/**
 * \brief Abstract stream interface
 * \ingroup core-streams
 *
 * Abstract I/O interface that allows reading/writing from a stream in an
 * asynchronous manner.
 *
 * The derived classes must implement non-blocking recv and send operations.
 */
class stream {
public:
	/**
	 * \brief Read completion handler.
	 */
	using recv_handler = std::function<void (std::error_code, nlohmann::json)>;

	/**
	 * \brief Write completion handler.
	 */
	using send_handler = std::function<void (std::error_code)>;

	/**
	 * Default constructor.
	 */
	stream() = default;

	/**
	 * Virtual destructor defaulted.
	 */
	virtual ~stream() = default;

	/**
	 * Start asynchronous read.
	 *
	 * \pre another read operation must not be running
	 * \pre handler != nullptr
	 * \param handler the handler
	 */
	virtual void recv(recv_handler handler) = 0;

	/**
	 * Start asynchronous write.
	 *
	 * \pre json.is_object()
	 * \pre another write operation must not be running
	 * \pre handler != nullptr
	 * \param json the JSON message
	 * \param handler the handler
	 */
	virtual void send(const nlohmann::json& json, send_handler handler) = 0;
};

// {{{ socket_stream_base

/**
 * \brief Abstract base interface for Boost.Asio sockets.
 * \ingroup core-streams
 *
 * This class provides convenient functions for underlying sockets.
 *
 * \see basic_socket_stream
 */
class socket_stream_base : public stream {
private:
	boost::asio::streambuf input_{2048};
	boost::asio::streambuf output_;

#if !defined(NDEBUG)
	bool is_receiving_{false};
	bool is_sending_{false};
#endif

	void handle_recv(boost::system::error_code, std::size_t, recv_handler);
	void handle_send(boost::system::error_code, std::size_t, send_handler);

protected:
	/**
	 * Convenient function for receiving for the underlying socket.
	 *
	 * \param sc the socket
	 * \param handler the handler
	 */
	template <typename Socket>
	void recv(Socket& sc, recv_handler handler);

	/**
	 * Convenient function for sending for the underlying socket.
	 *
	 * \param json the JSON object
	 * \param sc the socket
	 * \param handler the handler
	 */
	template <typename Socket>
	void send(const nlohmann::json& json, Socket& sc, send_handler handler);
};

inline void socket_stream_base::handle_recv(boost::system::error_code code,
                                            std::size_t xfer,
                                            recv_handler handler)
{
#if !defined(NDEBUG)
	is_receiving_ = false;
#endif

	if (code == boost::asio::error::not_found) {
		handler(make_error_code(std::errc::argument_list_too_long), nullptr);
		return;
	}
	if (code == boost::asio::error::eof || xfer == 0) {
		handler(make_error_code(std::errc::connection_reset), nullptr);
		return;
	}
	if (code) {
		handler(std::move(code), nullptr);
		return;
	}

	// 1. Convert the buffer safely.
	std::string buffer;

	try {
		buffer = std::string(
			boost::asio::buffers_begin(input_.data()),
			boost::asio::buffers_begin(input_.data()) + xfer - /* \r\n\r\n */ 4
		);

		input_.consume(xfer);
	} catch (const std::bad_alloc&) {
		handler(make_error_code(std::errc::not_enough_memory), nullptr);
		return;
	}

	// 2. Convert to JSON.
	nlohmann::json doc;

	try {
		doc = nlohmann::json::parse(buffer);
	} catch (const std::exception&) {
		handler(make_error_code(std::errc::invalid_argument), nullptr);
		return;
	}

	if (!doc.is_object())
		handler(make_error_code(std::errc::invalid_argument), nullptr);
	else
		handler(std::error_code(), std::move(doc));
}

inline void socket_stream_base::handle_send(boost::system::error_code code,
                                            std::size_t xfer,
                                            send_handler handler)
{
#if !defined(NDEBUG)
	is_sending_ = false;
#endif

	if (code == boost::asio::error::eof || xfer == 0) {
		handler(make_error_code(std::errc::connection_reset));
		return;
	}
	else
		handler(std::move(code));
}

template <typename Socket>
inline void socket_stream_base::recv(Socket& sc, recv_handler handler)
{
#if !defined(NDEBUG)
	assert(!is_receiving_);

	is_receiving_ = true;
#endif

	assert(handler);

	async_read_until(sc, input_, "\r\n\r\n", [this, handler] (auto code, auto xfer) {
		handle_recv(code, xfer, handler);
	});
}

template <typename Socket>
inline void socket_stream_base::send(const nlohmann::json& json, Socket& sc, send_handler handler)
{
#if !defined(NDEBUG)
	assert(!is_sending_);

	is_sending_ = true;
#endif

	assert(json.is_object());
	assert(handler);

	std::ostream out(&output_);

	out << json.dump(0);
	out << "\r\n\r\n";
	out << std::flush;

	async_write(sc, output_, [this, handler] (auto code, auto xfer) {
		handle_send(code, xfer, handler);
	});
}

// }}}

// {{{ basic_socket_stream

/**
 * \brief Complete implementation for basic sockets
 * \ingroup core-streams
 * \tparam Socket Boost.Asio socket (e.g. boost::asio::ip::tcp::socket)
 */
template <typename Socket>
class basic_socket_stream : public socket_stream_base {
private:
	Socket socket_;

public:
	/**
	 * Construct a socket stream.
	 *
	 * \param service the I/O service
	 */
	basic_socket_stream(boost::asio::io_context& service);

	/**
	 * Get the underlying socket.
	 *
	 * \return the socket
	 */
	auto get_socket() const noexcept -> const Socket&;

	/**
	 * Overloaded function.
	 *
	 * \return the socket
	 */
	auto get_socket() noexcept -> Socket&;

	/**
	 * \copydoc stream::recv
	 */
	void recv(recv_handler handler) override;

	/**
	 * \copydoc stream::send
	 */
	void send(const nlohmann::json& json, send_handler handler) override;
};

template <typename Socket>
inline basic_socket_stream<Socket>::basic_socket_stream(boost::asio::io_context& ctx)
	: socket_(ctx)
{
}

template <typename Socket>
inline auto basic_socket_stream<Socket>::get_socket() const noexcept -> const Socket&
{
	return socket_;
}

template <typename Socket>
inline auto basic_socket_stream<Socket>::get_socket() noexcept -> Socket&
{
	return socket_;
}

template <typename Socket>
inline void basic_socket_stream<Socket>::recv(recv_handler handler)
{
	socket_stream_base::recv(socket_, handler);
}

template <typename Socket>
inline void basic_socket_stream<Socket>::send(const nlohmann::json& json, send_handler handler)
{
	socket_stream_base::send(json, socket_, handler);
}

// }}}

// {{{ ip_stream

/**
 * \brief Convenient alias for boost::asio::ip::tcp::socket
 * \ingroup core-streams
 */
using ip_stream = basic_socket_stream<boost::asio::ip::tcp::socket>;

// }}}

// {{{ local_stream

#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)

/**
 * \brief Convenient alias for boost::asio::local::stream_protocol::socket
 * \ingroup core-streams
 */
using local_stream = basic_socket_stream<boost::asio::local::stream_protocol::socket>;

#endif // !BOOST_ASIO_HAS_LOCAL_SOCKETS

// }}}

// {{{ tls_stream

#if defined(IRCCD_HAVE_SSL)

/**
 * \brief TLS/SSL streams.
 * \ingroup core-streams
 * \tparam Socket the Boost.Asio compatible socket.
 */
template <typename Socket>
class tls_stream : public socket_stream_base {
private:
	boost::asio::ssl::stream<Socket> socket_;
	std::shared_ptr<boost::asio::ssl::context> context_;

public:
	/**
	 * Constructor.
	 *
	 * \param service the I/O service
	 * \param ctx the shared context
	 */
	tls_stream(boost::asio::io_context& service, std::shared_ptr<boost::asio::ssl::context> ctx);

	/**
	 * Get the SSL socket.
	 *
	 * \return the socket
	 */
	auto get_socket() const noexcept -> const boost::asio::ssl::stream<Socket>&;

	/**
	 * Overloaded function.
	 *
	 * \return the socket
	 */
	auto get_socket() noexcept -> boost::asio::ssl::stream<Socket>&;

	/**
	 * \copydoc stream::recv
	 */
	void recv(recv_handler handler) override;

	/**
	 * \copydoc stream::send
	 */
	void send(const nlohmann::json& json, send_handler handler) override;
};

template <typename Socket>
inline tls_stream<Socket>::tls_stream(boost::asio::io_context& service, std::shared_ptr<boost::asio::ssl::context> ctx)
	: socket_(service, *ctx)
	, context_(std::move(ctx))
{
}

template <typename Socket>
inline auto tls_stream<Socket>::get_socket() const noexcept -> const boost::asio::ssl::stream<Socket>&
{
	return socket_;
}

template <typename Socket>
inline auto tls_stream<Socket>::get_socket() noexcept -> boost::asio::ssl::stream<Socket>&
{
	return socket_;
}

template <typename Socket>
inline void tls_stream<Socket>::recv(recv_handler handler)
{
	socket_stream_base::recv(socket_, handler);
}

template <typename Socket>
inline void tls_stream<Socket>::send(const nlohmann::json& json, send_handler handler)
{
	socket_stream_base::send(json, socket_, handler);
}

/**
 * \brief Convenient alias.
 */
using tls_ip_stream = tls_stream<boost::asio::ip::tcp::socket>;

#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)

/**
 * \brief Convenient alias.
 */
using tls_local_stream = tls_stream<boost::asio::local::stream_protocol::socket>;

#endif // !BOOST_ASIO_HAS_LOCAL_SOCKETS

#endif // !IRCCD_HAVE_SSL

// }}}

} // !irccd

#endif // !IRCCD_STREAM_HPP
