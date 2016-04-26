/*
 * cmd-server-message.hpp -- implementation of server-message transport command
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

#ifndef IRCCD_CMD_SERVER_MESSAGE_HPP
#define IRCCD_CMD_SERVER_MESSAGE_HPP

/**
 * \file cmd-server-message.hpp
 * \brief Implementation of server-message transport command.
 */

#include "command.hpp"

namespace irccd {

namespace command {

/**
 * \class ServerMessage
 * \brief Implementation of server-message transport command.
 */
class ServerMessage : public RemoteCommand {
public:
	ServerMessage();

	/**
	 * \copydoc RemoteCommand::help
	 */
	std::string help() const override;

	/**
	 * \copydoc RemoteCommand::args
	 */
	std::vector<Arg> args() const override;

	/**
	 * \copydoc RemoteCommand::request
	 */
	json::Value request(Irccdctl &irccdctl, const RemoteCommandRequest &args) const override;

	/**
	 * \copydoc RemoteCommand::exec
	 */
	json::Value exec(Irccd &irccd, const json::Value &request) const override;
};

} // !command

} // !irccd

#endif // !IRCCD_CMD_SERVER_MESSAGE_HPP