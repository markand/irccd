/*
 * transport_util.hpp -- transport utilities
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

/**
 * \file transport_util.hpp
 * \brief Transport utilities.
 */

#ifndef IRCCD_DAEMON_TRANSPORT_UTIL_HPP
#define IRCCD_DAEMON_TRANSPORT_UTIL_HPP

/*
 * \file transport_util.hpp
 * \brief Transport utilities.
 */

#include <irccd/sysconfig.hpp>

#include <memory>

#include <boost/asio/io_service.hpp>

namespace irccd {

namespace ini {

class section;

} // !ini

namespace daemon {

class transport_server;

/*
 * \brief Transport utilities.
 */
namespace transport_util {

/**
 * Load a transport from a [transport] configuration section.
 *
 * \param service the IO service
 * \param sc the configuration
 * \throw transport_error on errors
 * \return the transport
 */
auto from_config(boost::asio::io_context& service,
                 const ini::section& sc) -> std::unique_ptr<transport_server>;

} // !transport_util

} // !daemon

} // !irccd

#endif // !IRCCD_DAEMON_TRANSPORT_UTIL_HPP
