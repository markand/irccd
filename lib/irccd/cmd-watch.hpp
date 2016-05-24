/*
 * cmd-watch.hpp -- implementation of irccdctl watch
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

#ifndef IRCCD_CMD_WATCH_HPP
#define IRCCD_CMD_WATCH_HPP

/**
 * \file cmd-watch.hpp
 * \brief Implementation of irccdctl watch.
 */

#include "command.hpp"

namespace irccd {

namespace command {

/**
 * \class Watch
 * \brief Implementation of irccdctl watch.
 */
class Watch : public RemoteCommand {
public:
	IRCCD_EXPORT Watch();

	IRCCD_EXPORT std::vector<Option> options() const override;

	/**
	 * \copydoc RemoteCommand::help
	 */
	IRCCD_EXPORT std::string help() const override;

	/**
	 * \copydoc RemoteCommand::request
	 */
	IRCCD_EXPORT json::Value request(Irccdctl &irccdctl, const RemoteCommandRequest &request) const override;
};

} // !command

} // !irccd

#endif // !IRCCD_CMD_WATCH_HPP
