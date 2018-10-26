/*
 * tls_stream.hpp -- TLS/SSL streams
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

#ifndef IRCCD_COMMON_TLS_STREAM_HPP
#define IRCCD_COMMON_TLS_STREAM_HPP

/**
 * \file tls_stream.hpp
 * \brief TLS/SSL streams.
 */

#include <irccd/sysconfig.hpp>

#if defined(IRCCD_HAVE_SSL)

#include <boost/asio/ssl.hpp>

#include "socket_stream.hpp"

namespace irccd {

/**
 * \brief TLS/SSL streams.
 * \tparam Socket the Boost.Asio compatible socket.
 */
template <typename Socket = boost::asio::ip::tcp::socket>
class tls_stream : public socket_stream<boost::asio::ssl::stream<Socket>> {
public:
	/**
	 * Constructor.
	 *
	 * \param args the arguments to boost::asio::ssl::stream<Socket>
	 */
	template <typename... Args>
	tls_stream(Args&&... args)
		: socket_stream<boost::asio::ssl::stream<Socket>>(std::forward<Args>(args)...)
	{
	}
};

} // !irccd

#endif // !IRCCD_HAVE_SSL

#endif // !IRCCD_COMMON_TLS_STREAM_HPP
