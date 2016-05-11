/*
 * service-server.cpp -- manage IRC servers
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

#include <format.h>

#include "irccd.hpp"
#include "logger.hpp"
#include "plugin.hpp"
#include "server.hpp"
#include "server-event.hpp"
#include "service-server.hpp"
#include "service-transport.hpp"
#include "sysconfig.hpp"
#include "util.hpp"

using namespace fmt::literals;

namespace irccd {

void ServerService::handleChannelMode(std::weak_ptr<Server> ptr, std::string origin, std::string channel, std::string mode, std::string arg)
{
	std::shared_ptr<Server> server = ptr.lock();

	if (!server) {
		return;
	}

	log::debug() << "server " << server->info().name << ": event onChannelMode:\n";
	log::debug() << "  origin: " << origin << "\n";
	log::debug() << "  channel: " << channel << "\n";
	log::debug() << "  mode: " << mode << "\n";
	log::debug() << "  argument: " << arg << std::endl;

	m_irccd.transportService().broadcast(json::object({
		{ "event",	"onChannelMode"		},
		{ "server",	server->info().name	},
		{ "origin",	origin			},
		{ "channel",	channel			},
		{ "mode",	mode			},
		{ "argument",	arg			}
	}).toJson(0));

#if defined(WITH_JS)
	m_irccd.post(ServerEvent(server->info().name, origin, channel,
		[=] (Plugin &) -> std::string {
			return "onChannelMode";
		},
		[=] (Plugin &plugin) {
			plugin.onChannelMode(std::move(server), std::move(origin), std::move(channel), std::move(mode), std::move(arg));
		}
	));
#endif
}

void ServerService::handleChannelNotice(std::weak_ptr<Server> ptr, std::string origin, std::string channel, std::string message)
{
	std::shared_ptr<Server> server = ptr.lock();

	if (!server) {
		return;
	}

	log::debug() << "server " << server->info().name << ": event onChannelNotice:\n";
	log::debug() << "  origin: " << origin << "\n";
	log::debug() << "  channel: " << channel << "\n";
	log::debug() << "  message: " << message << std::endl;

	m_irccd.transportService().broadcast(json::object({
		{ "event",	"onChannelNotice"	},
		{ "server",	server->info().name	},
		{ "origin",	origin			},
		{ "channel",	channel			},
		{ "message",	message			}
	}).toJson(0));

#if defined(WITH_JS)
	m_irccd.post(ServerEvent(server->info().name, origin, channel,
		[=] (Plugin &) -> std::string {
			return "onChannelNotice";
		},
		[=] (Plugin &plugin) {
			plugin.onChannelNotice(std::move(server), std::move(origin), std::move(channel), std::move(message));
		}
	));
#endif
}

void ServerService::handleConnect(std::weak_ptr<Server> ptr)
{
	std::shared_ptr<Server> server = ptr.lock();

	if (!server) {
		return;
	}

	log::debug() << "server " << server->info().name << ": event onConnect" << std::endl;

	m_irccd.transportService().broadcast(json::object({
		{ "event",	"onConnect"		},
		{ "server",	server->info().name	}
	}).toJson(0));

#if defined(WITH_JS)
	m_irccd.post(ServerEvent(server->info().name, /* origin */ "", /* channel */ "",
		[=] (Plugin &) -> std::string {
			return "onConnect";
		},
		[=] (Plugin &plugin) {
			plugin.onConnect(std::move(server));
		}
	));
#endif
}

void ServerService::handleInvite(std::weak_ptr<Server> ptr, std::string origin, std::string channel, std::string target)
{
	std::shared_ptr<Server> server = ptr.lock();

	if (!server) {
		return;
	}

	log::debug() << "server " << server->info().name << ": event onInvite:\n";
	log::debug() << "  origin: " << origin << "\n";
	log::debug() << "  channel: " << channel << "\n";
	log::debug() << "  target: " << target << std::endl;

	m_irccd.transportService().broadcast(json::object({
		{ "event",	"onInvite"		},
		{ "server",	server->info().name	},
		{ "origin",	origin			},
		{ "channel",	channel			}
	}).toJson(0));

#if defined(WITH_JS)
	m_irccd.post(ServerEvent(server->info().name, origin, channel,
		[=] (Plugin &) -> std::string {
			return "onInvite";
		},
		[=] (Plugin &plugin) {
			plugin.onInvite(std::move(server), std::move(origin), std::move(channel));
		}
	));
#endif
}

void ServerService::handleJoin(std::weak_ptr<Server> ptr, std::string origin, std::string channel)
{
	std::shared_ptr<Server> server = ptr.lock();

	if (!server) {
		return;
	}

	log::debug() << "server " << server->info().name << ": event onJoin:\n";
	log::debug() << "  origin: " << origin << "\n";
	log::debug() << "  channel: " << channel << std::endl;

	m_irccd.transportService().broadcast(json::object({
		{ "event",	"onJoin"		},
		{ "server",	server->info().name	},
		{ "origin",	origin			},
		{ "channel",	channel			}
	}).toJson(0));

#if defined(WITH_JS)
	m_irccd.post(ServerEvent(server->info().name, origin, channel,
		[=] (Plugin &) -> std::string {
			return "onJoin";
		},
		[=] (Plugin &plugin) {
			plugin.onJoin(std::move(server), std::move(origin), std::move(channel));
		}
	));
#endif
}

void ServerService::handleKick(std::weak_ptr<Server> ptr, std::string origin, std::string channel, std::string target, std::string reason)
{
	std::shared_ptr<Server> server = ptr.lock();

	if (!server) {
		return;
	}

	log::debug() << "server " << server->info().name << ": event onKick:\n";
	log::debug() << "  origin: " << origin << "\n";
	log::debug() << "  channel: " << channel << "\n";
	log::debug() << "  target: " << target << "\n";
	log::debug() << "  reason: " << reason << std::endl;

	m_irccd.transportService().broadcast(json::object({
		{ "event",	"onKick"		},
		{ "server",	server->info().name	},
		{ "origin",	origin			},
		{ "channel",	channel			},
		{ "target",	target			},
		{ "reason",	reason			}
	}).toJson(0));

#if defined(WITH_JS)
	m_irccd.post(ServerEvent(server->info().name, origin, channel,
		[=] (Plugin &) -> std::string {
			return "onKick";
		},
		[=] (Plugin &plugin) {
			plugin.onKick(std::move(server), std::move(origin), std::move(channel), std::move(target), std::move(reason));
		}
	));
#endif
}

void ServerService::handleMessage(std::weak_ptr<Server> ptr, std::string origin, std::string channel, std::string message)
{
	std::shared_ptr<Server> server = ptr.lock();

	if (!server) {
		return;
	}

	log::debug() << "server " << server->info().name << ": event onMessage:\n";
	log::debug() << "  origin: " << origin << "\n";
	log::debug() << "  channel: " << channel << "\n";
	log::debug() << "  message: " << message << std::endl;

	m_irccd.transportService().broadcast(json::object({
		{ "event",	"onMessage"		},
		{ "server",	server->info().name	},
		{ "origin",	origin			},
		{ "channel",	channel			},
		{ "message",	message			}
	}).toJson(0));

#if defined(WITH_JS)
	m_irccd.post(ServerEvent(server->info().name, origin, channel,
		[=] (Plugin &plugin) -> std::string {
			return util::parseMessage(message, server->settings().command, plugin.name()).second == util::MessageType::Command ? "onCommand" : "onMessage";
		},
		[=] (Plugin &plugin) {
			util::MessagePair pack = util::parseMessage(message, server->settings().command, plugin.name());

			if (pack.second == util::MessageType::Command)
				plugin.onCommand(std::move(server), std::move(origin), std::move(channel), std::move(pack.first));
			else
				plugin.onMessage(std::move(server), std::move(origin), std::move(channel), std::move(pack.first));
		}
	));
#endif
}

void ServerService::handleMe(std::weak_ptr<Server> ptr, std::string origin, std::string target, std::string message)
{
	std::shared_ptr<Server> server = ptr.lock();

	if (!server) {
		return;
	}

	log::debug() << "server " << server->info().name << ": event onMe:\n";
	log::debug() << "  origin: " << origin << "\n";
	log::debug() << "  target: " << target << "\n";
	log::debug() << "  message: " << message << std::endl;

	m_irccd.transportService().broadcast(json::object({
		{ "event",	"onMe"			},
		{ "server",	server->info().name	},
		{ "origin",	origin			},
		{ "target",	target			},
		{ "message",	message			}
	}).toJson(0));

#if defined(WITH_JS)
	m_irccd.post(ServerEvent(server->info().name, origin, target,
		[=] (Plugin &) -> std::string {
			return "onMe";
		},
		[=] (Plugin &plugin) {
			plugin.onMe(std::move(server), std::move(origin), std::move(target), std::move(message));
		}
	));
#endif
}

void ServerService::handleMode(std::weak_ptr<Server> ptr, std::string origin, std::string mode)
{
	std::shared_ptr<Server> server = ptr.lock();

	if (!server) {
		return;
	}

	log::debug() << "server " << server->info().name << ": event onMode\n";
	log::debug() << "  origin: " << origin << "\n";
	log::debug() << "  mode: " << mode << std::endl;

	m_irccd.transportService().broadcast(json::object({
		{ "event",	"onMode"		},
		{ "server",	server->info().name	},
		{ "origin",	origin			},
		{ "mode",	mode			}
	}).toJson(0));

#if defined(WITH_JS)
	m_irccd.post(ServerEvent(server->info().name, origin, /* channel */ "",
		[=] (Plugin &) -> std::string {
			return "onMode";
		},
		[=] (Plugin &plugin) {
			plugin.onMode(std::move(server), std::move(origin), std::move(mode));
		}
	));
#endif
}

void ServerService::handleNames(std::weak_ptr<Server> ptr, std::string channel, std::set<std::string> nicknames)
{
	std::shared_ptr<Server> server = ptr.lock();

	if (!server) {
		return;
	}

	log::debug() << "server " << server->info().name << ": event onNames:\n";
	log::debug() << "  channel: " << channel << "\n";
	log::debug() << "  names: " << util::join(nicknames.begin(), nicknames.end(), ", ") << std::endl;

	json::Value names(std::vector<json::Value>(nicknames.begin(), nicknames.end()));

	m_irccd.transportService().broadcast(json::object({
		{ "event",	"onNames"		},
		{ "server",	server->info().name	},
		{ "channel",	channel			},
		{ "names",	std::move(names)	}
	}).toJson(0));

#if defined(WITH_JS)
	m_irccd.post(ServerEvent(server->info().name, /* origin */ "", channel,
		[=] (Plugin &) -> std::string {
			return "onNames";
		},
		[=] (Plugin &plugin) {
			plugin.onNames(std::move(server), std::move(channel), std::vector<std::string>(nicknames.begin(), nicknames.end()));
		}
	));
#endif
}

void ServerService::handleNick(std::weak_ptr<Server> ptr, std::string origin, std::string nickname)
{
	std::shared_ptr<Server> server = ptr.lock();

	if (!server) {
		return;
	}

	log::debug() << "server " << server->info().name << ": event onNick:\n";
	log::debug() << "  origin: " << origin << "\n";
	log::debug() << "  nickname: " << nickname << std::endl;

	m_irccd.transportService().broadcast(json::object({
		{ "event",	"onNick"		},
		{ "server",	server->info().name	},
		{ "origin",	origin			},
		{ "nickname",	nickname		}
	}).toJson(0));

#if defined(WITH_JS)
	m_irccd.post(ServerEvent(server->info().name, origin, /* channel */ "",
		[=] (Plugin &) -> std::string {
			return "onNick";
		},
		[=] (Plugin &plugin) {
			plugin.onNick(std::move(server), std::move(origin), std::move(nickname));
		}
	));
#endif
}

void ServerService::handleNotice(std::weak_ptr<Server> ptr, std::string origin, std::string message)
{
	std::shared_ptr<Server> server = ptr.lock();

	if (!server) {
		return;
	}

	log::debug() << "server " << server->info().name << ": event onNotice:\n";
	log::debug() << "  origin: " << origin << "\n";
	log::debug() << "  message: " << message << std::endl;

	m_irccd.transportService().broadcast(json::object({
		{ "event",	"onNotice"		},
		{ "server",	server->info().name	},
		{ "origin",	origin			},
		{ "message",	message			}
	}).toJson(0));

#if defined(WITH_JS)
	m_irccd.post(ServerEvent(server->info().name, origin, /* channel */ "",
		[=] (Plugin &) -> std::string {
			return "onNotice";
		},
		[=] (Plugin &plugin) {
			plugin.onNotice(std::move(server), std::move(origin), std::move(message));
		}
	));
#endif
}

void ServerService::handlePart(std::weak_ptr<Server> ptr, std::string origin, std::string channel, std::string reason)
{
	std::shared_ptr<Server> server = ptr.lock();

	if (!server) {
		return;
	}

	log::debug() << "server " << server->info().name << ": event onPart:\n";
	log::debug() << "  origin: " << origin << "\n";
	log::debug() << "  channel: " << channel << "\n";
	log::debug() << "  reason: " << reason << std::endl;

	m_irccd.transportService().broadcast(json::object({
		{ "event",	"onPart"		},
		{ "server",	server->info().name	},
		{ "origin",	origin			},
		{ "channel",	channel			},
		{ "reason",	reason			}
	}).toJson(0));

#if defined(WITH_JS)
	m_irccd.post(ServerEvent(server->info().name, origin, channel,
		[=] (Plugin &) -> std::string {
			return "onPart";
		},
		[=] (Plugin &plugin) {
			plugin.onPart(std::move(server), std::move(origin), std::move(channel), std::move(reason));
		}
	));
#endif
}

void ServerService::handleQuery(std::weak_ptr<Server> ptr, std::string origin, std::string message)
{
	std::shared_ptr<Server> server = ptr.lock();

	if (!server) {
		return;
	}

	log::debug() << "server " << server->info().name << ": event onQuery:\n";
	log::debug() << "  origin: " << origin << "\n";
	log::debug() << "  message: " << message << std::endl;

	m_irccd.transportService().broadcast(json::object({
		{ "event",	"onQuery"		},
		{ "server",	server->info().name	},
		{ "origin",	origin			},
		{ "message",	message			}
	}).toJson(0));

#if defined(WITH_JS)
	m_irccd.post(ServerEvent(server->info().name, origin, /* channel */ "",
		[=] (Plugin &plugin) -> std::string {
			return util::parseMessage(message, server->settings().command, plugin.name()).second == util::MessageType::Command ? "onQueryCommand" : "onQuery";
		},
		[=] (Plugin &plugin) {
			util::MessagePair pack = util::parseMessage(message, server->settings().command, plugin.name());

			if (pack.second == util::MessageType::Command)
				plugin.onQueryCommand(std::move(server), std::move(origin), std::move(pack.first));
			else
				plugin.onQuery(std::move(server), std::move(origin), std::move(pack.first));
		}
	));
#endif
}

void ServerService::handleTopic(std::weak_ptr<Server> ptr, std::string origin, std::string channel, std::string topic)
{
	std::shared_ptr<Server> server = ptr.lock();

	if (!server) {
		return;
	}

	log::debug() << "server " << server->info().name << ": event onTopic:\n";
	log::debug() << "  origin: " << origin << "\n";
	log::debug() << "  channel: " << channel << "\n";
	log::debug() << "  topic: " << topic << std::endl;

	m_irccd.transportService().broadcast(json::object({
		{ "event",	"onTopic"		},
		{ "server",	server->info().name	},
		{ "origin",	origin			},
		{ "channel",	channel			},
		{ "topic",	topic			}
	}).toJson(0));

#if defined(WITH_JS)
	m_irccd.post(ServerEvent(server->info().name, origin, channel,
		[=] (Plugin &) -> std::string {
			return "onTopic";
		},
		[=] (Plugin &plugin) {
			plugin.onTopic(std::move(server), std::move(origin), std::move(channel), std::move(topic));
		}
	));
#endif
}

void ServerService::handleWhois(std::weak_ptr<Server> ptr, ServerWhois whois)
{
	std::shared_ptr<Server> server = ptr.lock();

	if (!server) {
		return;
	}

	log::debug() << "server " << server->info().name << ": event onWhois\n";
	log::debug() << "  nickname: " << whois.nick << "\n";
	log::debug() << "  username: " << whois.user << "\n";
	log::debug() << "  host: " << whois.host << "\n";
	log::debug() << "  realname: " << whois.realname << "\n";
	log::debug() << "  channels: " << util::join(whois.channels.begin(), whois.channels.end()) << std::endl;

	m_irccd.transportService().broadcast(json::object({
		{ "server",	server->info().name	},
		{ "nickname",	whois.nick		},
		{ "username",	whois.user		},
		{ "host",	whois.host		},
		{ "realname",	whois.realname		}
	}).toJson(0));

#if defined(WITH_JS)
	m_irccd.post(ServerEvent(server->info().name, /* origin */ "", /* channel */ "",
		[=] (Plugin &) -> std::string {
			return "onWhois";
		},
		[=] (Plugin &plugin) {
			plugin.onWhois(std::move(server), std::move(whois));
		}
	));
#endif
}

ServerService::ServerService(Irccd &irccd)
	: m_irccd(irccd)
{
}

void ServerService::prepare(fd_set &in, fd_set &out, net::Handle &max)
{
	for (auto &server : m_servers) {
		server->update();
		server->prepare(in, out, max);
	}
}

void ServerService::sync(fd_set &in, fd_set &out)
{
	for (auto &server : m_servers) {
		server->sync(in, out);
	}
}

bool ServerService::hasServer(const std::string &name) const noexcept
{
	return std::count_if(m_servers.cbegin(), m_servers.end(), [&] (const auto &sv) {
		return sv->info().name == name;
	}) > 0;
}

void ServerService::addServer(std::shared_ptr<Server> server) noexcept
{
	assert(!hasServer(server->info().name));

	using namespace std::placeholders;

	std::weak_ptr<Server> ptr(server);

	server->onChannelMode.connect(std::bind(&ServerService::handleChannelMode, this, ptr, _1, _2, _3, _4));
	server->onChannelNotice.connect(std::bind(&ServerService::handleChannelNotice, this, ptr, _1, _2, _3));
	server->onConnect.connect(std::bind(&ServerService::handleConnect, this, ptr));
	server->onInvite.connect(std::bind(&ServerService::handleInvite, this, ptr, _1, _2, _3));
	server->onJoin.connect(std::bind(&ServerService::handleJoin, this, ptr, _1, _2));
	server->onKick.connect(std::bind(&ServerService::handleKick, this, ptr, _1, _2, _3, _4));
	server->onMessage.connect(std::bind(&ServerService::handleMessage, this, ptr, _1, _2, _3));
	server->onMe.connect(std::bind(&ServerService::handleMe, this, ptr, _1, _2, _3));
	server->onMode.connect(std::bind(&ServerService::handleMode, this, ptr, _1, _2));
	server->onNames.connect(std::bind(&ServerService::handleNames, this, ptr, _1, _2));
	server->onNick.connect(std::bind(&ServerService::handleNick, this, ptr, _1, _2));
	server->onNotice.connect(std::bind(&ServerService::handleNotice, this, ptr, _1, _2));
	server->onPart.connect(std::bind(&ServerService::handlePart, this, ptr, _1, _2, _3));
	server->onQuery.connect(std::bind(&ServerService::handleQuery, this, ptr, _1, _2));
	server->onTopic.connect(std::bind(&ServerService::handleTopic, this, ptr, _1, _2, _3));
	server->onWhois.connect(std::bind(&ServerService::handleWhois, this, ptr, _1));
	server->onDie.connect([this, ptr] () {
		m_irccd.post([=] (Irccd &) {
			auto server = ptr.lock();

			if (server) {
				log::info("server {}: removed"_format(server->info().name));
				m_servers.erase(std::find(m_servers.begin(), m_servers.end(), server));
			}
		});
	});

	m_servers.push_back(std::move(server));
}

std::shared_ptr<Server> ServerService::getServer(const std::string &name) const noexcept
{
	auto it = std::find_if(m_servers.begin(), m_servers.end(), [&] (const auto &server) {
		return server->info().name == name;
	});

	if (it == m_servers.end()) {
		return nullptr;
	}

	return *it;
}

std::shared_ptr<Server> ServerService::requireServer(const std::string &name) const
{
	auto server = getServer(name);

	if (!server) {
		throw std::invalid_argument("server {} not found"_format(name));
	}

	return server;
}

void ServerService::removeServer(const std::string &name)
{
	auto it = std::find_if(m_servers.begin(), m_servers.end(), [&] (const auto &server) {
		return server->info().name == name;
	});

	if (it != m_servers.end()) {
		(*it)->disconnect();
		m_servers.erase(it);
	}
}

void ServerService::clearServers() noexcept
{
	for (auto &server : m_servers) {
		server->disconnect();
	}

	m_servers.clear();
}

} // !irccd
