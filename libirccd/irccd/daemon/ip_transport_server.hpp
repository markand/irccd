/*
 * ip_transport_server.hpp -- server side transports (TCP/IP support)
 *
 * Copyright (c) 2013-2017 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_DAEMON_IP_TRANSPORT_SERVER_HPP
#define IRCCD_DAEMON_IP_TRANSPORT_SERVER_HPP

/**
 * \file ip_transport_server.hpp
 * \brief Server side transports (TCP/IP support).
 */

#include "basic_transport_server.hpp"

namespace irccd {

/**
 * Convenient type for IP/TCP
 */
using ip_transport_server = basic_transport_server<boost::asio::ip::tcp>;

} // !irccd

#endif // !IRCCD_DAEMON_IP_TRANSPORT_SERVER_HPP
