/*
 * cmd-server-cnotice.hpp -- implementation of server-cnotice transport command
 *
 * Copyright (c) 2013-2016 David Demelier <markand@malikania.fr>
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

#ifndef IRCCD_CMD_SERVER_CNOTICE_HPP
#define IRCCD_CMD_SERVER_CNOTICE_HPP

/**
 * \file cmd-server-cnotice.hpp
 * \brief Implementation of server-cnotice transport command.
 */

#include "command.hpp"

namespace irccd {

namespace command {

/**
 * \class ServerChannelNotice
 * \brief Implementation of server-cnotice transport command.
 *
 * Send a channel notice to the specified channel.
 *
 * {
 *   "command": "server-cnotice",
 *   "server": "the server name",
 *   "channel": "name",
 *   "message": "the message"
 * }
 */
class ServerChannelNotice : public RemoteCommand {
public:
	/**
	 * Constructor.
	 */
	ServerChannelNotice();

	/**
	 * \copydoc RemoteCommand::help
	 */
	std::string help() const override;

	/**
	 * \copydoc RemoteCommand::args
	 */
	std::vector<Arg> args() const override;

	/**
	 * \copydoc RemoteCommand::exec
	 */
	json::Value exec(Irccd &irccd, const json::Value &request) const override;
};

} // !command

} // !irccd

#endif // !IRCCD_CMD_SERVER_CNOTICE_HPP