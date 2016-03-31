/*
 * cmd-plugin-unload.h -- implementation of plugin-unload transport command
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

#ifndef IRCCD_CMD_PLUGIN_UNLOAD_H
#define IRCCD_CMD_PLUGIN_UNLOAD_H

/**
 * @file cmd-plugin-unload.h
 * @brief Implementation of plugin-unload transport command.
 */

#include "command.h"

namespace irccd {

namespace command {

/**
 * @class PluginUnload
 * @brief Implementation of plugin-unload transport command.
 */
class PluginUnload : public RemoteCommand {
public:
	/**
	 * Constructor.
	 */
	PluginUnload();

	/**
	 * @copydoc TransportCommand::help
	 */
	std::string help() const override;

	/**
	 * @copydoc TransportCommand::args
	 */
	std::vector<Arg> args() const override;

	/**
	 * @copydoc RemoteCommand::exec
	 */
	json::Value exec(Irccd &irccd, const json::Value &object) const override;
};

} // !command

} // !irccd

#endif // !IRCCD_CMD_PLUGIN_UNLOAD_H
