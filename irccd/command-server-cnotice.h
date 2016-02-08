/*
 * command-server-cnotice.h -- implementation of server-cnotice transport command
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

#ifndef _IRCCD_COMMAND_SERVER_CNOTICE_H_
#define _IRCCD_COMMAND_SERVER_CNOTICE_H_

/**
 * @file command-server-cnotice.h
 * @brief Implementation of server-cnotice transport command.
 */

#include "transport-command.h"

namespace irccd {

namespace command {

/**
 * @class ServerChannelNotice
 * @brief Implementation of server-cnotice transport command.
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
class ServerChannelNotice : public TransportCommand {
public:
	/**
	 * @copydoc TransportCommand::exec
	 */
	void exec(Irccd &irccd, TransportClient &tc, const json::Value &object) const override;
};

} // !command

} // !irccd

#endif // !_IRCCD_COMMAND_SERVER_CNOTICE_H_
