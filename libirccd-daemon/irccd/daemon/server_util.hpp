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
#include <string_view>

#include <boost/asio/io_service.hpp>

#include <json.hpp>

namespace irccd {

namespace ini {

class section;

} // !ini

class config;

namespace daemon {

class server;

/**
 * \brief Server utilities.
 */
namespace server_util {

/**
 * \brief Pack a message and its type
 *
 * On channels and queries, you may have a special command or a standard message
 * depending on the beginning of the message.
 *
 * Example: `!reminder help' may invoke the command event if a plugin reminder
 * exists.
 */
struct message_type {
	/**
	 * \brief Describe which type of message has been received.
	 */
	enum kind {
		is_command,     //!< special command
		is_message      //!< standard message
	};

	/**
	 * Message kind.
	 */
	kind type;

	/**
	 * Message content.
	 */
	std::string message;

	/**
	 * Parse IRC message and determine if it's a command or a simple message.
	 *
	 * If it's a command, the plugin invocation command is removed from the
	 * original message, otherwise it is copied verbatime.
	 *
	 * \param message the message line
	 * \param cchar the command char (e.g '!')
	 * \param plugin the plugin name
	 * \return the pair
	 */
	static auto parse(std::string_view message,
	                  std::string_view cchar,
	                  std::string_view plugin) -> message_type;
};

/**
 * Convert a JSON object to a server.
 *
 * \param service the io service
 * \param object the object
 * \return the server
 * \throw server_error on errors
 */
auto from_json(boost::asio::io_service& service,
               const nlohmann::json& object) -> std::shared_ptr<server>;

/**
 * Convert a INI section to a server.
 *
 * \param service the io service
 * \param sc the server section
 * \return the server
 * \throw server_error on errors
 */
auto from_config(boost::asio::io_service& service,
                 const ini::section& sc) -> std::shared_ptr<server>;

} // !server_util

} // !daemon

} // !irccd

#endif // !IRCCD_DAEMON_SERVER_UTIL_HPP
