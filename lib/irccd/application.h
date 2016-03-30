/*
 * application.h -- super base class to create irccd front ends
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

#ifndef IRCCD_APPLICATION_H
#define IRCCD_APPLICATION_H

#include <cassert>
#include <memory>
#include <unordered_map>

#include "command.h"

namespace irccd {

using RemoteCommands = std::unordered_map<std::string, std::unique_ptr<RemoteCommand>>;

class Application {
protected:
	RemoteCommands m_commands;

public:
	Application();

	inline const RemoteCommands &commands() const noexcept
	{
		return m_commands;
	}

	inline void addCommand(std::unique_ptr<RemoteCommand> command)
	{
		assert(command);
		assert(m_commands.count(command->name()) == 0);

		m_commands.emplace(command->name(), std::move(command));
	}
};

} // !irccd

#endif // !_IRCCD_APPLICATION_H_
