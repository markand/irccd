/*
 * local_transport_server.hpp -- server side transports (Unix support)
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

#ifndef IRCCD_LOCAL_TRANSPORT_SERVER_HPP
#define IRCCD_LOCAL_TRANSPORT_SERVER_HPP

/**
 * \file local_transport_server.hpp
 * \brief Server side transports (Unix support).
 */

#include <irccd/sysconfig.hpp>

#if !defined(IRCCD_SYSTEM_WINDOWS)

#include "basic_transport_server.hpp"

namespace irccd {

/**
 * Convenient type for UNIX local sockets.
 */
using local_transport_server = basic_transport_server<boost::asio::local::stream_protocol>;

#endif // !_WIN32

} // !irccd

#endif // !IRCCD_LOCAL_TRANSPORT_SERVER_HPP
