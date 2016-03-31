/*
 * irccd.cpp -- main irccd class
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

#include <algorithm>
#include <stdexcept>

#include "irccd.h"
#include "fs.h"
#include "logger.h"
#include "path.h"
#include "util.h"

using namespace std;
using namespace std::placeholders;
using namespace std::string_literals;

namespace irccd {

void Irccd::handleServerChannelMode(std::weak_ptr<Server> ptr, std::string origin, std::string channel, std::string mode, std::string arg)
{
	std::shared_ptr<Server> server = ptr.lock();

	if (!server)
		return;

	log::debug() << "server " << server->info().name << ": event onChannelMode:\n";
	log::debug() << "  origin: " << origin << "\n";
	log::debug() << "  channel: " << channel << "\n";
	log::debug() << "  mode: " << mode << "\n";
	log::debug() << "  argument: " << arg << std::endl;

	json::Value json = json::object({
		{ "event",	"onChannelMode"		},
		{ "server",	server->info().name	},
		{ "origin",	origin			},
		{ "channel",	channel			},
		{ "mode",	mode			},
		{ "argument",	arg			}
	});

	postServerEvent({server->info().name, origin, channel, json.toJson(0)
#if defined(WITH_JS)
		, [=] (Plugin &) -> std::string {
			return "onChannelMode";
		}
		, [=] (Plugin &plugin) {
			plugin.onChannelMode(std::move(server), std::move(origin), std::move(channel), std::move(mode), std::move(arg));
		}
#endif
	});
}

void Irccd::handleServerChannelNotice(std::weak_ptr<Server> ptr, std::string origin, std::string channel, std::string message)
{
	std::shared_ptr<Server> server = ptr.lock();

	if (!server)
		return;

	log::debug() << "server " << server->info().name << ": event onChannelNotice:\n";
	log::debug() << "  origin: " << origin << "\n";
	log::debug() << "  channel: " << channel << "\n";
	log::debug() << "  message: " << message << std::endl;

	json::Value json = json::object({
		{ "event",	"onChannelNotice"	},
		{ "server",	server->info().name	},
		{ "origin",	origin			},
		{ "channel",	channel			},
		{ "message",	message			}
	});

	postServerEvent({server->info().name, origin, channel, json.toJson(0)
#if defined(WITH_JS)
		, [=] (Plugin &) -> std::string {
			return "onChannelNotice";
		}
		, [=] (Plugin &plugin) {
			plugin.onChannelNotice(std::move(server), std::move(origin), std::move(channel), std::move(message));
		}
#endif
	});
}

void Irccd::handleServerConnect(std::weak_ptr<Server> ptr)
{
	std::shared_ptr<Server> server = ptr.lock();

	if (!server)
		return;

	log::debug() << "server " << server->info().name << ": event onConnect" << std::endl;

	json::Value json = json::object({
		{ "event",	"onConnect"		},
		{ "server",	server->info().name	}
	});

	postServerEvent({server->info().name, /* origin */ "", /* channel */ "", json.toJson(0)
#if defined(WITH_JS)
		, [=] (Plugin &) -> std::string {
			return "onConnect";
		}
		, [=] (Plugin &plugin) {
			plugin.onConnect(std::move(server));
		}
#endif
	});
}

void Irccd::handleServerInvite(std::weak_ptr<Server> ptr, std::string origin, std::string channel, std::string target)
{
	std::shared_ptr<Server> server = ptr.lock();

	if (!server)
		return;

	log::debug() << "server " << server->info().name << ": event onInvite:\n";
	log::debug() << "  origin: " << origin << "\n";
	log::debug() << "  channel: " << channel << "\n";
	log::debug() << "  target: " << target << std::endl;

	json::Value json = json::object({
		{ "event",	"onInvite"		},
		{ "server",	server->info().name	},
		{ "origin",	origin			},
		{ "channel",	channel			}
	});

	postServerEvent({server->info().name, origin, channel, json.toJson(0)
#if defined(WITH_JS)
		, [=] (Plugin &) -> std::string {
			return "onInvite";
		}
		, [=] (Plugin &plugin) {
			plugin.onInvite(std::move(server), std::move(origin), std::move(channel));
		}
#endif
	});
}

void Irccd::handleServerJoin(std::weak_ptr<Server> ptr, std::string origin, std::string channel)
{
	std::shared_ptr<Server> server = ptr.lock();

	if (!server)
		return;

	log::debug() << "server " << server->info().name << ": event onJoin:\n";
	log::debug() << "  origin: " << origin << "\n";
	log::debug() << "  channel: " << channel << std::endl;

	json::Value json = json::object({
		{ "event",	"onJoin"		},
		{ "server",	server->info().name	},
		{ "origin",	origin			},
		{ "channel",	channel			}
	});

	postServerEvent({server->info().name, origin, channel, json.toJson(0)
#if defined(WITH_JS)
		, [=] (Plugin &) -> std::string {
			return "onJoin";
		}
		, [=] (Plugin &plugin) {
			plugin.onJoin(std::move(server), std::move(origin), std::move(channel));
		}
#endif
	});
}

void Irccd::handleServerKick(std::weak_ptr<Server> ptr, std::string origin, std::string channel, std::string target, std::string reason)
{
	std::shared_ptr<Server> server = ptr.lock();

	if (!server)
		return;

	log::debug() << "server " << server->info().name << ": event onKick:\n";
	log::debug() << "  origin: " << origin << "\n";
	log::debug() << "  channel: " << channel << "\n";
	log::debug() << "  target: " << target << "\n";
	log::debug() << "  reason: " << reason << std::endl;

	json::Value json = json::object({
		{ "event",	"onKick"		},
		{ "server",	server->info().name	},
		{ "origin",	origin			},
		{ "channel",	channel			},
		{ "target",	target			},
		{ "reason",	reason			}
	});

	postServerEvent({server->info().name, origin, channel, json.toJson(0)
#if defined(WITH_JS)
		, [=] (Plugin &) -> std::string {
			return "onKick";
		}
		, [=] (Plugin &plugin) {
			plugin.onKick(std::move(server), std::move(origin), std::move(channel), std::move(target), std::move(reason));
		}
#endif
	});
}

void Irccd::handleServerMessage(std::weak_ptr<Server> ptr, std::string origin, std::string channel, std::string message)
{
	std::shared_ptr<Server> server = ptr.lock();

	if (!server)
		return;

	log::debug() << "server " << server->info().name << ": event onMessage:\n";
	log::debug() << "  origin: " << origin << "\n";
	log::debug() << "  channel: " << channel << "\n";
	log::debug() << "  message: " << message << std::endl;

	json::Value json = json::object({
		{ "event",	"onMessage"		},
		{ "server",	server->info().name	},
		{ "origin",	origin			},
		{ "channel",	channel			},
		{ "message",	message			}
	});

	postServerEvent({server->info().name, origin, channel, json.toJson(0)
#if defined(WITH_JS)
		, [=] (Plugin &plugin) -> std::string {
			return util::parseMessage(message, server->settings().command, plugin.info().name).second == util::MessageType::Command ? "onCommand" : "onMessage";
		}
		, [=] (Plugin &plugin) {
			util::MessagePair pack = util::parseMessage(message, server->settings().command, plugin.info().name);

			if (pack.second == util::MessageType::Command)
				plugin.onCommand(std::move(server), std::move(origin), std::move(channel), std::move(pack.first));
			else
				plugin.onMessage(std::move(server), std::move(origin), std::move(channel), std::move(pack.first));
		}
#endif
	});
}

void Irccd::handleServerMe(std::weak_ptr<Server> ptr, std::string origin, std::string target, std::string message)
{
	std::shared_ptr<Server> server = ptr.lock();

	if (!server)
		return;

	log::debug() << "server " << server->info().name << ": event onMe:\n";
	log::debug() << "  origin: " << origin << "\n";
	log::debug() << "  target: " << target << "\n";
	log::debug() << "  message: " << message << std::endl;

	json::Value json = json::object({
		{ "event",	"onMe"			},
		{ "server",	server->info().name	},
		{ "origin",	origin			},
		{ "target",	target			},
		{ "message",	message			}
	});

	postServerEvent({server->info().name, origin, target, json.toJson(0)
#if defined(WITH_JS)
		, [=] (Plugin &) -> std::string {
			return "onMe";
		}
		, [=] (Plugin &plugin) {
			plugin.onMe(std::move(server), std::move(origin), std::move(target), std::move(message));
		}
#endif
	});
}

void Irccd::handleServerMode(std::weak_ptr<Server> ptr, std::string origin, std::string mode)
{
	std::shared_ptr<Server> server = ptr.lock();

	if (!server)
		return;

	log::debug() << "server " << server->info().name << ": event onMode\n";
	log::debug() << "  origin: " << origin << "\n";
	log::debug() << "  mode: " << mode << std::endl;

	json::Value json = json::object({
		{ "event",	"onMode"		},
		{ "server",	server->info().name	},
		{ "origin",	origin			},
		{ "mode",	mode			}
	});

	postServerEvent({server->info().name, origin, /* channel */ "", json.toJson(0)
#if defined(WITH_JS)
		, [=] (Plugin &) -> std::string {
			return "onMode";
		}
		, [=] (Plugin &plugin) {
			plugin.onMode(std::move(server), std::move(origin), std::move(mode));
		}
#endif
	});
}

void Irccd::handleServerNames(std::weak_ptr<Server> ptr, std::string channel, std::set<std::string> nicknames)
{
	std::shared_ptr<Server> server = ptr.lock();

	if (!server)
		return;

	log::debug() << "server " << server->info().name << ": event onNames:\n";
	log::debug() << "  channel: " << channel << "\n";
	log::debug() << "  names: " << util::join(nicknames.begin(), nicknames.end(), ", ") << std::endl;

	json::Value names(std::vector<json::Value>(nicknames.begin(), nicknames.end()));
	json::Value json = json::object({
		{ "event",	"onNames"		},
		{ "server",	server->info().name	},
		{ "channel",	channel			},
		{ "names",	std::move(names)	}
	});

	postServerEvent({server->info().name, /* origin */ "", channel, json.toJson(0)
#if defined(WITH_JS)
		, [=] (Plugin &) -> std::string {
			return "onNames";
		}
		, [=] (Plugin &plugin) {
			plugin.onNames(std::move(server), std::move(channel), std::vector<std::string>(nicknames.begin(), nicknames.end()));
		}
#endif
	});
}

void Irccd::handleServerNick(std::weak_ptr<Server> ptr, std::string origin, std::string nickname)
{
	std::shared_ptr<Server> server = ptr.lock();

	if (!server)
		return;

	log::debug() << "server " << server->info().name << ": event onNick:\n";
	log::debug() << "  origin: " << origin << "\n";
	log::debug() << "  nickname: " << nickname << std::endl;

	json::Value json = json::object({
		{ "event",	"onNick"		},
		{ "server",	server->info().name	},
		{ "origin",	origin			},
		{ "nickname",	nickname		}
	});

	postServerEvent({server->info().name, origin, /* channel */ "", json.toJson(0)
#if defined(WITH_JS)
		, [=] (Plugin &) -> std::string {
			return "onNick";
		}
		, [=] (Plugin &plugin) {
			plugin.onNick(std::move(server), std::move(origin), std::move(nickname));
		}
#endif
	});
}

void Irccd::handleServerNotice(std::weak_ptr<Server> ptr, std::string origin, std::string message)
{
	std::shared_ptr<Server> server = ptr.lock();

	if (!server)
		return;

	log::debug() << "server " << server->info().name << ": event onNotice:\n";
	log::debug() << "  origin: " << origin << "\n";
	log::debug() << "  message: " << message << std::endl;

	json::Value json = json::object({
		{ "event",	"onNotice"		},
		{ "server",	server->info().name	},
		{ "origin",	origin			},
		{ "message",	message			}
	});

	postServerEvent({server->info().name, origin, /* channel */ "", json.toJson(0)
#if defined(WITH_JS)
		, [=] (Plugin &) -> std::string {
			return "onNotice";
		}
		, [=] (Plugin &plugin) {
			plugin.onNotice(std::move(server), std::move(origin), std::move(message));
		}
#endif
	});
}

void Irccd::handleServerPart(std::weak_ptr<Server> ptr, std::string origin, std::string channel, std::string reason)
{
	std::shared_ptr<Server> server = ptr.lock();

	if (!server)
		return;

	log::debug() << "server " << server->info().name << ": event onPart:\n";
	log::debug() << "  origin: " << origin << "\n";
	log::debug() << "  channel: " << channel << "\n";
	log::debug() << "  reason: " << reason << std::endl;

	json::Value json = json::object({
		{ "event",	"onPart"		},
		{ "server",	server->info().name	},
		{ "origin",	origin			},
		{ "channel",	channel			},
		{ "reason",	reason			}
	});

	postServerEvent({server->info().name, origin, channel, json.toJson(0)
#if defined(WITH_JS)
		, [=] (Plugin &) -> std::string {
			return "onPart";
		}
		, [=] (Plugin &plugin) {
			plugin.onPart(std::move(server), std::move(origin), std::move(channel), std::move(reason));
		}
#endif
	});
}

void Irccd::handleServerQuery(std::weak_ptr<Server> ptr, std::string origin, std::string message)
{
	std::shared_ptr<Server> server = ptr.lock();

	if (!server)
		return;

	log::debug() << "server " << server->info().name << ": event onQuery:\n";
	log::debug() << "  origin: " << origin << "\n";
	log::debug() << "  message: " << message << std::endl;

	json::Value json = json::object({
		{ "event",	"onQuery"		},
		{ "server",	server->info().name	},
		{ "origin",	origin			},
		{ "message",	message			}
	});

	postServerEvent({server->info().name, origin, /* channel */ "", json.toJson(0)
#if defined(WITH_JS)
		, [=] (Plugin &plugin) -> std::string {
			return util::parseMessage(message, server->settings().command, plugin.info().name).second == util::MessageType::Command ? "onQueryCommand" : "onQuery";
		}
		, [=] (Plugin &plugin) {
			util::MessagePair pack = util::parseMessage(message, server->settings().command, plugin.info().name);

			if (pack.second == util::MessageType::Command)
				plugin.onQueryCommand(std::move(server), std::move(origin), std::move(pack.first));
			else
				plugin.onQuery(std::move(server), std::move(origin), std::move(pack.first));
		}
#endif
	});
}

void Irccd::handleServerTopic(std::weak_ptr<Server> ptr, std::string origin, std::string channel, std::string topic)
{
	std::shared_ptr<Server> server = ptr.lock();

	if (!server)
		return;

	log::debug() << "server " << server->info().name << ": event onTopic:\n";
	log::debug() << "  origin: " << origin << "\n";
	log::debug() << "  channel: " << channel << "\n";
	log::debug() << "  topic: " << topic << std::endl;

	json::Value json = json::object({
		{ "event",	"onTopic"		},
		{ "server",	server->info().name	},
		{ "origin",	origin			},
		{ "channel",	channel			},
		{ "topic",	topic			}
	});

	postServerEvent({server->info().name, origin, channel, json.toJson(0)
#if defined(WITH_JS)
		, [=] (Plugin &) -> std::string {
			return "onTopic";
		}
		, [=] (Plugin &plugin) {
			plugin.onTopic(std::move(server), std::move(origin), std::move(channel), std::move(topic));
		}
#endif
	});
}

void Irccd::handleServerWhois(std::weak_ptr<Server> ptr, ServerWhois whois)
{
	std::shared_ptr<Server> server = ptr.lock();

	if (!server)
		return;

	log::debug() << "server " << server->info().name << ": event onWhois\n";
	log::debug() << "  nickname: " << whois.nick << "\n";
	log::debug() << "  username: " << whois.user << "\n";
	log::debug() << "  host: " << whois.host << "\n";
	log::debug() << "  realname: " << whois.realname << "\n";
	log::debug() << "  channels: " << util::join(whois.channels.begin(), whois.channels.end()) << std::endl;

	json::Value object = json::object({
		{ "server",	server->info().name	},
		{ "nickname",	whois.nick		},
		{ "username",	whois.user		},
		{ "host",	whois.host		},
		{ "realname",	whois.realname		}
	});

	postServerEvent({server->info().name, /* origin */ "", /* channel */ "", object.toJson(-1)
#if defined(WITH_JS)
		, [=] (Plugin &) -> std::string {
			return "onWhois";
		}
		, [=] (Plugin &plugin) {
			plugin.onWhois(std::move(server), std::move(whois));
		}
#endif
	});
}

void Irccd::handleTransportCommand(std::weak_ptr<TransportClient> ptr, const json::Value &object)
{
	assert(object.isObject());

	post([=] () {
		/* 0. Be sure the object still exists */
		auto tc = ptr.lock();

		if (!tc)
			return;

		/* 1. Check if the Json object is valid */
		auto name = object.find("command");
		if (name == object.end() || name->typeOf() != json::Type::String) {
			// TODO: send error
			log::warning() << "invalid command object" << std::endl;
			return;
		}

		/* 2. Search for a command */
		auto it = m_commands.find(name->toString());

		if (it == m_commands.end()) {
			// TODO: send error again
			log::warning() << "command does not exists" << std::endl;
			return;
		}

		/* 3. Try to execute it */
		json::Value response = json::object({});

		try {
			response = it->second->exec(*this, object);

			/* Adjust if command has returned something else */
			if (!response.isObject())
				response = json::object({});
			
			response.insert("status", true);
		} catch (const std::exception &ex) {
			response.insert("status", false);
			response.insert("error", ex.what());
		}

		/* 4. Store the command name result */
		response.insert("response", it->first);

		/* 5. Send the result */
		tc->send(response.toJson(0));
	});
}

void Irccd::handleTransportDie(std::weak_ptr<TransportClient> ptr)
{
	post([=] () {
		log::info() << "transport: client disconnected" << std::endl;

		auto tc = ptr.lock();

		if (tc)
			m_lookupTransportClients.erase(tc->handle());
	});
}

void Irccd::processIpc(fd_set &input)
{
	if (FD_ISSET(m_socketServer.handle(), &input)) {
		try {
			(void)m_socketServer.recv(8);
		} catch (const exception &) {
			// TODO: think what we can do here
		}
	}
}

void Irccd::processTransportClients(fd_set &input, fd_set &output)
{
	for (auto &pair : m_lookupTransportClients)
		pair.second->sync(input, output);
}

void Irccd::processTransportServers(fd_set &input)
{
	for (auto &pair : m_lookupTransportServers) {
		if (!FD_ISSET(pair.second->handle(), &input))
			continue;

		log::debug() << "transport: new client connected" << endl;

		std::shared_ptr<TransportClient> client = pair.second->accept();
		std::weak_ptr<TransportClient> ptr(client);

		/* Send some information */
		json::Value object = json::object({
			{ "program",	"irccd"			},
			{ "major",	IRCCD_VERSION_MAJOR	},
			{ "minor",	IRCCD_VERSION_MINOR	},
			{ "patch",	IRCCD_VERSION_PATCH	}
		});

#if defined(WITH_JS)
		object.insert("javascript", true);
#endif
#if defined(WITH_SSL)
		object.insert("ssl", true);
#endif

		client->send(object.toJson(0));

		/* Connect signals */
		client->onCommand.connect(std::bind(&Irccd::handleTransportCommand, this, ptr, _1));
		client->onDie.connect(std::bind(&Irccd::handleTransportDie, this, ptr));

		/* Register it */
		m_lookupTransportClients.emplace(client->handle(), move(client));
	}
}

void Irccd::processServers(fd_set &input, fd_set &output)
{
	for (auto &pair : m_servers)
		pair.second->sync(input, output);
}

void Irccd::process(fd_set &setinput, fd_set &setoutput)
{
	/* 1. May be IPC */
	processIpc(setinput);

	/* 2. Check for transport clients */
	processTransportClients(setinput, setoutput);

	/* 3. Check for transport servers */
	processTransportServers(setinput);

	/* 4. Check for servers */
	processServers(setinput, setoutput);
}

void Irccd::postServerEvent(ServerEvent event) noexcept
{
#if defined(WITH_JS)
	post([=] () {
		for (auto &pair : m_plugins) {
			auto name = event.name(*pair.second);
			auto allowed = Rule::solve(m_rules, event.server, event.target, event.origin, pair.first, name);

			if (!allowed) {
				log::debug() << "rule: event skipped on match" << std::endl;
				continue;
			} else {
				log::debug() << "rule: event allowed" << std::endl;
			}

			try {
				event.exec(*pair.second);
			} catch (const duk::ErrorInfo &info) {
				log::warning() << "plugin " << pair.second->info().name << ": error: " << info.what() << std::endl;

				if (!info.fileName.empty())
					log::warning() << "    " << info.fileName << ":" << info.lineNumber << std::endl;
				if (!info.stack.empty())
					log::warning() << "    " << info.stack << std::endl;
			}
		}
	});
#endif

	/* Asynchronous send */
	for (auto &pair : m_lookupTransportClients)
		pair.second->send(event.json);
}

Irccd::Irccd()
{
	/* Bind a socket to any port */
	m_socketServer.set(net::option::SockReuseAddress{true});
	m_socketServer.bind(net::address::Ip{"*", 0});
	m_socketServer.listen(1);

	/* Do the socket pair */
	m_socketClient.connect(net::address::Ip{"127.0.0.1", m_socketServer.address().port()});
	m_socketServer = m_socketServer.accept(nullptr);
	m_socketClient.set(net::option::SockBlockMode{false});
}

void Irccd::post(Event ev) noexcept
{
	std::lock_guard<mutex> lock(m_mutex);

	m_events.push_back(move(ev));

	/* Silently discard */
	try {
		m_socketClient.send(" ");
	} catch (...) {
	}
}

void Irccd::addServer(shared_ptr<Server> server) noexcept
{
	assert(m_servers.count(server->info().name) == 0);

	std::weak_ptr<Server> ptr(server);

	server->onChannelMode.connect(std::bind(&Irccd::handleServerChannelMode, this, ptr, _1, _2, _3, _4));
	server->onChannelNotice.connect(std::bind(&Irccd::handleServerChannelNotice, this, ptr, _1, _2, _3));
	server->onConnect.connect(std::bind(&Irccd::handleServerConnect, this, ptr));
	server->onInvite.connect(std::bind(&Irccd::handleServerInvite, this, ptr, _1, _2, _3));
	server->onJoin.connect(std::bind(&Irccd::handleServerJoin, this, ptr, _1, _2));
	server->onKick.connect(std::bind(&Irccd::handleServerKick, this, ptr, _1, _2, _3, _4));
	server->onMessage.connect(std::bind(&Irccd::handleServerMessage, this, ptr, _1, _2, _3));
	server->onMe.connect(std::bind(&Irccd::handleServerMe, this, ptr, _1, _2, _3));
	server->onMode.connect(std::bind(&Irccd::handleServerMode, this, ptr, _1, _2));
	server->onNames.connect(std::bind(&Irccd::handleServerNames, this, ptr, _1, _2));
	server->onNick.connect(std::bind(&Irccd::handleServerNick, this, ptr, _1, _2));
	server->onNotice.connect(std::bind(&Irccd::handleServerNotice, this, ptr, _1, _2));
	server->onPart.connect(std::bind(&Irccd::handleServerPart, this, ptr, _1, _2, _3));
	server->onQuery.connect(std::bind(&Irccd::handleServerQuery, this, ptr, _1, _2));
	server->onTopic.connect(std::bind(&Irccd::handleServerTopic, this, ptr, _1, _2, _3));
	server->onWhois.connect(std::bind(&Irccd::handleServerWhois, this, ptr, _1));
	server->onDie.connect([this, ptr] () {
		post([=] () {
			auto server = ptr.lock();

			if (server) {
				log::info() << "server " << server->info().name << ": removed" << std::endl;
				m_servers.erase(server->info().name);
			}
		});
	});

	m_servers.emplace(server->info().name, move(server));
}

std::shared_ptr<Server> Irccd::getServer(const std::string &name) const noexcept
{
	auto it = m_servers.find(name);

	if (it == m_servers.end())
		return nullptr;

	return it->second;
}

std::shared_ptr<Server> Irccd::requireServer(const std::string &name) const
{
	auto it = m_servers.find(name);

	if (it == m_servers.end())
		throw std::invalid_argument("server " + name + " not found");

	return it->second;
}

void Irccd::removeServer(const std::string &name)
{
	auto it = m_servers.find(name);

	if (it != m_servers.end()) {
		it->second->disconnect();
		m_servers.erase(it);
	}
}

void Irccd::clearServers() noexcept
{
	for (auto &pair : m_servers)
		pair.second->disconnect();

	m_servers.clear();
}

void Irccd::addTransport(std::shared_ptr<TransportServer> ts)
{
	m_lookupTransportServers.emplace(ts->handle(), ts);
}

#if defined(WITH_JS)

std::shared_ptr<Plugin> Irccd::getPlugin(const std::string &name) const noexcept
{
	auto it = m_plugins.find(name);

	if (it == m_plugins.end())
		return nullptr;

	return it->second;
}

std::shared_ptr<Plugin> Irccd::requirePlugin(const std::string &name) const
{
	auto it = m_plugins.find(name);

	if (it == m_plugins.end())
		throw std::out_of_range(std::string("plugin ") + name + " not found");

	return it->second;
}

void Irccd::addPlugin(std::shared_ptr<Plugin> plugin)
{
	std::weak_ptr<Plugin> ptr(plugin);

	plugin->onTimerSignal.connect(std::bind(&Irccd::handleTimerSignal, this, ptr, _1));
	plugin->onTimerEnd.connect(std::bind(&Irccd::handleTimerEnd, this, ptr, _1));

	/* Store reference to irccd */
	duk::putGlobal(plugin->context(), "\xff""\xff""irccd", duk::RawPointer<Irccd>{this});

	/* Initial load now */
	try {
		plugin->onLoad();
		m_plugins.insert({plugin->info().name, plugin});
	} catch (const std::exception &ex) {
		log::info() << "plugin " << plugin->info().name << ": " << ex.what() << std::endl;
	}
}

void Irccd::loadPlugin(std::string name, const std::string &source, bool find)
{
	if (m_plugins.count(name) > 0)
		throw std::invalid_argument("plugin already loaded");

	std::vector<string> paths;
	std::shared_ptr<Plugin> plugin;

	if (find)
		for (const std::string &dir : path::list(path::PathPlugins))
			paths.push_back(dir + source + ".js");
	else
		paths.push_back(source);

	/* Iterate over all paths */
	log::info() << "plugin " << name << ": trying to load:" << std::endl;

	for (const auto &path : paths) {
		log::info() << "  from " << path << std::endl;

		try {
			plugin = std::make_shared<Plugin>(name, path, m_pluginConf[name]);
			break;
		} catch (const std::exception &ex) {
			log::info() << "    error: " << ex.what() << std::endl;
		}
	}

	if (plugin)
		addPlugin(std::move(plugin));
	else
		throw std::runtime_error("no suitable plugin found");
}

void Irccd::reloadPlugin(const std::string &name)
{
	auto plugin = getPlugin(name);

	if (plugin)
		plugin->onReload();
}

void Irccd::unloadPlugin(const std::string &name)
{
	auto plugin = getPlugin(name);

	if (plugin) {
		plugin->onUnload();
		m_plugins.erase(name);
	}
}

#endif // !WITH_JS

/*
 * Timer slots
 * ------------------------------------------------------------------
 *
 * These slots are called from timer threads.
 */

#if defined(WITH_JS)

void Irccd::handleTimerSignal(std::weak_ptr<Plugin> ptr, std::shared_ptr<Timer> timer)
{
	post([this, ptr, timer] () {
		auto plugin = ptr.lock();

		if (!plugin)
			return;

		auto &ctx = plugin->context();

		duk::StackAssert sa(ctx);

		try {
			duk::getGlobal<void>(ctx, "\xff""\xff""timer-" + std::to_string(reinterpret_cast<std::intptr_t>(timer.get())));
			duk::pcall(ctx, 0);
			duk::pop(ctx);
		} catch (const std::exception &) {
			log::info() << "failure" << std::endl;
		}
	});
}

void Irccd::handleTimerEnd(std::weak_ptr<Plugin> ptr, std::shared_ptr<Timer> timer)
{
	post([this, ptr, timer] () {
		auto plugin = ptr.lock();

		if (plugin) {
			log::debug() << "timer: finished, removing from plugin `" << plugin->info().name << "'" << std::endl;
			plugin->removeTimer(timer);
		}
	});
}

#endif

void Irccd::run()
{
	while (m_running) {
		poll();
		dispatch();
	}
}

void Irccd::poll()
{
	fd_set setinput;
	fd_set setoutput;
	auto max = m_socketServer.handle();
	auto set = [&] (fd_set &set, net::Handle handle) {
		FD_SET(handle, &set);

		if (handle > max)
			max = handle;
	};

	FD_ZERO(&setinput);
	FD_ZERO(&setoutput);

	/* 1. Add master socket */
	FD_SET(m_socketServer.handle(), &setinput);

	/* 2. Add servers */
	for (auto &pair : m_servers) {
		pair.second->update();
		pair.second->prepare(setinput, setoutput, max);
	}

	/* 3. Add transports clients */
	for (auto &pair : m_lookupTransportClients) {
		set(setinput, pair.first);

		if (pair.second->hasOutput())
			set(setoutput, pair.first);
	}

	/* 4. Add transport servers */
	for (auto &pair : m_lookupTransportServers)
		set(setinput, pair.first);

	/* 5. Do the selection */
	struct timeval tv;

	tv.tv_sec = 0;
	tv.tv_usec = 250000;

	int error = select(max + 1, &setinput, &setoutput, nullptr, &tv);

	/* Skip anyway */
	if (!m_running)
		return;

	/* Skip on error */
	if (error < 0 && errno != EINTR) {
		log::warning() << "irccd: " << net::error(error) << endl;
		return;
	}

	process(setinput, setoutput);
}

void Irccd::dispatch()
{
	/*
	 * Make a copy because the events can add other events while we are iterating it. Also lock because the timers
	 * may alter these events too.
	 */
	std::vector<Event> copy;

	{
		std::lock_guard<mutex> lock(m_mutex);

		copy = move(m_events);

		/* Clear for safety */
		m_events.clear();
	}

	if (copy.size() > 0)
		log::debug() << "irccd: dispatching " << copy.size() << " event" << (copy.size() > 1 ? "s" : "") << endl;

	for (auto &ev : copy)
		ev();
}

void Irccd::stop()
{
	log::debug() << "irccd: requesting to stop now" << endl;

	m_running = false;
}

} // !irccd
