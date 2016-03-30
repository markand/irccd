/*
 * command-plugin-list.h -- implementation of plugin-list transport command
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

#ifndef _IRCCD_COMMAND_PLUGIN_LIST_H_
#define _IRCCD_COMMAND_PLUGIN_LIST_H_

/**
 * @file command-plugin-list.h
 * @brief Implementation of plugin-list transport command.
 */

#include "command.h"

namespace irccd {

namespace command {

/**
 * @class PluginList
 * @brief Implementation of plugin-list transport command.
 */
class PluginList : public RemoteCommand {
public:
	/**
	 * Constructor.
	 */
	PluginList();

	/**
	 * @copydoc RemoteCommand::help
	 */
	std::string help() const override;

	/**
	 * @copydoc RemoteCommand::exec
	 */
	json::Value exec(Irccd &irccd, const json::Value &request) const override;

	/**
	 * @copydoc RemoteCommand::result
	 */
	void result(Irccdctl &irccdctl, const json::Value &object) const override;
};

} // !command

} // !irccd

#endif // !_IRCCD_COMMAND_PLUGIN_LIST_H_