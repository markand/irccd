/*
 * application.hpp -- super base class to create irccd front ends
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

#ifndef IRCCD_APPLICATION_HPP
#define IRCCD_APPLICATION_HPP

/**
 * \file application.hpp
 * \brief Base class for irccd and irccdctl.
 */

#include <cassert>
#include <memory>
#include <unordered_map>

#include "command.hpp"
#include "sysconfig.hpp"

namespace irccd {

/**
 * Map of commands.
 */
using RemoteCommands = std::unordered_map<std::string, std::unique_ptr<RemoteCommand>>;

/**
 * \brief Base class for creating irccd front ends.
 */
class Application {
protected:
	/**
	 * Map of commands.
	 */
	RemoteCommands m_commands;

public:
	/**
	 * Create the application and fill the commands with predefined commands.
	 */
	IRCCD_EXPORT Application();

	/**
	 * Access the remote commands.
	 *
	 * \return the commands
	 */
	inline const RemoteCommands &commands() const noexcept
	{
		return m_commands;
	}

	/**
	 * Add a new command.
	 *
	 * \pre command must not be null
	 * \pre the command must not exist
	 * \param command the command
	 */
	inline void addCommand(std::unique_ptr<RemoteCommand> command)
	{
		assert(command);
		assert(m_commands.count(command->name()) == 0);

		m_commands.emplace(command->name(), std::move(command));
	}
};

} // !irccd

#endif // !_IRCCD_APPLICATION_HPP_
