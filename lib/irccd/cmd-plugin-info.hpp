/*
 * cmd-plugin-info.hpp -- implementation of plugin-info command
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

#ifndef IRCCD_CMD_PLUGIN_INFO_HPP
#define IRCCD_CMD_PLUGIN_INFO_HPP

/**
 * \file cmd-plugin-info.hpp
 * \brief Implementation of plugin-info transport command.
 */

#include "command.hpp"

namespace irccd {

namespace command {

/**
 * \class PluginInfo
 * \brief Implementation of plugin-info transport command.
 */
class PluginInfo : public RemoteCommand {
public:
	/**
	 * Constructor.
	 */
	PluginInfo();

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

	/**
	 * \copydoc RemoteCommand::result
	 */
	void result(Irccdctl &irccdctl, const json::Value &response) const override;
};

} // !command

} // !irccd

#endif // !IRCCD_CMD_PLUGIN_INFO_HPP
