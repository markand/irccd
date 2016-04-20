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
 * @file config.hpp
 * @brief Read .ini configuration file for irccd
 */

#include <irccd/options.hpp>

namespace irccd {

namespace ini {

class Document;
class Section;

} // !ini

class Irccd;

/**
 * @class Config
 * @brief Read .ini configuration file for irccd
 */
class Config {
private:
	parser::Result m_options;

	void loadGeneral(const ini::Document &config) const;
	void loadLogFile(const ini::Section &sc) const;
	void loadLogSyslog() const;
	void loadLogs(const ini::Document &config) const;
	void loadPlugins(Irccd &irccd, const ini::Section &sc) const;
	void loadPluginConfig(Irccd &irccd, const ini::Section &sc, std::string name) const;
	void loadPlugins(Irccd &irccd, const ini::Document &config) const;
	void loadServer(Irccd &irccd, const ini::Section &sc) const;
	void loadServers(Irccd &irccd, const ini::Document &config) const;
	void loadIdentity(Irccd &irccd, const ini::Section &sc) const;
	void loadIdentities(Irccd &irccd, const ini::Document &config) const;
	void loadRule(Irccd &irccd, const ini::Section &sc) const;
	void loadRules(Irccd &irccd, const ini::Document &config) const;
	void loadTransportIp(Irccd &irccd, const ini::Section &sc) const;
	void loadTransportUnix(Irccd &irccd, const ini::Section &sc) const;
	void loadTransports(Irccd &irccd, const ini::Document &config) const;
	bool openConfig(Irccd &irccd, const std::string &path) const;

public:
	/**
	 * Construct the configuration file loader. If path is empty, then the configuration file is searched through
	 * the standard directories.
	 *
	 * @param options the option parsed at command line
	 */
	Config(parser::Result options) noexcept;

	/**
	 * Load the config into irccd.
	 *
	 * @param irccd the irccd instance
	 */
	void load(Irccd &irccd);
};

} // !irccd

#endif // !IRCCD_CONFIG_HPP
