/*
 * server_service.hpp -- server service
 *
 * Copyright (c) 2013-2017 David Demelier <markand@malikania.fr>
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

#include "irccd.hpp"
#include "logger.hpp"
#include "plugin_service.hpp"
#include "rule_service.hpp"
#include "server_service.hpp"
#include "string_util.hpp"
#include "transport_service.hpp"

namespace irccd {

namespace {

template <typename EventNameFunc, typename ExecFunc>
void dispatch(irccd& daemon,
              const std::string& server,
              const std::string& origin,
              const std::string& target,
              EventNameFunc&& name_func,
              ExecFunc exec_func)
{
    for (auto& plugin : daemon.plugins().list()) {
        auto eventname = name_func(*plugin);
        auto allowed = daemon.rules().solve(server, target, origin, plugin->name(), eventname);

        if (!allowed) {
            log::debug() << "rule: event skipped on match" << std::endl;
            continue;
        }

        log::debug() << "rule: event allowed" << std::endl;

        try {
            exec_func(*plugin);
        } catch (const std::exception& ex) {
            log::warning() << "plugin " << plugin->name() << ": error: " << ex.what() << std::endl;
        }
    }
}

} // !namespace

void server_service::handle_channel_mode(const channel_mode_event& ev)
{
    log::debug() << "server " << ev.server->name() << ": event onChannelMode:\n";
    log::debug() << "  origin: " << ev.origin << "\n";
    log::debug() << "  channel: " << ev.channel << "\n";
    log::debug() << "  mode: " << ev.mode << "\n";
    log::debug() << "  argument: " << ev.argument << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onChannelMode"     },
        { "server",     ev.server->name()   },
        { "origin",     ev.origin           },
        { "channel",    ev.channel          },
        { "mode",       ev.mode             },
        { "argument",   ev.argument         }
    }));

    dispatch(irccd_, ev.server->name(), ev.origin, ev.channel,
        [=] (plugin&) -> std::string {
            return "onChannelMode";
        },
        [=] (plugin& plugin) {
            plugin.on_channel_mode(irccd_, ev);
        }
    );
}

void server_service::handle_channel_notice(const channel_notice_event& ev)
{
    log::debug() << "server " << ev.server->name() << ": event onChannelNotice:\n";
    log::debug() << "  origin: " << ev.origin << "\n";
    log::debug() << "  channel: " << ev.channel << "\n";
    log::debug() << "  message: " << ev.message << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onChannelNotice"   },
        { "server",     ev.server->name()   },
        { "origin",     ev.origin           },
        { "channel",    ev.channel          },
        { "message",    ev.message          }
    }));

    dispatch(irccd_, ev.server->name(), ev.origin, ev.channel,
        [=] (plugin&) -> std::string {
            return "onChannelNotice";
        },
        [=] (plugin& plugin) {
            plugin.on_channel_notice(irccd_, ev);
        }
    );
}

void server_service::handle_connect(const connect_event& ev)
{
    log::debug() << "server " << ev.server->name() << ": event onConnect" << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onConnect"         },
        { "server",     ev.server->name()   }
    }));

    dispatch(irccd_, ev.server->name(), /* origin */ "", /* channel */ "",
        [=] (plugin&) -> std::string {
            return "onConnect";
        },
        [=] (plugin& plugin) {
            plugin.on_connect(irccd_, ev);
        }
    );
}

void server_service::handle_invite(const invite_event& ev)
{
    log::debug() << "server " << ev.server->name() << ": event onInvite:\n";
    log::debug() << "  origin: " << ev.origin << "\n";
    log::debug() << "  channel: " << ev.channel << "\n";
    log::debug() << "  target: " << ev.nickname << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onInvite"          },
        { "server",     ev.server->name()   },
        { "origin",     ev.origin           },
        { "channel",    ev.channel          }
    }));

    dispatch(irccd_, ev.server->name(), ev.origin, ev.channel,
        [=] (plugin&) -> std::string {
            return "onInvite";
        },
        [=] (plugin& plugin) {
            plugin.on_invite(irccd_, ev);
        }
    );
}

void server_service::handle_join(const join_event& ev)
{
    log::debug() << "server " << ev.server->name() << ": event onJoin:\n";
    log::debug() << "  origin: " << ev.origin << "\n";
    log::debug() << "  channel: " << ev.channel << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onJoin"            },
        { "server",     ev.server->name()   },
        { "origin",     ev.origin           },
        { "channel",    ev.channel          }
    }));

    dispatch(irccd_, ev.server->name(), ev.origin, ev.channel,
        [=] (plugin&) -> std::string {
            return "onJoin";
        },
        [=] (plugin& plugin) {
            plugin.on_join(irccd_, ev);
        }
    );
}

void server_service::handle_kick(const kick_event& ev)
{
    log::debug() << "server " << ev.server->name() << ": event onKick:\n";
    log::debug() << "  origin: " << ev.origin << "\n";
    log::debug() << "  channel: " << ev.channel << "\n";
    log::debug() << "  target: " << ev.target << "\n";
    log::debug() << "  reason: " << ev.reason << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onKick"            },
        { "server",     ev.server->name()   },
        { "origin",     ev.origin           },
        { "channel",    ev.channel          },
        { "target",     ev.target           },
        { "reason",     ev.reason           }
    }));

    dispatch(irccd_, ev.server->name(), ev.origin, ev.channel,
        [=] (plugin&) -> std::string {
            return "onKick";
        },
        [=] (plugin& plugin) {
            plugin.on_kick(irccd_, ev);
        }
    );
}

void server_service::handle_message(const message_event& ev)
{
    log::debug() << "server " << ev.server->name() << ": event onMessage:\n";
    log::debug() << "  origin: " << ev.origin << "\n";
    log::debug() << "  channel: " << ev.channel << "\n";
    log::debug() << "  message: " << ev.message << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onMessage"         },
        { "server",     ev.server->name()   },
        { "origin",     ev.origin           },
        { "channel",    ev.channel          },
        { "message",    ev.message          }
    }));

    dispatch(irccd_, ev.server->name(), ev.origin, ev.channel,
        [=] (plugin& plugin) -> std::string {
            return string_util::parse_message(
                ev.message,
                ev.server->command_char(),
                plugin.name()
            ).type == string_util::message_pack::type::command ? "onCommand" : "onMessage";
        },
        [=] (plugin& plugin) mutable {
            auto copy = ev;
            auto pack = string_util::parse_message(copy.message, copy.server->command_char(), plugin.name());

            copy.message = pack.message;

            if (pack.type == string_util::message_pack::type::command)
                plugin.on_command(irccd_, copy);
            else
                plugin.on_message(irccd_, copy);
        }
    );
}

void server_service::handle_me(const me_event& ev)
{
    log::debug() << "server " << ev.server->name() << ": event onMe:\n";
    log::debug() << "  origin: " << ev.origin << "\n";
    log::debug() << "  target: " << ev.channel << "\n";
    log::debug() << "  message: " << ev.message << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onMe"              },
        { "server",     ev.server->name()   },
        { "origin",     ev.origin           },
        { "target",     ev.channel          },
        { "message",    ev.message          }
    }));

    dispatch(irccd_, ev.server->name(), ev.origin, ev.channel,
        [=] (plugin&) -> std::string {
            return "onMe";
        },
        [=] (plugin& plugin) {
            plugin.on_me(irccd_, ev);
        }
    );
}

void server_service::handle_mode(const mode_event& ev)
{
    log::debug() << "server " << ev.server->name() << ": event onMode\n";
    log::debug() << "  origin: " << ev.origin << "\n";
    log::debug() << "  mode: " << ev.mode << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onMode"            },
        { "server",     ev.server->name()   },
        { "origin",     ev.origin           },
        { "mode",       ev.mode             }
    }));

    dispatch(irccd_, ev.server->name(), ev.origin, /* channel */ "",
        [=] (plugin &) -> std::string {
            return "onMode";
        },
        [=] (plugin &plugin) {
            plugin.on_mode(irccd_, ev);
        }
    );
}

void server_service::handle_names(const names_event& ev)
{
    log::debug() << "server " << ev.server->name() << ": event onNames:\n";
    log::debug() << "  channel: " << ev.channel << "\n";
    log::debug() << "  names: " << string_util::join(ev.names.begin(), ev.names.end(), ", ") << std::endl;

    auto names = nlohmann::json::array();

    for (const auto& v : ev.names)
        names.push_back(v);

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onNames"           },
        { "server",     ev.server->name()   },
        { "channel",    ev.channel          },
        { "names",      std::move(names)    }
    }));

    dispatch(irccd_, ev.server->name(), /* origin */ "", ev.channel,
        [=] (plugin&) -> std::string {
            return "onNames";
        },
        [=] (plugin& plugin) {
            plugin.on_names(irccd_, ev);
        }
    );
}

void server_service::handle_nick(const nick_event& ev)
{
    log::debug() << "server " << ev.server->name() << ": event onNick:\n";
    log::debug() << "  origin: " << ev.origin << "\n";
    log::debug() << "  nickname: " << ev.nickname << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onNick"            },
        { "server",     ev.server->name()   },
        { "origin",     ev.origin           },
        { "nickname",   ev.nickname         }
    }));

    dispatch(irccd_, ev.server->name(), ev.origin, /* channel */ "",
        [=] (plugin&) -> std::string {
            return "onNick";
        },
        [=] (plugin& plugin) {
            plugin.on_nick(irccd_, ev);
        }
    );
}

void server_service::handle_notice(const notice_event& ev)
{
    log::debug() << "server " << ev.server->name() << ": event onNotice:\n";
    log::debug() << "  origin: " << ev.origin << "\n";
    log::debug() << "  message: " << ev.message << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onNotice"          },
        { "server",     ev.server->name()   },
        { "origin",     ev.origin           },
        { "message",    ev.message          }
    }));

    dispatch(irccd_, ev.server->name(), ev.origin, /* channel */ "",
        [=] (plugin&) -> std::string {
            return "onNotice";
        },
        [=] (plugin& plugin) {
            plugin.on_notice(irccd_, ev);
        }
    );
}

void server_service::handle_part(const part_event& ev)
{
    log::debug() << "server " << ev.server->name() << ": event onPart:\n";
    log::debug() << "  origin: " << ev.origin << "\n";
    log::debug() << "  channel: " << ev.channel << "\n";
    log::debug() << "  reason: " << ev.reason << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onPart"            },
        { "server",     ev.server->name()   },
        { "origin",     ev.origin           },
        { "channel",    ev.channel          },
        { "reason",     ev.reason           }
    }));

    dispatch(irccd_, ev.server->name(), ev.origin, ev.channel,
        [=] (plugin&) -> std::string {
            return "onPart";
        },
        [=] (plugin& plugin) {
            plugin.on_part(irccd_, ev);
        }
    );
}

void server_service::handle_query(const query_event& ev)
{
    log::debug() << "server " << ev.server->name() << ": event onQuery:\n";
    log::debug() << "  origin: " << ev.origin << "\n";
    log::debug() << "  message: " << ev.message << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onQuery"           },
        { "server",     ev.server->name()   },
        { "origin",     ev.origin           },
        { "message",    ev.message          }
    }));

    dispatch(irccd_, ev.server->name(), ev.origin, /* channel */ "",
        [=] (plugin& plugin) -> std::string {
            return string_util::parse_message(
                ev.message,
                ev.server->command_char(),
                plugin.name()
            ).type == string_util::message_pack::type::command ? "onQueryCommand" : "onQuery";
        },
        [=] (plugin& plugin) mutable {
            auto copy = ev;
            auto pack = string_util::parse_message(copy.message, copy.server->command_char(), plugin.name());

            copy.message = pack.message;

            if (pack.type == string_util::message_pack::type::command)
                plugin.on_query_command(irccd_, copy);
            else
                plugin.on_query(irccd_, copy);
        }
    );
}

void server_service::handle_topic(const topic_event& ev)
{
    log::debug() << "server " << ev.server->name() << ": event onTopic:\n";
    log::debug() << "  origin: " << ev.origin << "\n";
    log::debug() << "  channel: " << ev.channel << "\n";
    log::debug() << "  topic: " << ev.topic << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onTopic"           },
        { "server",     ev.server->name()   },
        { "origin",     ev.origin           },
        { "channel",    ev.channel          },
        { "topic",      ev.topic            }
    }));

    dispatch(irccd_, ev.server->name(), ev.origin, ev.channel,
        [=] (plugin&) -> std::string {
            return "onTopic";
        },
        [=] (plugin& plugin) {
            plugin.on_topic(irccd_, ev);
        }
    );
}

void server_service::handle_whois(const whois_event& ev)
{
    log::debug() << "server " << ev.server->name() << ": event onWhois\n";
    log::debug() << "  nickname: " << ev.whois.nick << "\n";
    log::debug() << "  username: " << ev.whois.user << "\n";
    log::debug() << "  host: " << ev.whois.host << "\n";
    log::debug() << "  realname: " << ev.whois.realname << "\n";
    log::debug() << "  channels: " << string_util::join(ev.whois.channels.begin(), ev.whois.channels.end(), ", ") << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onWhois"           },
        { "server",     ev.server->name()   },
        { "nickname",   ev.whois.nick       },
        { "username",   ev.whois.user       },
        { "host",       ev.whois.host       },
        { "realname",   ev.whois.realname   }
    }));

    dispatch(irccd_, ev.server->name(), /* origin */ "", /* channel */ "",
        [=] (plugin&) -> std::string {
            return "onWhois";
        },
        [=] (plugin& plugin) {
            plugin.on_whois(irccd_, ev);
        }
    );
}

server_service::server_service(irccd &irccd)
    : irccd_(irccd)
{
}

bool server_service::has(const std::string& name) const noexcept
{
    return std::count_if(servers_.cbegin(), servers_.end(), [&] (const auto& server) {
        return server->name() == name;
    }) > 0;
}

void server_service::add(std::shared_ptr<server> server)
{
    assert(!has(server->name()));

    std::weak_ptr<class server> ptr(server);

    server->on_channel_mode.connect(boost::bind(&server_service::handle_channel_mode, this, _1));
    server->on_channel_notice.connect(boost::bind(&server_service::handle_channel_notice, this, _1));
    server->on_connect.connect(boost::bind(&server_service::handle_connect, this, _1));
    server->on_invite.connect(boost::bind(&server_service::handle_invite, this, _1));
    server->on_join.connect(boost::bind(&server_service::handle_join, this, _1));
    server->on_kick.connect(boost::bind(&server_service::handle_kick, this, _1));
    server->on_message.connect(boost::bind(&server_service::handle_message, this, _1));
    server->on_me.connect(boost::bind(&server_service::handle_me, this, _1));
    server->on_mode.connect(boost::bind(&server_service::handle_mode, this, _1));
    server->on_names.connect(boost::bind(&server_service::handle_names, this, _1));
    server->on_nick.connect(boost::bind(&server_service::handle_nick, this, _1));
    server->on_notice.connect(boost::bind(&server_service::handle_notice, this, _1));
    server->on_part.connect(boost::bind(&server_service::handle_part, this, _1));
    server->on_query.connect(boost::bind(&server_service::handle_query, this, _1));
    server->on_topic.connect(boost::bind(&server_service::handle_topic, this, _1));
    server->on_whois.connect(boost::bind(&server_service::handle_whois, this, _1));
    server->on_die.connect([this, ptr] () {
        auto server = ptr.lock();

        if (server) {
            log::info(string_util::sprintf("server %s: removed", server->name()));
            servers_.erase(std::find(servers_.begin(), servers_.end(), server));
        }
    });

    server->connect();
    servers_.push_back(std::move(server));
}

std::shared_ptr<server> server_service::get(const std::string& name) const noexcept
{
    auto it = std::find_if(servers_.begin(), servers_.end(), [&] (const auto& server) {
        return server->name() == name;
    });

    if (it == servers_.end())
        return nullptr;

    return *it;
}

std::shared_ptr<server> server_service::require(const std::string& name) const
{
    auto server = get(name);

    if (!server)
        throw std::invalid_argument(string_util::sprintf("server %s not found", name));

    return server;
}

void server_service::remove(const std::string& name)
{
    auto it = std::find_if(servers_.begin(), servers_.end(), [&] (const auto& server) {
        return server->name() == name;
    });

    if (it != servers_.end()) {
        (*it)->disconnect();
        servers_.erase(it);
    }
}

void server_service::clear() noexcept
{
    for (auto &server : servers_)
        server->disconnect();

    servers_.clear();
}

} // !irccd
