/*
 * server_service.cpp -- server service
 *
 * Copyright (c) 2013-2019 David Demelier <markand@malikania.fr>
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

#include <irccd/sysconfig.hpp>

#include <irccd/json_util.hpp>
#include <irccd/string_util.hpp>

#include "bot.hpp"
#include "logger.hpp"
#include "plugin_service.hpp"
#include "rule_service.hpp"
#include "server.hpp"
#include "server_service.hpp"
#include "server_util.hpp"
#include "transport_service.hpp"

namespace irccd::daemon {

namespace {

class dispatcher {
private:
	bot& bot_;

	template <typename EventNameFunc, typename ExecFunc>
	void dispatch(std::string_view, std::string_view, std::string_view, EventNameFunc&&, ExecFunc);

public:
	dispatcher(bot& bot);
	void operator()(const std::monostate&);
	void operator()(const connect_event&);
	void operator()(const disconnect_event&);
	void operator()(const invite_event&);
	void operator()(const join_event&);
	void operator()(const kick_event&);
	void operator()(const message_event&);
	void operator()(const me_event&);
	void operator()(const mode_event&);
	void operator()(const names_event&);
	void operator()(const nick_event&);
	void operator()(const notice_event&);
	void operator()(const part_event&);
	void operator()(const topic_event&);
	void operator()(const whois_event&);
};

template <typename EventNameFunc, typename ExecFunc>
void dispatcher::dispatch(std::string_view server,
                          std::string_view origin,
                          std::string_view target,
                          EventNameFunc&& name_func,
                          ExecFunc exec_func)
{
	for (const auto& plugin : bot_.plugins().list()) {
		const auto eventname = name_func(*plugin);
		const auto allowed = bot_.rules().solve(server, target, origin, plugin->get_name(), eventname);

		if (!allowed) {
			bot_.get_log().debug("rule", "") << "event skipped on match" << std::endl;
			continue;
		}

		bot_.get_log().debug("rule", "") << "event allowed" << std::endl;

		try {
			exec_func(*plugin);
		} catch (const std::exception& ex) {
			bot_.get_log().warning(*plugin) << ex.what() << std::endl;
		}
	}
}

dispatcher::dispatcher(bot& bot)
	: bot_(bot)
{
}

void dispatcher::operator()(const std::monostate&)
{
}

void dispatcher::operator()(const connect_event& ev)
{
	bot_.get_log().debug(*ev.server) << "event onConnect" << std::endl;
	bot_.transports().broadcast(nlohmann::json::object({
		{ "event",      "onConnect"             },
		{ "server",     ev.server->get_id()     }
	}));

	dispatch(ev.server->get_id(), /* origin */ "", /* channel */ "",
		[=] (plugin&) -> std::string {
			return "onConnect";
		},
		[=] (plugin& plugin) {
			plugin.handle_connect(bot_, ev);
		}
	);
}

void dispatcher::operator()(const disconnect_event& ev)
{
	bot_.get_log().debug(*ev.server) << "event onDisconnect" << std::endl;
	bot_.transports().broadcast(nlohmann::json::object({
		{ "event",      "onDisconnect"          },
		{ "server",     ev.server->get_id()     }
	}));

	dispatch(ev.server->get_id(), /* origin */ "", /* channel */ "",
		[=] (plugin&) -> std::string {
			return "onDisconnect";
		},
		[=] (plugin& plugin) {
			plugin.handle_disconnect(bot_, ev);
		}
	);
}

void dispatcher::operator()(const invite_event& ev)
{
	bot_.get_log().debug(*ev.server) << "event onInvite:" << std::endl;
	bot_.get_log().debug(*ev.server) << "  origin: " << ev.origin << std::endl;
	bot_.get_log().debug(*ev.server) << "  channel: " << ev.channel << std::endl;
	bot_.get_log().debug(*ev.server) << "  target: " << ev.nickname << std::endl;

	bot_.transports().broadcast(nlohmann::json::object({
		{ "event",      "onInvite"              },
		{ "server",     ev.server->get_id()     },
		{ "origin",     ev.origin               },
		{ "channel",    ev.channel              }
	}));

	dispatch(ev.server->get_id(), ev.origin, ev.channel,
		[=] (plugin&) -> std::string {
			return "onInvite";
		},
		[=] (plugin& plugin) {
			plugin.handle_invite(bot_, ev);
		}
	);
}

void dispatcher::operator()(const join_event& ev)
{
	bot_.get_log().debug(*ev.server) << "event onJoin:" << std::endl;
	bot_.get_log().debug(*ev.server) << "  origin: " << ev.origin << std::endl;
	bot_.get_log().debug(*ev.server) << "  channel: " << ev.channel << std::endl;

	bot_.transports().broadcast(nlohmann::json::object({
		{ "event",      "onJoin"                },
		{ "server",     ev.server->get_id()     },
		{ "origin",     ev.origin               },
		{ "channel",    ev.channel              }
	}));

	dispatch(ev.server->get_id(), ev.origin, ev.channel,
		[=] (plugin&) -> std::string {
			return "onJoin";
		},
		[=] (plugin& plugin) {
			plugin.handle_join(bot_, ev);
		}
	);
}

void dispatcher::operator()(const kick_event& ev)
{
	bot_.get_log().debug(*ev.server) << "event onKick:" << std::endl;
	bot_.get_log().debug(*ev.server) << "  origin: " << ev.origin << std::endl;
	bot_.get_log().debug(*ev.server) << "  channel: " << ev.channel << std::endl;
	bot_.get_log().debug(*ev.server) << "  target: " << ev.target << std::endl;
	bot_.get_log().debug(*ev.server) << "  reason: " << ev.reason << std::endl;

	bot_.transports().broadcast(nlohmann::json::object({
		{ "event",      "onKick"                },
		{ "server",     ev.server->get_id()     },
		{ "origin",     ev.origin               },
		{ "channel",    ev.channel              },
		{ "target",     ev.target               },
		{ "reason",     ev.reason               }
	}));

	dispatch(ev.server->get_id(), ev.origin, ev.channel,
		[=] (plugin&) -> std::string {
			return "onKick";
		},
		[=] (plugin& plugin) {
			plugin.handle_kick(bot_, ev);
		}
	);
}

void dispatcher::operator()(const message_event& ev)
{
	bot_.get_log().debug(*ev.server) << "event onMessage:" << std::endl;
	bot_.get_log().debug(*ev.server) << "  origin: " << ev.origin << std::endl;
	bot_.get_log().debug(*ev.server) << "  channel: " << ev.channel << std::endl;
	bot_.get_log().debug(*ev.server) << "  message: " << ev.message << std::endl;

	bot_.transports().broadcast(nlohmann::json::object({
		{ "event",      "onMessage"             },
		{ "server",     ev.server->get_id()     },
		{ "origin",     ev.origin               },
		{ "channel",    ev.channel              },
		{ "message",    ev.message              }
	}));

	dispatch(ev.server->get_id(), ev.origin, ev.channel,
		[=] (plugin& plugin) -> std::string {
			return server_util::message_type::parse(
				ev.message,
				ev.server->get_command_char(),
				plugin.get_id()
			).type == server_util::message_type::is_command ? "onCommand" : "onMessage";
		},
		[=] (plugin& plugin) mutable {
			auto copy = ev;
			auto pack = server_util::message_type::parse(
				copy.message,
				copy.server->get_command_char(),
				plugin.get_id()
			);

			copy.message = pack.message;

			if (pack.type == server_util::message_type::is_command)
				plugin.handle_command(bot_, copy);
			else
				plugin.handle_message(bot_, copy);
		}
	);
}

void dispatcher::operator()(const me_event& ev)
{
	bot_.get_log().debug(*ev.server) << "event onMe:" << std::endl;
	bot_.get_log().debug(*ev.server) << "  origin: " << ev.origin << std::endl;
	bot_.get_log().debug(*ev.server) << "  target: " << ev.channel << std::endl;
	bot_.get_log().debug(*ev.server) << "  message: " << ev.message << std::endl;

	bot_.transports().broadcast(nlohmann::json::object({
		{ "event",      "onMe"                  },
		{ "server",     ev.server->get_id()     },
		{ "origin",     ev.origin               },
		{ "target",     ev.channel              },
		{ "message",    ev.message              }
	}));

	dispatch(ev.server->get_id(), ev.origin, ev.channel,
		[=] (plugin&) -> std::string {
			return "onMe";
		},
		[=] (plugin& plugin) {
			plugin.handle_me(bot_, ev);
		}
	);
}

void dispatcher::operator()(const mode_event& ev)
{
	bot_.get_log().debug(*ev.server) << "event onMode" << std::endl;
	bot_.get_log().debug(*ev.server) << "  origin: " << ev.origin << std::endl;
	bot_.get_log().debug(*ev.server) << "  channel: " << ev.channel << std::endl;
	bot_.get_log().debug(*ev.server) << "  mode: " << ev.mode << std::endl;
	bot_.get_log().debug(*ev.server) << "  limit: " << ev.limit << std::endl;
	bot_.get_log().debug(*ev.server) << "  user: " << ev.user << std::endl;
	bot_.get_log().debug(*ev.server) << "  mask: " << ev.mask << std::endl;

	bot_.transports().broadcast(nlohmann::json::object({
		{ "event",      "onMode"                },
		{ "server",     ev.server->get_id()     },
		{ "origin",     ev.origin               },
		{ "channel",    ev.channel              },
		{ "mode",       ev.mode                 },
		{ "limit",      ev.limit                },
		{ "user",       ev.user                 },
		{ "mask",       ev.mask                 }
	}));

	dispatch(ev.server->get_id(), ev.origin, /* channel */ "",
		[=] (plugin &) -> std::string {
			return "onMode";
		},
		[=] (plugin &plugin) {
			plugin.handle_mode(bot_, ev);
		}
	);
}

void dispatcher::operator()(const names_event& ev)
{
	bot_.get_log().debug(*ev.server) << "event onNames:" << std::endl;
	bot_.get_log().debug(*ev.server) << "  channel: " << ev.channel << std::endl;
	bot_.get_log().debug(*ev.server) << "  names: " << string_util::join(ev.names.begin(), ev.names.end(), ", ") << std::endl;

	auto names = nlohmann::json::array();

	for (const auto& v : ev.names)
		names.push_back(v);

	bot_.transports().broadcast(nlohmann::json::object({
		{ "event",      "onNames"               },
		{ "server",     ev.server->get_id()     },
		{ "channel",    ev.channel              },
		{ "names",      std::move(names)        }
	}));

	dispatch(ev.server->get_id(), /* origin */ "", ev.channel,
		[=] (plugin&) -> std::string {
			return "onNames";
		},
		[=] (plugin& plugin) {
			plugin.handle_names(bot_, ev);
		}
	);
}

void dispatcher::operator()(const nick_event& ev)
{
	bot_.get_log().debug(*ev.server) << "event onNick:" << std::endl;
	bot_.get_log().debug(*ev.server) << "  origin: " << ev.origin << std::endl;
	bot_.get_log().debug(*ev.server) << "  nickname: " << ev.nickname << std::endl;

	bot_.transports().broadcast(nlohmann::json::object({
		{ "event",      "onNick"                },
		{ "server",     ev.server->get_id()     },
		{ "origin",     ev.origin               },
		{ "nickname",   ev.nickname             }
	}));

	dispatch(ev.server->get_id(), ev.origin, /* channel */ "",
		[=] (plugin&) -> std::string {
			return "onNick";
		},
		[=] (plugin& plugin) {
			plugin.handle_nick(bot_, ev);
		}
	);
}

void dispatcher::operator()(const notice_event& ev)
{
	bot_.get_log().debug(*ev.server) << "event onNotice:" << std::endl;
	bot_.get_log().debug(*ev.server) << "  origin: " << ev.origin << std::endl;
	bot_.get_log().debug(*ev.server) << "  channel: " << ev.channel << std::endl;
	bot_.get_log().debug(*ev.server) << "  message: " << ev.message << std::endl;

	bot_.transports().broadcast(nlohmann::json::object({
		{ "event",      "onNotice"              },
		{ "server",     ev.server->get_id()     },
		{ "origin",     ev.origin               },
		{ "channel",    ev.channel              },
		{ "message",    ev.message              }
	}));

	dispatch(ev.server->get_id(), ev.origin, /* channel */ "",
		[=] (plugin&) -> std::string {
			return "onNotice";
		},
		[=] (plugin& plugin) {
			plugin.handle_notice(bot_, ev);
		}
	);
}

void dispatcher::operator()(const part_event& ev)
{
	bot_.get_log().debug(*ev.server) << "event onPart:" << std::endl;
	bot_.get_log().debug(*ev.server) << "  origin: " << ev.origin << std::endl;
	bot_.get_log().debug(*ev.server) << "  channel: " << ev.channel << std::endl;
	bot_.get_log().debug(*ev.server) << "  reason: " << ev.reason << std::endl;

	bot_.transports().broadcast(nlohmann::json::object({
		{ "event",      "onPart"                },
		{ "server",     ev.server->get_id()     },
		{ "origin",     ev.origin               },
		{ "channel",    ev.channel              },
		{ "reason",     ev.reason               }
	}));

	dispatch(ev.server->get_id(), ev.origin, ev.channel,
		[=] (plugin&) -> std::string {
			return "onPart";
		},
		[=] (plugin& plugin) {
			plugin.handle_part(bot_, ev);
		}
	);
}

void dispatcher::operator()(const topic_event& ev)
{
	bot_.get_log().debug(*ev.server) << "event onTopic:" << std::endl;
	bot_.get_log().debug(*ev.server) << "  origin: " << ev.origin << std::endl;
	bot_.get_log().debug(*ev.server) << "  channel: " << ev.channel << std::endl;
	bot_.get_log().debug(*ev.server) << "  topic: " << ev.topic << std::endl;

	bot_.transports().broadcast(nlohmann::json::object({
		{ "event",      "onTopic"               },
		{ "server",     ev.server->get_id()     },
		{ "origin",     ev.origin               },
		{ "channel",    ev.channel              },
		{ "topic",      ev.topic                }
	}));

	dispatch(ev.server->get_id(), ev.origin, ev.channel,
		[=] (plugin&) -> std::string {
			return "onTopic";
		},
		[=] (plugin& plugin) {
			plugin.handle_topic(bot_, ev);
		}
	);
}

void dispatcher::operator()(const whois_event& ev)
{
	bot_.get_log().debug(*ev.server) << "event onWhois" << std::endl;
	bot_.get_log().debug(*ev.server) << "  nickname: " << ev.whois.nick << std::endl;
	bot_.get_log().debug(*ev.server) << "  username: " << ev.whois.user << std::endl;
	bot_.get_log().debug(*ev.server) << "  hostname: " << ev.whois.hostname << std::endl;
	bot_.get_log().debug(*ev.server) << "  realname: " << ev.whois.realname << std::endl;
	bot_.get_log().debug(*ev.server) << "  channels: " << string_util::join(ev.whois.channels, ", ") << std::endl;

	bot_.transports().broadcast(nlohmann::json::object({
		{ "event",      "onWhois"               },
		{ "server",     ev.server->get_id()     },
		{ "nickname",   ev.whois.nick           },
		{ "username",   ev.whois.user           },
		{ "hostname",   ev.whois.hostname       },
		{ "realname",   ev.whois.realname       }
	}));

	dispatch(ev.server->get_id(), /* origin */ "", /* channel */ "",
		[=] (plugin&) -> std::string {
			return "onWhois";
		},
		[=] (plugin& plugin) {
			plugin.handle_whois(bot_, ev);
		}
	);
}

} // !namespace

void server_service::handle_connect(const std::shared_ptr<server>& server, const std::error_code& code)
{
	if (code)
		handle_error(server, code);
	else
		recv(server);
}

void server_service::handle_error(const std::shared_ptr<server>& server,
                                  const std::error_code& code)
{
	assert(server);

	bot_.get_log().warning(*server) << code.message() << std::endl;

	if ((server->get_options() & server::options::auto_reconnect) != server::options::auto_reconnect) {
		remove(server->get_id());
		return;
	}

	bot_.get_log().info(*server) << "reconnecting in " << server->get_reconnect_delay()
	                             << " second(s)" << std::endl;

	server->wait([this, server] (auto code) {
		handle_wait(server, code);
	});

	dispatcher{bot_}(disconnect_event{server});
}

void server_service::handle_recv(const std::shared_ptr<server>& server,
                                 const std::error_code& code,
                                 const event& event)
{
	assert(server);

	if (code) {
		handle_error(server, code);
		return;
	}

	recv(server);
	std::visit(dispatcher(bot_), event);
}

void server_service::handle_wait(const std::shared_ptr<server>& server, const std::error_code& code)
{
	if (code == std::errc::operation_canceled || server->get_state() != server::state::disconnected)
		return;

	connect(server);
}

void server_service::connect(const std::shared_ptr<server>& server)
{
	assert(server);

	server->connect([this, server] (auto code) {
		handle_connect(server, code);
	});
}

void server_service::disconnect(const std::shared_ptr<server>& server)
{
	if (server->get_state() == server::state::disconnected)
		return;

	server->disconnect();
	servers_.erase(std::find(servers_.begin(), servers_.end(), server), servers_.end());
	dispatcher{bot_}(disconnect_event{server});
}

void server_service::reconnect(const std::shared_ptr<server>& server)
{
	disconnect(server);

	if (has(server->get_id()))
		connect(server);
	else
		add(server);
}

void server_service::recv(const std::shared_ptr<server>& server)
{
	assert(server);

	server->recv([this, server] (auto code, auto event) {
		handle_recv(server, code, event);
	});
}

server_service::server_service(bot& bot)
	: bot_(bot)
{
}

auto server_service::list() const noexcept -> const std::vector<std::shared_ptr<server>>&
{
	return servers_;
}

auto server_service::has(std::string_view name) const noexcept -> bool
{
	return std::count_if(servers_.begin(), servers_.end(), [&] (const auto& server) {
		return server->get_id() == name;
	}) > 0;
}

void server_service::add(std::shared_ptr<server> server)
{
	assert(server);
	assert(!has(server->get_id()));

	servers_.push_back(server);
	connect(server);
}

auto server_service::get(std::string_view name) const noexcept -> std::shared_ptr<server>
{
	const auto it = std::find_if(servers_.begin(), servers_.end(), [&] (const auto& server) {
		return server->get_id() == name;
	});

	if (it == servers_.end())
		return nullptr;

	return *it;
}

auto server_service::require(std::string_view name) const -> std::shared_ptr<server>
{
	if (!string_util::is_identifier(name))
		throw server_error(server_error::invalid_identifier);

	const auto s = get(name);

	if (!s)
		throw server_error(server_error::not_found);

	return s;
}

void server_service::disconnect(std::string_view id)
{
	disconnect(require(id));
}

void server_service::reconnect(std::string_view id)
{
	reconnect(require(id));
}

void server_service::reconnect()
{
	const auto save = servers_;

	for (const auto& s : save) {
		try {
			reconnect(s);
		} catch (const server_error& ex) {
			bot_.get_log().warning(*s) << ex.what() << std::endl;
		}
	}
}

void server_service::remove(std::string_view name)
{
	const auto it = std::find_if(servers_.begin(), servers_.end(), [&] (const auto& server) {
		return server->get_id() == name;
	});

	if (it != servers_.end()) {
		(*it)->disconnect();
		servers_.erase(it);
	}
}

void server_service::clear() noexcept
{
	/*
	 * Copy the array, because disconnect() interrupts signals and may
	 * remove the server from the array.
	 */
	const auto save = servers_;

	for (const auto& server : save)
		disconnect(server);

	servers_.clear();
}

void server_service::load(const config& cfg) noexcept
{
	for (const auto& section : cfg) {
		if (section.get_key() != "server")
			continue;

		const auto id = section.get("name").get_value();

		try {
			auto server = server_util::from_config(bot_.get_service(), section);

			if (has(server->get_id()))
				throw server_error(server_error::already_exists);

			add(std::move(server));
		} catch (const std::exception& ex) {
			bot_.get_log().warning("server", id) << ex.what() << std::endl;
		}
	}
}

namespace logger {

auto type_traits<server>::get_category(const server&) -> std::string_view
{
	return "server";
}

auto type_traits<server>::get_component(const server& sv) -> std::string_view
{
	return sv.get_id();
}

} // !logger

} // !irccd::daemon
