/*
 * socket_acceptor.hpp -- socket stream acceptor interface
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

#ifndef IRCCD_SOCKET_ACCEPTOR_HPP
#define IRCCD_SOCKET_ACCEPTOR_HPP

/**
 * \file socket_acceptor.hpp
 * \brief Socket stream acceptor interface.
 */

#include <irccd/sysconfig.hpp>

#include "acceptor.hpp"
#include "socket_stream.hpp"

namespace irccd {

/**
 * \brief Socket stream acceptor interface.
 * \tparam Protocol a Boost.Asio compatible protocol (e.g. ip::tcp)
 */
template <typename Protocol>
class socket_acceptor : public acceptor {
public:
	/**
	 * Convenient endpoint alias.
	 */
	using endpoint = typename Protocol::endpoint;

	/**
	 * Convenient acceptor alias.
	 */
	using acceptor = typename Protocol::acceptor;

	/**
	 * Convenient socket alias.
	 */
	using socket = typename Protocol::socket;

private:
	acceptor acceptor_;

#if !defined(NDEBUG)
	bool is_accepting_{false};
#endif

protected:
	/**
	 * Helper to accept on the real underlying socket.
	 *
	 * \param socket the real socket
	 * \param handler the handler
	 */
	template <typename Socket, typename Handler>
	void do_accept(Socket& socket, Handler handler);

public:
	/**
	 * Construct the socket_acceptor.
	 *
	 * \pre acceptor must be ready (is_open() returns true)
	 * \param acceptor the Boost.Asio acceptor
	 */
	socket_acceptor(acceptor acceptor) noexcept;

	/**
	 * Get the underlying acceptor.
	 *
	 * \return the acceptor
	 */
	auto get_acceptor() const noexcept -> const acceptor&;

	/**
	 * Overloaded function.
	 *
	 * \return the acceptor
	 */
	auto get_acceptor() noexcept -> acceptor&;

	/**
	 * \copydoc acceptor::accept
	 */

	void accept(handler handler) override;
};

template <typename Protocol>
template <typename Socket, typename Handler>
void socket_acceptor<Protocol>::do_accept(Socket& socket, Handler handler)
{
#if !defined(NDEBUG)
	assert(!is_accepting_);

	is_accepting_ = true;
#endif

	acceptor_.async_accept(socket, [this, handler] (auto code) {
#if !defined(NDEBUG)
		is_accepting_ = false;
#endif
		(void)this;
		handler(code);
	});
}

template <typename Protocol>
socket_acceptor<Protocol>::socket_acceptor(acceptor acceptor) noexcept
	: acceptor_(std::move(acceptor))
{
	assert(acceptor_.is_open());
}

template <typename Protocol>
auto socket_acceptor<Protocol>::get_acceptor() const noexcept -> const acceptor&
{
	return acceptor_;
}

template <typename Protocol>
auto socket_acceptor<Protocol>::get_acceptor() noexcept -> acceptor&
{
	return acceptor_;
}

template <typename Protocol>
void socket_acceptor<Protocol>::accept(handler handler)
{
	assert(handler);

	const auto client = std::make_shared<socket_stream<socket>>(acceptor_.get_io_service());

	do_accept(client->get_socket(), [client, handler] (auto code) {
		handler(std::move(code), code ? nullptr : std::move(client));
	});
}

/**
 * Convenient TCP/IP acceptor type.
 */
using ip_acceptor = socket_acceptor<boost::asio::ip::tcp>;

#if !BOOST_OS_WINDOWS

/**
 * Convenient Unix acceptor type.
 */
using local_acceptor = socket_acceptor<boost::asio::local::stream_protocol>;

#endif

} // !irccd

#endif // !IRCCD_SOCKET_ACCEPTOR_HPP
