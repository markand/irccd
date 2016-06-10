/*
 * cmd-plugin-reload.hpp -- implementation of plugin-reload transport command
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

#ifndef IRCCD_CMD_PLUGIN_RELOAD_HPP
#define IRCCD_CMD_PLUGIN_RELOAD_HPP

/**
 * \file cmd-plugin-reload.hpp
 * \brief Implementation of plugin-reload transport command.
 */

#include "command.hpp"

namespace irccd {

namespace command {

/**
 * \class PluginReload
 * \brief Implementation of plugin-reload transport command.
 */
class PluginReload : public Command {
public:
	/**
	 * Constructor.
	 */
	IRCCD_EXPORT PluginReload();

	/**
	 * \copydoc Command::help
	 */
	IRCCD_EXPORT std::string help() const override;

	/**
	 * \copydoc Command::args
	 */
	IRCCD_EXPORT std::vector<Arg> args() const override;

	/**
	 * \copydoc Command::properties
	 */
	IRCCD_EXPORT std::vector<Property> properties() const override;

	/**
	 * \copydoc Command::exec
	 */
	IRCCD_EXPORT json::Value exec(Irccd &irccd, const json::Value &request) const override;
};

} // !command

} // !irccd

#endif // !IRCCD_CMD_PLUGIN_RELOAD_HPP
