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

#include <cassert>

#include <format.h>

#include "config.hpp"
#include "fs.hpp"
#include "irccd.hpp"
#include "logger.hpp"
#include "path.hpp"
#include "plugin-js.hpp"
#include "rule.hpp"
#include "server.hpp"
#include "sysconfig.hpp"
#include "transport-server.hpp"
#include "util.hpp"

using namespace fmt::literals;

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

std::string get(const ini::Document &doc, const std::string &section, const std::string &key)
{
	auto its = doc.find(section);

	if (its == doc.end()) {
		return "";
	}

	auto ito = its->find(key);

	if (ito == its->end()) {
		return "";
	}

	return ito->value();
}

ServerIdentity loadIdentity(const ini::Section &sc)
{
	assert(sc.key() == "identity");
	assert(sc.contains("name") && util::isIdentifierValid(sc["name"].value()));

	ServerIdentity identity;

	// Mandatory stuff.
	identity.name = sc["name"].value();

	// Optional stuff.
	ini::Section::const_iterator it;

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

	return identity;
}

PluginConfig loadPluginConfig(const ini::Section &sc)
{
	PluginConfig config;

	for (const auto &option : sc) {
		config.emplace(option.key(), option.value());
	}

	return config;
}

std::unique_ptr<log::Interface> loadLogFile(const ini::Section &sc)
{
	/*
	 * TODO: improve that with CMake options.
	 */
#if defined(IRCCD_SYSTEM_WINDOWS)
	std::string normal = "log.txt";
	std::string errors = "errors.txt";
#else
	std::string normal = "/var/log/irccd/log.txt";
	std::string errors = "/var/log/irccd/errors.txt";
#endif

	ini::Section::const_iterator it;

	if ((it = sc.find("path-logs")) != sc.end()) {
		normal = it->value();
	}
	if ((it = sc.find("path-errors")) != sc.end()) {
		errors = it->value();
	}

	return std::make_unique<log::File>(std::move(normal), std::move(errors));
}

std::unique_ptr<log::Interface> loadLogSyslog()
{
#if defined(HAVE_SYSLOG)
	return std::make_unique<log::Syslog>();
#else
	throw std::runtime_error("logs: syslog is not available on this platform");
#endif // !HAVE_SYSLOG
}

std::shared_ptr<TransportServer> loadTransportIp(const ini::Section &sc)
{
	assert(sc.key() == "transport");

	std::shared_ptr<TransportServer> transport;
	ini::Section::const_iterator it;

	// Port
	int port;

	if ((it = sc.find("port")) == sc.cend()) {
		throw std::invalid_argument("transport: missing 'port' parameter");
	}

	try {
		port = util::toNumber<std::uint16_t>(it->value());
	} catch (const std::exception &) {
		throw std::invalid_argument("transport: invalid port number: {}"_format(it->value()));
	}

	// Address
	std::string address = "*";

	if ((it = sc.find("address")) != sc.end()) {
		address = it->value();
	}

	// Domain
	bool ipv6 = true;
	bool ipv4 = true;

	if ((it = sc.find("domain")) != sc.end()) {
		ipv6 = false;
		ipv4 = false;

		for (const auto &v : *it) {
			if (v == "ipv4") {
				ipv4 = true;
			}
			if (v == "ipv6") {
				ipv6 = true;
			}
		}
	}

	if (ipv6) {
		transport = std::make_shared<TransportServerIp>(AF_INET6, move(address), port, !ipv4);
	} else if (ipv4) {
		transport = std::make_shared<TransportServerIp>(AF_INET, move(address), port);
	} else {
		throw std::invalid_argument("transport: domain must at least have ipv4 or ipv6");
	}

	return transport;
}

std::shared_ptr<TransportServer> loadTransportUnix(const ini::Section &sc)
{
	assert(sc.key() == "transport");

#if !defined(IRCCD_SYSTEM_WINDOWS)
	ini::Section::const_iterator it = sc.find("path");

	if (it == sc.end()) {
		throw std::invalid_argument("transport: missing 'path' parameter");
	}

	return std::make_shared<TransportServerUnix>(it->value());
#else
	(void)sc;

	throw std::invalid_argument("transport: unix transport not supported on on this platform");
#endif
}

std::shared_ptr<TransportServer> loadTransport(const ini::Section &sc)
{
	assert(sc.key() == "transport");

	std::shared_ptr<TransportServer> transport;
	ini::Section::const_iterator it = sc.find("type");

	if (it == sc.end()) {
		throw std::invalid_argument("transport: missing 'type' parameter");
	}

	if (it->value() == "ip") {
		transport = loadTransportIp(sc);
	} else if (it->value() == "unix") {
		transport = loadTransportUnix(sc);
	} else {
		throw std::invalid_argument("transport: invalid type given: {}"_format(it->value()));
	}

	return transport;
}

Rule loadRule(const ini::Section &sc)
{
	assert(sc.key() == "rule");

	// Simple converter from std::vector to std::unordered_set
	auto toSet = [] (const std::vector<std::string> &v) -> std::unordered_set<std::string> {
		return std::unordered_set<std::string>(v.begin(), v.end());
	};

	RuleSet servers, channels, origins, plugins, events;
	RuleAction action = RuleAction::Accept;

	// Get the sets
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

	// Get the action
	if ((it = sc.find("action")) == sc.end()) {
		throw std::invalid_argument("rule: missing 'action'' parameter");
	}

	if (it->value() == "drop") {
		action = RuleAction::Drop;
	} else if (it->value() == "accept") {
		action = RuleAction::Accept;
	} else {
		throw std::invalid_argument("rule: invalid action given: {}"_format(it->value()));
	}

	return Rule(std::move(servers),
		    std::move(channels),
		    std::move(origins),
		    std::move(plugins),
		    std::move(events),
		    action);
}

std::shared_ptr<Server> loadServer(const ini::Section &sc, const Config &config)
{
	assert(sc.key() == "server");

	std::string name;
	ServerInfo info;
	ServerIdentity identity;
	ServerSettings settings;

	// Name
	ini::Section::const_iterator it;

	if ((it = sc.find("name")) == sc.end()) {
		throw std::invalid_argument("server: missing 'name' parameter");
	} else if (!util::isIdentifierValid(it->value())) {
		throw std::invalid_argument("server: invalid identifier: {}"_format(it->value()));
	}

	name = it->value();

	// Host
	if ((it = sc.find("host")) == sc.end()) {
		throw std::invalid_argument("server {}: missing host"_format(name));
	}

	info.host = it->value();

	// Optional identity
	if ((it = sc.find("identity")) != sc.end()) {
		identity = config.findIdentity(it->value());
	}

	// Optional port
	if ((it = sc.find("port")) != sc.end()) {
		try {
			info.port = util::toNumber<std::uint16_t>(it->value());
		} catch (const std::exception &) {
			throw std::invalid_argument("server {}: invalid number for {}: {}"_format(name, it->key(), it->value()));
		}
	}

	// Optional password
	if ((it = sc.find("password")) != sc.end()) {
		info.password = it->value();
	}

	// Optional flags
	if ((it = sc.find("ipv6")) != sc.end() && util::isBoolean(it->value())) {
		info.flags |= ServerInfo::Ipv6;
	}
	if ((it = sc.find("ssl")) != sc.end()) {
		if (util::isBoolean(it->value())) {
			info.flags |= ServerInfo::Ssl;
		}
	}
	if ((it = sc.find("ssl-verify")) != sc.end()) {
		if (util::isBoolean(it->value())) {
			info.flags |= ServerInfo::SslVerify;
		}
	}

	// Options
	if ((it = sc.find("auto-rejoin")) != sc.end() && util::isBoolean(it->value())) {
		settings.flags |= ServerSettings::AutoRejoin;
	}
	if ((it = sc.find("join-invite")) != sc.end() && util::isBoolean(it->value())) {
		settings.flags |= ServerSettings::JoinInvite;
	}

	// Channels
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

	// Reconnect and ping timeout
	try {
		if ((it = sc.find("reconnect-tries")) != sc.end()) {
			settings.reconnectTries = util::toNumber<std::int8_t>(it->value());
		}
		if ((it = sc.find("reconnect-timeout")) != sc.end()) {
			settings.reconnectDelay = util::toNumber<std::uint16_t>(it->value());
		}
		if ((it = sc.find("ping-timeout")) != sc.end()) {
			settings.pingTimeout = util::toNumber<std::uint16_t>(it->value());
		}
	} catch (const std::exception &) {
		log::warning("server {}: invalid number for {}: {}"_format(name, it->key(), it->value()));
	}

	return std::make_shared<Server>(std::move(name), std::move(info), std::move(identity), std::move(settings));
}

} // !namespace

Config Config::find()
{
	for (const auto &path : path::list(path::PathConfig)) {
		std::string fullpath = path + "irccd.conf";

		if (!fs::isReadable(fullpath)) {
			continue;
		}

		try {
			return Config(fullpath);
		} catch (const std::exception &ex) {
			throw std::runtime_error("{}: {}"_format(fullpath, ex.what()));
		}
	}

	throw std::runtime_error("no configuration file found");
}

ServerIdentity Config::findIdentity(const std::string &name) const
{
	assert(util::isIdentifierValid(name));

	for (const auto &section : m_document) {
		if (section.key() != "identity") {
			continue;
		}

		auto it = section.find("name");

		if (it == section.end()) {
			log::warning("identity: missing 'name' property");
			continue;
		}
		if (!util::isIdentifierValid(it->value())) {
			log::warning("identity: invalid identifier: {}"_format(it->value()));
			continue;
		}
		if (it->value() != name) {
			continue;
		}

		return loadIdentity(section);
	}

	return ServerIdentity();
}

PluginConfig Config::findPluginConfig(const std::string &name) const
{
	assert(util::isIdentifierValid(name));

	std::string fullname = std::string("plugin.") + name;

	for (const auto &section : m_document) {
		if (section.key() != fullname) {
			continue;
		}

		return loadPluginConfig(section);
	}

	return PluginConfig();
}

bool Config::isVerbose() const noexcept
{
	return util::isBoolean(get(m_document, "logs", "verbose"));
}

bool Config::isForeground() const noexcept
{
	return util::isBoolean(get(m_document, "general", "foreground"));
}

std::string Config::pidfile() const
{
	return get(m_document, "general", "pidfile");
}

std::string Config::uid() const
{
	return get(m_document, "general", "uid");
}

std::string Config::gid() const
{
	return get(m_document, "general", "gid");
}

void Config::loadLogs() const
{
	ini::Document::const_iterator sc = m_document.find("logs");

	if (sc == m_document.end()) {
		return;
	}

	ini::Section::const_iterator it;

	if ((it = sc->find("type")) != sc->end()) {
		std::unique_ptr<log::Interface> iface;

		/* Console is the default, no test case */
		if (it->value() == "file") {
			iface = loadLogFile(*sc);
		} else if (it->value() == "syslog") {
			iface = loadLogSyslog();
		} else {
			throw std::runtime_error("logs: unknown log type: {}"_format(it->value()));
		}

		if (iface) {
			log::setInterface(std::move(iface));
		}
	}
}

void Config::loadFormats() const
{
	ini::Document::const_iterator sc = m_document.find("format");

	if (sc == m_document.end()) {
		return;
	}

	std::unique_ptr<IrccdLogFilter> filter = std::make_unique<IrccdLogFilter>();
	ini::Section::const_iterator it;

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

std::vector<std::shared_ptr<TransportServer>> Config::loadTransports() const
{
	std::vector<std::shared_ptr<TransportServer>> transports;

	for (const auto &section : m_document) {
		if (section.key() == "transport") {
			transports.push_back(loadTransport(section));
		}
	}

	return transports;
}

std::vector<Rule> Config::loadRules() const
{
	std::vector<Rule> rules;

	for (const auto &section : m_document) {
		if (section.key() == "rule") {
			rules.push_back(loadRule(section));
		}
	}

	return rules;
}

std::vector<std::shared_ptr<Server>> Config::loadServers() const
{
	std::vector<std::shared_ptr<Server>> servers;

	for (const auto &section : m_document) {
		if (section.key() != "server") {
			continue;
		}

		try {
			servers.push_back(loadServer(section, *this));
		} catch (const std::exception &ex) {
			log::warning(ex.what());
		}
	}

	return servers;
}

std::vector<std::shared_ptr<Plugin>> Config::loadPlugins() const
{
	std::vector<std::shared_ptr<Plugin>> plugins;

	// Plugins are defined in only one section.
	auto it = m_document.find("plugins");

	if (it == m_document.end()) {
		return plugins;
	}

	for (const auto &option : *it) {
		std::string name = option.key();
		std::string path = option.value();

		try {
			log::info("plugin {}: loading"_format(name));

			if (path.empty()) {
				// plugins.push_back(Plugin::find(name, findPluginConfig(name)));
			} else {
				log::info("plugin {}: trying {}"_format(name, path));
				plugins.push_back(std::make_shared<JsPlugin>(name, path, findPluginConfig(name)));
			}
		} catch (const duk::ErrorInfo &ex) {
			log::warning("plugin {}: {}"_format(option.key(), ex.what()));
			log::warning("plugin {}: {}"_format(option.key(), ex.stack));
		} catch (const std::exception &ex) {
			log::warning("plugin {}: {}"_format(option.key(), ex.what()));
		}
	}

	return plugins;
}

} // !irccd
