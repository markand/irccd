/*
 * tls_acceptor.hpp -- TLS/SSL acceptors
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

#ifndef IRCCD_TLS_ACCEPTOR_HPP
#define IRCCD_TLS_ACCEPTOR_HPP

/**
 * \file tls_acceptor.hpp
 * \brief TLS/SSL acceptors.
 */

#include <irccd/sysconfig.hpp>

#if defined(IRCCD_HAVE_SSL)

#include "socket_acceptor.hpp"
#include "tls_stream.hpp"

namespace irccd {

/**
 * \brief TLS/SSL acceptors.
 * \tparam Protocol a Boost.Asio compatible protocol (e.g. ip::tcp)
 */
template <typename Protocol = boost::asio::ip::tcp>
class tls_acceptor : public socket_acceptor<Protocol> {
private:
	using socket = typename Protocol::socket;

	boost::asio::ssl::context context_;

public:
	/**
	 * Construct a secure layer transport server.
	 *
	 * \param context the SSL context
	 * \param args the socket_acceptor arguments
	 */
	template <typename... Args>
	tls_acceptor(boost::asio::ssl::context context, Args&&... args);

	/**
	 * \copydoc acceptor::accept
	 */
	void accept(acceptor::handler handler) override;
};

template <typename Protocol>
void tls_acceptor<Protocol>::accept(acceptor::handler handler)
{
	assert(handler);

	auto client = std::make_shared<tls_stream<socket>>(this->get_acceptor().get_io_service(), this->context_);

	socket_acceptor<Protocol>::do_accept(client->get_socket().lowest_layer(), [handler, client] (auto code) {
		using boost::asio::ssl::stream_base;

		if (code) {
			handler(code, nullptr);
			return;
		}

		client->get_socket().async_handshake(stream_base::server, [handler, client] (auto code) {
			handler(code, code ? nullptr : std::move(client));
		});
	});
}

template <typename Protocol>
template <typename... Args>
tls_acceptor<Protocol>::tls_acceptor(boost::asio::ssl::context context, Args&&... args)
	: socket_acceptor<Protocol>(std::forward<Args>(args)...)
	, context_(std::move(context))
{
}

} // !irccd

#endif // !IRCCD_HAVE_SSL

#endif // !IRCCD_TLS_ACCEPTOR_HPP
