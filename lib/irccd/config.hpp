/*
 * config.hpp -- irccd configuration loader
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

#ifndef IRCCD_CONFIG_HPP
#define IRCCD_CONFIG_HPP

/**
 * \file config.hpp
 * \brief Read .ini configuration file for irccd
 */

#include <memory>
#include <string>
#include <vector>

#include "ini.hpp"
#include "plugin.hpp"

namespace irccd {

class Irccd;
class Rule;
class Server;
class ServerIdentity;
class TransportServer;

/**
 * \class Config
 * \brief Read .ini configuration file for irccd
 */
class Config {
private:
	std::string m_path;
	ini::Document m_document;

public:
	/**
	 * Search the configuration file into the standard defined paths.
	 *
	 * \return the config
	 * \throw std::exception on errors or if no config could be found
	 */
	static Config find();

	/**
	 * Load the configuration from the specified path.
	 *
	 * \param path the path
	 */
	inline Config(std::string path)
		: m_path(std::move(path))
		, m_document(ini::readFile(m_path))
	{
	}

	/**
	 * Get the path to the configuration file.
	 *
	 * \return the path
	 */
	inline const std::string &path() const noexcept
	{
		return m_path;
	}

	/**
	 * Find an entity if defined in the configuration file.
	 *
	 * \pre util::isValidIdentifier(name)
	 * \return default identity if cannot be found
	 */
	ServerIdentity findIdentity(const std::string &name) const;

	/**
	 * Find a plugin configuration if defined in the configuration file.
	 *
	 * \pre util::isValidIdentifier(name)
	 * \return the configuration or empty if not found
	 */
	PluginConfig findPluginConfig(const std::string &name) const;

	/**
	 * Find plugin formats if defined.
	 *
	 * \pre util::isValidIdentifier(name)
	 * \return the formats or empty one if not found
	 */
	PluginFormats findPluginFormats(const std::string &name) const;

	/**
	 * Get the path to the pidfile.
	 *
	 * \return the path or empty if not defined
	 */
	std::string pidfile() const;

	/**
	 * Get the uid.
	 *
	 * \return the uid or empty one if no one is set
	 */
	std::string uid() const;

	/**
	 * Get the gid.
	 *
	 * \return the gid or empty one if no one is set
	 */
	std::string gid() const;

	/**
	 * Check if verbosity is enabled.
	 *
	 * \return true if verbosity was requested
	 */
	bool isVerbose() const noexcept;

	/**
	 * Check if foreground is specified (= no daemonize).
	 *
	 * \return true if foreground was requested
	 */
	bool isForeground() const noexcept;

	/**
	 * Load logging interface.
	 */
	void loadLogs() const;

	/**
	 * Load formats for logging.
	 */
	void loadFormats() const;

	/**
	 * Load transports.
	 *
	 * \return the set of transports
	 */
	std::vector<std::shared_ptr<TransportServer>> loadTransports() const;

	/**
	 * Load rules.
	 *
	 * \return the rules
	 */
	std::vector<Rule> loadRules() const;

	/**
	 * Get the list of servers defined.
	 *
	 * \return the list of servers
	 */
	std::vector<std::shared_ptr<Server>> loadServers() const;

	/**
	 * Get the list of defined plugins.
	 *
	 * \param irccd the irccd instance
	 * \return the list of plugins
	 */
	void loadPlugins(Irccd &irccd) const;
};

} // !irccd

#endif // !IRCCD_CONFIG_HPP
