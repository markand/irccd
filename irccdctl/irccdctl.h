/*
 * irccdctl.h -- main irccdctl class
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

#ifndef _IRCCDCTL_H_
#define _IRCCDCTL_H_

#include <cassert>
#include <map>
#include <memory>
#include <string>

#include <options.h>

#include "alias.h"
#include "command.h"
#include "connection.h"

namespace irccd {

class Command;

namespace ini {

class Section;

} // !ini

class Irccdctl {
private:
	/* Irccd's information */
	unsigned short m_major{0};
	unsigned short m_minor{0};
	unsigned short m_patch{0};

	/* Irccd's compilation option */
	bool m_javascript{true};
	bool m_ssl{true};

	std::unique_ptr<Connection> m_connection;
	std::map<std::string, std::unique_ptr<Command>> m_commands;
	std::map<std::string, Alias> m_aliases;

	void usage() const;

	void readConnectIp(const ini::Section &sc);
	void readConnectUnix(const ini::Section &sc);
	void readConnect(const ini::Section &sc);
	void readGeneral(const ini::Section &sc);
	void readAliases(const ini::Section &sc);
	void read(const std::string &path, const parser::Result &options);

	void parseConnectIp(const parser::Result &options, bool ipv6);
	void parseConnectUnix(const parser::Result &options);
	void parseConnect(const parser::Result &options);
	parser::Result parse(int &argc, char **&argv) const;

	void exec(const Command &cmd, std::vector<std::string> args);
	void exec(const Alias &alias, std::vector<std::string> args);
	void exec(std::vector<std::string> args);

	void connect();

public:
	/**
	 * Get the connection.
	 *
	 * @return the connection
	 */
	inline Connection &connection() noexcept
	{
		return *m_connection;
	}

	/**
	 * Register a new command in irccdctl.
	 *
	 * @pre the command must not exist
	 * @param key the command name
	 */
	template <typename Cmd>
	inline void add(std::string key)
	{
		assert(m_commands.count(key) == 0);

		m_commands.emplace(std::move(key), std::make_unique<Cmd>());
	}

	/**
	 * Get all registered commands.
	 *
	 * @return the commands
	 */
	inline const std::map<std::string, std::unique_ptr<Command>> &commands() const noexcept
	{
		return m_commands;
	}

	void run(int argc, char **argv);
};

} // !irccd

#endif // !_IRCCDCTL_H_
