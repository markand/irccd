/*
 * server_service.cpp -- server service
 *
 * Copyright (c) 2013-2018 David Demelier <markand@malikania.fr>
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

#include <irccd/json_util.hpp>
#include <irccd/string_util.hpp>

#include <irccd/daemon/irccd.hpp>
#include <irccd/daemon/logger.hpp>
#include <irccd/daemon/server_util.hpp>

#include <irccd/daemon/service/plugin_service.hpp>
#include <irccd/daemon/service/rule_service.hpp>
#include <irccd/daemon/service/server_service.hpp>
#include <irccd/daemon/service/transport_service.hpp>

namespace irccd {

namespace {

template <typename EventNameFunc, typename ExecFunc>
void dispatch(irccd& daemon,
              std::string_view server,
              std::string_view origin,
              std::string_view target,
              EventNameFunc&& name_func,
              ExecFunc exec_func)
{
    for (const auto& plugin : daemon.plugins().all()) {
        const auto eventname = name_func(*plugin);
        const auto allowed = daemon.rules().solve(server, target, origin, plugin->get_name(), eventname);

        if (!allowed) {
            daemon.get_log().debug("rule", "") << "event skipped on match" << std::endl;
            continue;
        }

        daemon.get_log().debug("rule", "") << "event allowed" << std::endl;

        try {
            exec_func(*plugin);
        } catch (const std::exception& ex) {
            daemon.get_log().warning(*plugin) << ex.what() << std::endl;
        }
    }
}

} // !namespace

void server_service::handle_connect(const connect_event& ev)
{
    irccd_.get_log().debug(*ev.server) << "event onConnect" << std::endl;
    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onConnect"             },
        { "server",     ev.server->get_name()   }
    }));

    dispatch(irccd_, ev.server->get_name(), /* origin */ "", /* channel */ "",
        [=] (plugin&) -> std::string {
            return "onConnect";
        },
        [=] (plugin& plugin) {
            plugin.handle_connect(irccd_, ev);
        }
    );
}

void server_service::handle_disconnect(const disconnect_event& ev)
{
    irccd_.get_log().debug(*ev.server) << "event onDisconnect" << std::endl;
    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onDisconnect"          },
        { "server",     ev.server->get_name()   }
    }));

    dispatch(irccd_, ev.server->get_name(), /* origin */ "", /* channel */ "",
        [=] (plugin&) -> std::string {
            return "onDisconnect";
        },
        [=] (plugin& plugin) {
            plugin.handle_disconnect(irccd_, ev);
        }
    );
}

void server_service::handle_die(const disconnect_event& ev)
{
    servers_.erase(std::find(servers_.begin(), servers_.end(), ev.server));
}

void server_service::handle_invite(const invite_event& ev)
{
    irccd_.get_log().debug(*ev.server) << "event onInvite:" << std::endl;
    irccd_.get_log().debug(*ev.server) << "  origin: " << ev.origin << std::endl;
    irccd_.get_log().debug(*ev.server) << "  channel: " << ev.channel << std::endl;
    irccd_.get_log().debug(*ev.server) << "  target: " << ev.nickname << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onInvite"              },
        { "server",     ev.server->get_name()   },
        { "origin",     ev.origin               },
        { "channel",    ev.channel              }
    }));

    dispatch(irccd_, ev.server->get_name(), ev.origin, ev.channel,
        [=] (plugin&) -> std::string {
            return "onInvite";
        },
        [=] (plugin& plugin) {
            plugin.handle_invite(irccd_, ev);
        }
    );
}

void server_service::handle_join(const join_event& ev)
{
    irccd_.get_log().debug(*ev.server) << "event onJoin:" << std::endl;
    irccd_.get_log().debug(*ev.server) << "  origin: " << ev.origin << std::endl;
    irccd_.get_log().debug(*ev.server) << "  channel: " << ev.channel << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onJoin"                },
        { "server",     ev.server->get_name()   },
        { "origin",     ev.origin               },
        { "channel",    ev.channel              }
    }));

    dispatch(irccd_, ev.server->get_name(), ev.origin, ev.channel,
        [=] (plugin&) -> std::string {
            return "onJoin";
        },
        [=] (plugin& plugin) {
            plugin.handle_join(irccd_, ev);
        }
    );
}

void server_service::handle_kick(const kick_event& ev)
{
    irccd_.get_log().debug(*ev.server) << "event onKick:" << std::endl;
    irccd_.get_log().debug(*ev.server) << "  origin: " << ev.origin << std::endl;
    irccd_.get_log().debug(*ev.server) << "  channel: " << ev.channel << std::endl;
    irccd_.get_log().debug(*ev.server) << "  target: " << ev.target << std::endl;
    irccd_.get_log().debug(*ev.server) << "  reason: " << ev.reason << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onKick"                },
        { "server",     ev.server->get_name()   },
        { "origin",     ev.origin               },
        { "channel",    ev.channel              },
        { "target",     ev.target               },
        { "reason",     ev.reason               }
    }));

    dispatch(irccd_, ev.server->get_name(), ev.origin, ev.channel,
        [=] (plugin&) -> std::string {
            return "onKick";
        },
        [=] (plugin& plugin) {
            plugin.handle_kick(irccd_, ev);
        }
    );
}

void server_service::handle_message(const message_event& ev)
{
    irccd_.get_log().debug(*ev.server) << "event onMessage:" << std::endl;
    irccd_.get_log().debug(*ev.server) << "  origin: " << ev.origin << std::endl;
    irccd_.get_log().debug(*ev.server) << "  channel: " << ev.channel << std::endl;
    irccd_.get_log().debug(*ev.server) << "  message: " << ev.message << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onMessage"             },
        { "server",     ev.server->get_name()   },
        { "origin",     ev.origin               },
        { "channel",    ev.channel              },
        { "message",    ev.message              }
    }));

    dispatch(irccd_, ev.server->get_name(), ev.origin, ev.channel,
        [=] (plugin& plugin) -> std::string {
            return server_util::parse_message(
                ev.message,
                ev.server->get_command_char(),
                plugin.get_name()
            ).type == server_util::message_pack::type::command ? "onCommand" : "onMessage";
        },
        [=] (plugin& plugin) mutable {
            auto copy = ev;
            auto pack = server_util::parse_message(
                copy.message,
                copy.server->get_command_char(),
                plugin.get_name()
            );

            copy.message = pack.message;

            if (pack.type == server_util::message_pack::type::command)
                plugin.handle_command(irccd_, copy);
            else
                plugin.handle_message(irccd_, copy);
        }
    );
}

void server_service::handle_me(const me_event& ev)
{
    irccd_.get_log().debug(*ev.server) << "event onMe:" << std::endl;
    irccd_.get_log().debug(*ev.server) << "  origin: " << ev.origin << std::endl;
    irccd_.get_log().debug(*ev.server) << "  target: " << ev.channel << std::endl;
    irccd_.get_log().debug(*ev.server) << "  message: " << ev.message << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onMe"                  },
        { "server",     ev.server->get_name()   },
        { "origin",     ev.origin               },
        { "target",     ev.channel              },
        { "message",    ev.message              }
    }));

    dispatch(irccd_, ev.server->get_name(), ev.origin, ev.channel,
        [=] (plugin&) -> std::string {
            return "onMe";
        },
        [=] (plugin& plugin) {
            plugin.handle_me(irccd_, ev);
        }
    );
}

void server_service::handle_mode(const mode_event& ev)
{
    irccd_.get_log().debug(*ev.server) << "event onMode" << std::endl;
    irccd_.get_log().debug(*ev.server) << "  origin: " << ev.origin << std::endl;
    irccd_.get_log().debug(*ev.server) << "  channel: " << ev.channel << std::endl;
    irccd_.get_log().debug(*ev.server) << "  mode: " << ev.mode << std::endl;
    irccd_.get_log().debug(*ev.server) << "  limit: " << ev.limit << std::endl;
    irccd_.get_log().debug(*ev.server) << "  user: " << ev.user << std::endl;
    irccd_.get_log().debug(*ev.server) << "  mask: " << ev.mask << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onMode"                },
        { "server",     ev.server->get_name()   },
        { "origin",     ev.origin               },
        { "channel",    ev.channel              },
        { "mode",       ev.mode                 },
        { "limit",      ev.limit                },
        { "user",       ev.user                 },
        { "mask",       ev.mask                 }
    }));

    dispatch(irccd_, ev.server->get_name(), ev.origin, /* channel */ "",
        [=] (plugin &) -> std::string {
            return "onMode";
        },
        [=] (plugin &plugin) {
            plugin.handle_mode(irccd_, ev);
        }
    );
}

void server_service::handle_names(const names_event& ev)
{
    irccd_.get_log().debug(*ev.server) << "event onNames:" << std::endl;
    irccd_.get_log().debug(*ev.server) << "  channel: " << ev.channel << std::endl;
    irccd_.get_log().debug(*ev.server) << "  names: " << string_util::join(ev.names.begin(), ev.names.end(), ", ") << std::endl;

    auto names = nlohmann::json::array();

    for (const auto& v : ev.names)
        names.push_back(v);

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onNames"               },
        { "server",     ev.server->get_name()   },
        { "channel",    ev.channel              },
        { "names",      std::move(names)        }
    }));

    dispatch(irccd_, ev.server->get_name(), /* origin */ "", ev.channel,
        [=] (plugin&) -> std::string {
            return "onNames";
        },
        [=] (plugin& plugin) {
            plugin.handle_names(irccd_, ev);
        }
    );
}

void server_service::handle_nick(const nick_event& ev)
{
    irccd_.get_log().debug(*ev.server) << "event onNick:" << std::endl;
    irccd_.get_log().debug(*ev.server) << "  origin: " << ev.origin << std::endl;
    irccd_.get_log().debug(*ev.server) << "  nickname: " << ev.nickname << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onNick"                },
        { "server",     ev.server->get_name()   },
        { "origin",     ev.origin               },
        { "nickname",   ev.nickname             }
    }));

    dispatch(irccd_, ev.server->get_name(), ev.origin, /* channel */ "",
        [=] (plugin&) -> std::string {
            return "onNick";
        },
        [=] (plugin& plugin) {
            plugin.handle_nick(irccd_, ev);
        }
    );
}

void server_service::handle_notice(const notice_event& ev)
{
    irccd_.get_log().debug(*ev.server) << "event onNotice:" << std::endl;
    irccd_.get_log().debug(*ev.server) << "  origin: " << ev.origin << std::endl;
    irccd_.get_log().debug(*ev.server) << "  channel: " << ev.channel << std::endl;
    irccd_.get_log().debug(*ev.server) << "  message: " << ev.message << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onNotice"              },
        { "server",     ev.server->get_name()   },
        { "origin",     ev.origin               },
        { "channel",    ev.channel              },
        { "message",    ev.message              }
    }));

    dispatch(irccd_, ev.server->get_name(), ev.origin, /* channel */ "",
        [=] (plugin&) -> std::string {
            return "onNotice";
        },
        [=] (plugin& plugin) {
            plugin.handle_notice(irccd_, ev);
        }
    );
}

void server_service::handle_part(const part_event& ev)
{
    irccd_.get_log().debug(*ev.server) << "event onPart:" << std::endl;
    irccd_.get_log().debug(*ev.server) << "  origin: " << ev.origin << std::endl;
    irccd_.get_log().debug(*ev.server) << "  channel: " << ev.channel << std::endl;
    irccd_.get_log().debug(*ev.server) << "  reason: " << ev.reason << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onPart"                },
        { "server",     ev.server->get_name()   },
        { "origin",     ev.origin               },
        { "channel",    ev.channel              },
        { "reason",     ev.reason               }
    }));

    dispatch(irccd_, ev.server->get_name(), ev.origin, ev.channel,
        [=] (plugin&) -> std::string {
            return "onPart";
        },
        [=] (plugin& plugin) {
            plugin.handle_part(irccd_, ev);
        }
    );
}

void server_service::handle_topic(const topic_event& ev)
{
    irccd_.get_log().debug(*ev.server) << "event onTopic:" << std::endl;
    irccd_.get_log().debug(*ev.server) << "  origin: " << ev.origin << std::endl;
    irccd_.get_log().debug(*ev.server) << "  channel: " << ev.channel << std::endl;
    irccd_.get_log().debug(*ev.server) << "  topic: " << ev.topic << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onTopic"               },
        { "server",     ev.server->get_name()   },
        { "origin",     ev.origin               },
        { "channel",    ev.channel              },
        { "topic",      ev.topic                }
    }));

    dispatch(irccd_, ev.server->get_name(), ev.origin, ev.channel,
        [=] (plugin&) -> std::string {
            return "onTopic";
        },
        [=] (plugin& plugin) {
            plugin.handle_topic(irccd_, ev);
        }
    );
}

void server_service::handle_whois(const whois_event& ev)
{
    irccd_.get_log().debug(*ev.server) << "event onWhois" << std::endl;
    irccd_.get_log().debug(*ev.server) << "  nickname: " << ev.whois.nick << std::endl;
    irccd_.get_log().debug(*ev.server) << "  username: " << ev.whois.user << std::endl;
    irccd_.get_log().debug(*ev.server) << "  host: " << ev.whois.host << std::endl;
    irccd_.get_log().debug(*ev.server) << "  realname: " << ev.whois.realname << std::endl;
    irccd_.get_log().debug(*ev.server) << "  channels: " << string_util::join(ev.whois.channels, ", ") << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onWhois"               },
        { "server",     ev.server->get_name()   },
        { "nickname",   ev.whois.nick           },
        { "username",   ev.whois.user           },
        { "host",       ev.whois.host           },
        { "realname",   ev.whois.realname       }
    }));

    dispatch(irccd_, ev.server->get_name(), /* origin */ "", /* channel */ "",
        [=] (plugin&) -> std::string {
            return "onWhois";
        },
        [=] (plugin& plugin) {
            plugin.handle_whois(irccd_, ev);
        }
    );
}

server_service::server_service(irccd &irccd)
    : irccd_(irccd)
{
}

bool server_service::has(const std::string& name) const noexcept
{
    return std::count_if(servers_.begin(), servers_.end(), [&] (const auto& server) {
        return server->get_name() == name;
    }) > 0;
}

void server_service::add(std::shared_ptr<server> server)
{
    assert(!has(server->get_name()));

    server->on_connect.connect(boost::bind(&server_service::handle_connect, this, _1));
    server->on_disconnect.connect(boost::bind(&server_service::handle_disconnect, this, _1));
    server->on_die.connect(boost::bind(&server_service::handle_die, this, _1));
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
    server->on_topic.connect(boost::bind(&server_service::handle_topic, this, _1));
    server->on_whois.connect(boost::bind(&server_service::handle_whois, this, _1));
    server->connect();
    servers_.push_back(std::move(server));
}

std::shared_ptr<server> server_service::get(const std::string& name) const noexcept
{
    const auto it = std::find_if(servers_.begin(), servers_.end(), [&] (const auto& server) {
        return server->get_name() == name;
    });

    if (it == servers_.end())
        return nullptr;

    return *it;
}

std::shared_ptr<server> server_service::require(const std::string& name) const
{
    if (!string_util::is_identifier(name))
        throw server_error(server_error::invalid_identifier);

    const auto s = get(name);

    if (!s)
        throw server_error(server_error::not_found);

    return s;
}

void server_service::remove(const std::string& name)
{
    const auto it = std::find_if(servers_.begin(), servers_.end(), [&] (const auto& server) {
        return server->get_name() == name;
    });

    if (it != servers_.end()) {
        (*it)->disconnect();
        servers_.erase(it);
    }
}

void server_service::clear() noexcept
{
    /*
     * Copy the array, because disconnect() may trigger on_die signal which
     * erase the server from itself.
     */
    const auto save = servers_;

    for (const auto& server : save)
        server->disconnect();

    servers_.clear();
}

void server_service::load(const config& cfg) noexcept
{
    for (const auto& section : cfg) {
        if (section.key() != "server")
            continue;

        const auto id = section.get("name").value();

        try {
            auto server = server_util::from_config(irccd_.get_service(), cfg, section);

            if (has(server->get_name()))
                throw server_error(server_error::already_exists);

            add(std::move(server));
        } catch (const std::exception& ex) {
            irccd_.get_log().warning("server", id) << ex.what() << std::endl;
        }
    }
}

namespace logger {

auto loggable_traits<server>::get_category(const server&) -> std::string_view
{
    return "server";
}

auto loggable_traits<server>::get_component(const server& sv) -> std::string_view
{
    // TODO: get id instead.
    return sv.get_name();
}

} // !logger

} // !irccd
