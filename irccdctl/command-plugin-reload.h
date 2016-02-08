/*
 * command-plugin-reload.h -- implementation of irccdctl plugin-reload
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

#ifndef _IRCCDCTL_COMMAND_PLUGIN_RELOAD_H_
#define _IRCCDCTL_COMMAND_PLUGIN_RELOAD_H_

/**
 * @file command-plugin-reload.h
 * @brief Implementation of irccdctl plugin-reload.
 */

#include "command.h"

namespace irccd {

namespace command {

/**
 * @class PluginReload
 * @brief Implementation of irccdctl plugin-reload.
 */
class PluginReload : public Command {
public:
	/**
	 * @copydoc Command::usage
	 */
	void usage(Irccdctl &irccdctl) const override;

	/**
	 * @copydoc Command::exec
	 */
	void exec(Irccdctl &irccdctl, const std::vector<std::string> &args) const override;
};

} // !command

} // !irccd

#endif // !_IRCCDCTL_COMMAND_PLUGIN_RELOAD_H_
