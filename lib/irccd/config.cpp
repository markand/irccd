/*
 * config.cpp -- irccd configuration loader
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

#include "sysconfig.hpp"

#if defined(HAVE_GETPID)
#  include <sys/types.h>
#  include <unistd.h>
#  include <cerrno>
#  include <cstring>
#  include <fstream>
#endif

#if defined(HAVE_DAEMON)
#  include <cstdlib>
#endif

#include "config.hpp"
#include "ini.hpp"
#include "logger.hpp"
#include "path.hpp"
#include "sockets.hpp"
#include "system.hpp"
#include "util.hpp"
#include "irccd.hpp"

using namespace std;
using namespace std::string_literals;

namespace irccd {

namespace {

class IrccdLogFilter : public log::Filter {
private:
	std::string convert(const std::string &tmpl, std::string input) const
	{
		if (tmpl.empty()) {
			return input;
		}

		util::Substitution params;

		params.flags &= ~(util::Substitution::IrcAttrs);
		params.keywords.emplace("message", std::move(input));

		return util::format(tmpl, params);
	}

public:
	std::string m_debug;
	std::string m_info;
	std::string m_warning;

	std::string preDebug(std::string input) const override
	{
		return convert(m_debug, std::move(input));
	}

	std::string preInfo(std::string input) const override
	{
		return convert(m_info, std::move(input));
	}

	std::string preWarning(std::string input) const override
	{
		return convert(m_warning, std::move(input));
	}
};

} // !namespace

void Config::loadGeneral(const ini::Document &config) const
{
	ini::Document::const_iterator sc = config.find("general");
	ini::Section::const_iterator it;

#if defined(HAVE_GETPID)
	if (sc != config.end()) {
		it = sc->find("pidfile");

		if (it != sc->end() && !it->value().empty()) {
			std::string path = it->value();
			std::ofstream out(path, std::ofstream::trunc);

			if (!out) {
				log::warning() << "irccd: could not open pidfile " << path << ": " << std::strerror(errno) << std::endl;
			} else {
				log::debug() << "irccd: pid written in " << path << std::endl;
				out << getpid();
			}
		}
	}
#endif

#if defined(HAVE_DAEMON)
	/* CLI priority is higher */
	bool daemonize = m_options.count("-f") == 0 && m_options.count("--foreground") == 0;

	if (daemonize && sc != config.end()) {
		it = sc->find("foreground");

		if (it != sc->end()) {
			daemonize = !util::isBoolean(it->value());
		}
	}

	if (daemonize) {
		daemon(1, 0);
	}
#endif

	if (sc != config.end()) {
		try {
#if defined(HAVE_SETGID)
			if ((it = sc->find("gid")) != sc->end()) {
				sys::setGid(it->value());
			}
#endif
#if defined(HAVE_SETUID)
			if ((it = sc->find("uid")) != sc->end()) {
				sys::setUid(it->value());
			}
#endif
		} catch (const std::exception &ex) {
			log::warning() << "irccd: could not set " << it->key() << ": " << ex.what() << std::endl;
		}
	}
}

void Config::loadFormats(const ini::Document &config) const
{
	ini::Document::const_iterator sc = config.find("format");

	if (sc == config.end()) {
		return;
	}

	ini::Section::const_iterator it;
	std::unique_ptr<IrccdLogFilter> filter = std::make_unique<IrccdLogFilter>();

	if ((it = sc->find("debug")) != sc->cend()) {
		filter->m_debug = it->value();
	}
	if ((it = sc->find("info")) != sc->cend()) {
		filter->m_info = it->value();
	}
	if ((it = sc->find("warning")) != sc->cend()) {
		filter->m_warning = it->value();
	}

	log::setFilter(std::move(filter));
}

void Config::loadLogFile(const ini::Section &sc) const
{
	/*
	 * TODO: improve that with CMake options.
	 */
#if defined(IRCCD_SYSTEM_WINDOWS)
	string normal = "log.txt";
	string errors = "errors.txt";
#else
	string normal = "/var/log/irccd/log.txt";
	string errors = "/var/log/irccd/errors.txt";
#endif

	ini::Section::const_iterator it;

	if ((it = sc.find("path-logs")) != sc.end()) {
		normal = it->value();
	}
	if ((it = sc.find("path-errors")) != sc.end()) {
		errors = it->value();
	}

	log::setInterface(make_unique<log::File>(move(normal), move(errors)));
}

void Config::loadLogSyslog() const
{
#if defined(HAVE_SYSLOG)
	log::setInterface(make_unique<log::Syslog>());
#else
	log::warning() << "irccd: syslog is not available on this platform" << endl;
#endif // !HAVE_SYSLOG
}

void Config::loadLogs(const ini::Document &config) const
{
	ini::Document::const_iterator sc = config.find("logs");

	if (sc == config.end()) {
		return;
	}

	ini::Section::const_iterator it;

	if ((it = sc->find("verbose")) != sc->end() && m_options.count("-v") == 0 && m_options.count("--verbose")) {
		log::setVerbose(util::isBoolean(it->value()));
	}
	if ((it = sc->find("type")) != sc->end()) {
		/* Console is the default, no test case */
		if (it->value() == "file") {
			loadLogFile(*sc);
		} else if (it->value() == "syslog") {
			loadLogSyslog();
		} else {
			log::warning() << "irccd: unknown log type: " << it->value() << std::endl;
		}
	}
}

void Config::loadPlugins(Irccd &irccd, const ini::Section &sc) const
{
#if defined(WITH_JS)
	for (const ini::Option &option : sc) {
		try {
			if (option.value().empty()) {
				irccd.loadPlugin(option.key(), option.key(), true);
			} else {
				irccd.loadPlugin(option.key(), option.value(), false);
			}
		} catch (const std::exception &ex) {
			log::warning() << "plugin " << option.key() << ": " << ex.what() << std::endl;
		}
	}
#else
	(void)irccd;
	(void)sc;
#endif
}

void Config::loadPluginConfig(Irccd &irccd, const ini::Section &sc, string name) const
{
#if defined(WITH_JS)
	PluginConfig config;

	for (const ini::Option &option : sc) {
		config.emplace(option.key(), option.value());
	}

	irccd.addPluginConfig(std::move(name), std::move(config));
#else
	(void)irccd;
	(void)sc;
	(void)name;
#endif
}

void Config::loadPlugins(Irccd &irccd, const ini::Document &config) const
{
#if defined(WITH_JS)
	std::regex regex("^plugin\\.([A-Za-z0-9-_]+)$");
	std::smatch match;

	/*
	 * Load plugin configurations before we load plugins since we use them
	 * when we load the plugin itself.
	 */
	for (const ini::Section &section : config) {
		if (regex_match(section.key(), match, regex)) {
			loadPluginConfig(irccd, section, match[1]);
		}
	}

	ini::Document::const_iterator it = config.find("plugins");

	if (it != config.end()) {
		loadPlugins(irccd, *it);
	}
#else
	(void)irccd;
	(void)config;

	log::warning() << "irccd: JavaScript disabled, ignoring plugins" << std::endl;
#endif
}

void Config::loadServer(Irccd &irccd, const ini::Section &sc) const
{
	ServerInfo info;
	ServerIdentity identity;
	ServerSettings settings;

	/* Name */
	ini::Section::const_iterator it;

	if ((it = sc.find("name")) == sc.end()) {
		throw std::invalid_argument("server: missing name");
	} else if (!util::isIdentifierValid(it->value())) {
		throw std::invalid_argument("server " + it->value() + ": name is not valid");
	} else if (irccd.hasServer(it->value())) {
		throw std::invalid_argument("server " + it->value() + ": already exists");
	}

	info.name = it->value();

	/* Host */
	if ((it = sc.find("host")) == sc.end()) {
		throw std::invalid_argument("server " + info.name + ": missing host");
	}

	info.host = it->value();

	/* Optional identity */
	if ((it = sc.find("identity")) != sc.end()) {
		identity = irccd.findIdentity(it->value());
	}

	/* Optional port */
	if ((it = sc.find("port")) != sc.end()) {
		try {
			info.port = std::stoi(it->value());
		} catch (const std::exception &) {
			throw std::invalid_argument("server " + info.name + ": invalid port number: " + it->value());
		}
	}

	/* Optional password */
	if ((it = sc.find("password")) != sc.end()) {
		info.password = it->value();
	}

	/* Optional flags */
	if ((it = sc.find("ipv6")) != sc.end() && util::isBoolean(it->value())) {
		info.flags |= ServerInfo::Ipv6;
	}
	if ((it = sc.find("ssl")) != sc.end()) {
		if (util::isBoolean(it->value())) {
#if defined(WITH_SSL)
			info.flags |= ServerInfo::Ssl;
#else
			throw std::invalid_argument("server " + info.name + ": ssl is disabled");
#endif
		}
	}
	if ((it = sc.find("ssl-verify")) != sc.end()) {
		if (util::isBoolean(it->value())) {
#if defined(WITH_SSL)
			info.flags |= ServerInfo::SslVerify;
#else
			throw std::invalid_argument("server " + info.name + ": ssl is disabled");
#endif
		}
	}
	/* Options */
	if ((it = sc.find("auto-rejoin")) != sc.end() && util::isBoolean(it->value())) {
		settings.flags |= ServerSettings::AutoRejoin;
	}
	if ((it = sc.find("join-invite")) != sc.end() && util::isBoolean(it->value())) {
		settings.flags |= ServerSettings::JoinInvite;
	}

	/* Channels */
	if ((it = sc.find("channels")) != sc.end()) {
		for (const std::string &s : *it) {
			ServerChannel channel;

			if (auto pos = s.find(":") != std::string::npos) {
				channel.name = s.substr(0, pos);
				channel.password = s.substr(pos + 1);
			} else {
				channel.name = s;
			}

			settings.channels.push_back(std::move(channel));
		}
	}
	if ((it = sc.find("command-char")) != sc.end()) {
		settings.command = it->value();
	}

	/* Reconnect */
	try {
		if ((it = sc.find("reconnect-tries")) != sc.end()) {
			settings.reconnectTries = std::stoi(it->value());
		}
		if ((it = sc.find("reconnect-timeout")) != sc.end()) {
			settings.reconnectDelay = std::stoi(it->value());
		}
		if ((it = sc.find("ping-timeout")) != sc.end()) {
			settings.pingTimeout = std::stoi(it->value());
		}
	} catch (const std::exception &) {
		throw std::invalid_argument("server " + info.name + ": invalid number for " + it->key() + ": " + it->value());
	}

	irccd.addServer(std::make_shared<Server>(std::move(info), std::move(identity), std::move(settings)));
}

void Config::loadServers(Irccd &irccd, const ini::Document &config) const
{
	for (const ini::Section &section : config) {
		if (section.key() == "server") {
			try {
				loadServer(irccd, section);
			} catch (const exception &ex) {
				log::warning() << ex.what() << endl;
			}
		}
	}
}

void Config::loadIdentity(Irccd &irccd, const ini::Section &sc) const
{
	ServerIdentity identity;
	ini::Section::const_iterator it;

	if ((it = sc.find("name")) == sc.end()) {
		throw invalid_argument("missing name");
	} else if (!util::isIdentifierValid(it->value())) {
		throw invalid_argument("identity name not valid");
	}

	identity.name = it->value();

	/* Optional stuff */
	if ((it = sc.find("username")) != sc.end()) {
		identity.username = it->value();
	}
	if ((it = sc.find("realname")) != sc.end()) {
		identity.realname = it->value();
	}
	if ((it = sc.find("nickname")) != sc.end()) {
		identity.nickname = it->value();
	}
	if ((it = sc.find("ctcp-version")) != sc.end()) {
		identity.ctcpversion = it->value();
	}

	log::debug() << "identity " << identity.name << ": ";
	log::debug() << "nickname=" << identity.nickname << ", username=" << identity.username << ", ";
	log::debug() << "realname=" << identity.realname << ", ctcp-version=" << identity.ctcpversion << endl;

	irccd.addIdentity(move(identity));
}

void Config::loadIdentities(Irccd &irccd, const ini::Document &config) const
{
	for (const ini::Section &section : config) {
		if (section.key() == "identity") {
			try {
				loadIdentity(irccd, section);
			} catch (const exception &ex) {
				log::warning() << "identity: " << ex.what() << endl;
			}
		}
	}
}

void Config::loadRule(Irccd &irccd, const ini::Section &sc) const
{
	/* Simple converter from std::vector to std::unordered_set */
	auto toSet = [] (const std::vector<std::string> &v) -> std::unordered_set<std::string> {
		return std::unordered_set<std::string>(v.begin(), v.end());
	};

	RuleSet servers, channels, origins, plugins, events;
	RuleAction action = RuleAction::Accept;

	/* Get the sets */
	ini::Section::const_iterator it;

	if ((it = sc.find("servers")) != sc.end()) {
		servers = toSet(*it);
	}
	if ((it = sc.find("channels")) != sc.end()) {
		channels = toSet(*it);
	}
	if ((it = sc.find("origins")) != sc.end()) {
		origins = toSet(*it);
	}
	if ((it = sc.find("plugins")) != sc.end()) {
		plugins = toSet(*it);
	}
	if ((it = sc.find("channels")) != sc.end()) {
		channels = toSet(*it);
	}

	/* Get the action */
	if ((it = sc.find("action")) == sc.end()) {
		throw std::invalid_argument("missing action parameter");
	}
	if (it->value() == "drop") {
		action = RuleAction::Drop;
	} else if (it->value() == "accept") {
		action = RuleAction::Accept;
	} else {
		throw std::invalid_argument("invalid action given: " + it->value());
	}

	irccd.addRule(Rule(move(servers), move(channels), move(origins), move(plugins), move(events), action));
}

void Config::loadRules(Irccd &irccd, const ini::Document &config) const
{
	for (const ini::Section &sc : config) {
		if (sc.key() == "rule") {
			try {
				loadRule(irccd, sc);
			} catch (const std::exception &ex) {
				log::warning() << "rule: " << ex.what() << std::endl;
			}
		}
	}
}

void Config::loadTransportIp(Irccd &irccd, const ini::Section &sc) const
{
	bool ipv6 = true;
	bool ipv4 = true;

	ini::Section::const_iterator it;

	/* Port */
	int port;

	if ((it = sc.find("port")) == sc.end()) {
		throw invalid_argument("missing port");
	}

	try {
		port = stoi(it->value());
	} catch (const std::exception &) {
		throw std::invalid_argument("invalid port number: " + it->value());
	}

	/* Address*/
	std::string address = "*";

	if ((it = sc.find("address")) != sc.end()) {
		address = it->value();
	}

	/* Domain */
	if ((it = sc.find("domain")) != sc.end()) {
		ipv6 = false;
		ipv4 = false;

		for (const string &v : *it) {
			if (v == "ipv4") {
				ipv4 = true;
			}
			if (v == "ipv6") {
				ipv6 = true;
			}
		}
	}

	if (ipv6) {
		irccd.addTransport(std::make_shared<TransportServerIp>(AF_INET6, move(address), port, !ipv4));
	} else if (ipv4) {
		irccd.addTransport(std::make_shared<TransportServerIp>(AF_INET, move(address), port));
	} else {
		throw std::invalid_argument("domain must at least have ipv4 or ipv6");
	}
}

void Config::loadTransportUnix(Irccd &irccd, const ini::Section &sc) const
{
#if !defined(IRCCD_SYSTEM_WINDOWS)
	/* Path */
	ini::Section::const_iterator it = sc.find("path");

	if (it == sc.end()) {
		throw std::invalid_argument("missing path parameter");
	} else {
		string path = sc["path"].value();

		irccd.addTransport(std::make_shared<TransportServerUnix>(move(path)));
	}
#else
	(void)irccd;
	(void)sc;

	throw std::invalid_argument("local transport not supported on on this platform");
#endif
}

void Config::loadTransports(Irccd &irccd, const ini::Document &config) const
{
	for (const ini::Section &sc : config) {
		if (sc.key() == "transport") {
			try {
				ini::Section::const_iterator it = sc.find("type");

				if (it == sc.end()) {
					log::warning() << "transport: missing type parameter" << std::endl;
				} else if (it->value() == "ip") {
					loadTransportIp(irccd, sc);
				} else if (it->value() == "unix") {
					loadTransportUnix(irccd, sc);
				} else {
					log::warning() << "transport: invalid type given: " << std::endl;
				}
			} catch (const net::Error &error) {
				log::warning() << "transport: " << error.function() << ": " << error.what() << std::endl;
			} catch (const exception &ex) {
				log::warning() << "transport: error: " << ex.what() << endl;
			}
		}
	}
}

bool Config::openConfig(Irccd &irccd, const string &path) const
{
	try {
		/*
		 * Order matters, take care when you change this.
		 */
		ini::Document config(ini::File{path});

		loadGeneral(config);
		loadLogs(config);
		loadFormats(config);
		loadIdentities(irccd, config);
		loadServers(irccd, config);
		loadRules(irccd, config);
		loadPlugins(irccd, config);
		loadTransports(irccd, config);
	} catch (const ini::Error &ex) {
		log::warning() << sys::programName() << ": " << path << ":" << ex.line() << ":" << ex.column() << ": " << ex.what() << std::endl;
		return false;
	} catch (const std::exception &ex) {
		log::warning() << sys::programName() << ": " << path << ": " << ex.what() << std::endl;
		return false;
	}

	return true;
}

Config::Config(parser::Result options) noexcept
	: m_options(move(options))
{
}

void Config::load(Irccd &irccd)
{
	auto it = m_options.find("-c");
	auto found = false;

	if (it != m_options.end()) {
		found = openConfig(irccd, it->second);
	} else if ((it = m_options.find("--config")) != m_options.end()) {
		found = openConfig(irccd, it->second);
	} else {
		/* Search for a configuration file */
		for (const string &path : path::list(path::PathConfig)) {
			string fullpath = path + "irccd.conf";

			log::info() << "irccd: trying " << fullpath << endl;

			if (openConfig(irccd, fullpath)) {
				found = true;
				break;
			}
		}
	}

	if (!found) {
		log::warning() << "irccd: no configuration file could be found, exiting" << endl;
		exit(1);
	}
}

} // !irccd
