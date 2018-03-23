/*
 * server_util.hpp -- server utilities
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

#ifndef IRCCD_DAEMON_SERVER_UTIL_HPP
#define IRCCD_DAEMON_SERVER_UTIL_HPP

/**
 * \file server_util.hpp
 * \brief Server utilities.
 */

#include <memory>

#include <boost/asio/io_service.hpp>

#include <json.hpp>

#include <irccd/daemon/server.hpp>

namespace irccd {

namespace ini {

class section;

} // !ini

class config;

/**
 * \brief Server utilities.
 */
namespace server_util {

/**
 * Convert a JSON object to a server.
 *
 * \param service the io service
 * \param object the object
 * \return the server
 * \throw server_error on errors
 */
std::shared_ptr<server> from_json(boost::asio::io_service& service,
                                  const nlohmann::json& object);

/**
 * Convert a INI section to a server.
 *
 * \param service the io service
 * \param cfg the whole configuration
 * \param sc the server section
 * \return the server
 * \throw server_error on errors
 */
std::shared_ptr<server> from_config(boost::asio::io_service& service,
                                    const config& cfg,
                                    const ini::section& sc);

} // !server_util

} // !irccd

#endif // !IRCCD_DAEMON_SERVER_UTIL_HPP
