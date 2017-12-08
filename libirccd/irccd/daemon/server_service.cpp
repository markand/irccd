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

#include <irccd/json_util.hpp>
#include <irccd/string_util.hpp>

#include "irccd.hpp"
#include "logger.hpp"
#include "plugin_service.hpp"
#include "rule_service.hpp"
#include "server_service.hpp"
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
            daemon.log().debug("rule: event skipped on match");
            continue;
        }

        daemon.log().debug("rule: event allowed");

        try {
            exec_func(*plugin);
        } catch (const std::exception& ex) {
            daemon.log().warning() << "plugin " << plugin->name() << ": error: "
                << ex.what() << std::endl;
        }
    }
}

template <typename T>
T to_int(const std::string& value, const std::string& name, server_error::error errc)
{
    try {
        return string_util::to_int<T>(value);
    } catch (...) {
        throw server_error(errc, name);
    }
}

template <typename T>
T to_uint(const std::string& value, const std::string& name, server_error::error errc)
{
    try {
        return string_util::to_uint<T>(value);
    } catch (...) {
        throw server_error(errc, name);
    }
}

template <typename T>
T to_uint(const nlohmann::json& value, const std::string& name, server_error::error errc)
{
    if (!value.is_number())
        throw server_error(errc, name);

    auto n = value.get<unsigned>();

    if (n > std::numeric_limits<T>::max())
        throw server_error(errc, name);

    return static_cast<T>(n);
}

std::string to_id(const ini::section& sc)
{
    auto id = sc.get("name");

    if (!string_util::is_identifier(id.value()))
        throw server_error(server_error::invalid_identifier, "");

    return id.value();
}

std::string to_id(const nlohmann::json& object)
{
    auto id = json_util::get_string(object, "name");

    if (!string_util::is_identifier(id))
        throw server_error(server_error::invalid_identifier, "");

    return id;
}

std::string to_host(const ini::section& sc, const std::string& name)
{
    auto value = sc.get("host");

    if (value.empty())
        throw server_error(server_error::invalid_hostname, name);

    return value.value();
}

std::string to_host(const nlohmann::json& object, const std::string& name)
{
    auto value = json_util::get_string(object, "host");

    if (value.empty())
        throw server_error(server_error::invalid_hostname, name);

    return value;
}

void load_server_identity(std::shared_ptr<server>& server,
                          const config& cfg,
                          const std::string& identity)
{
    auto sc = std::find_if(cfg.doc().begin(), cfg.doc().end(), [&] (const auto& sc) {
        if (sc.key() != "identity")
            return false;

        auto name = sc.find("name");

        return name != sc.end() && name->value() == identity;
    });

    if (sc == cfg.doc().end())
        return;

    ini::section::const_iterator it;

    if ((it = sc->find("username")) != sc->end())
        server->set_username(it->value());
    if ((it = sc->find("realname")) != sc->end())
        server->set_realname(it->value());
    if ((it = sc->find("nickname")) != sc->end())
        server->set_nickname(it->value());
    if ((it = sc->find("ctcp-version")) != sc->end())
        server->set_ctcp_version(it->value());
}

std::shared_ptr<server> load_server(boost::asio::io_service& service,
                                    const config& cfg,
                                    const ini::section& sc)
{
    assert(sc.key() == "server");

    auto sv = std::make_shared<server>(service, to_id(sc));

    // Mandatory fields.
    sv->set_host(to_host(sc, sv->name()));

    // Optional fields.
    ini::section::const_iterator it;

    if ((it = sc.find("password")) != sc.end())
        sv->set_password(it->value());

    // Optional flags
    if ((it = sc.find("ipv6")) != sc.end() && string_util::is_boolean(it->value()))
        sv->set_flags(sv->flags() | server::ipv6);

    if ((it = sc.find("ssl")) != sc.end() && string_util::is_boolean(it->value())) {
#if defined(HAVE_SSL)
        sv->set_flags(sv->flags() | server::ssl);
#else
        throw server_error(server_error::ssl_disabled, sv->name());
#endif
    }

    if ((it = sc.find("ssl-verify")) != sc.end() && string_util::is_boolean(it->value()))
        sv->set_flags(sv->flags() | server::ssl_verify);

    // Optional identity
    if ((it = sc.find("identity")) != sc.end())
        load_server_identity(sv, cfg, it->value());

    // Options
    if ((it = sc.find("auto-rejoin")) != sc.end() && string_util::is_boolean(it->value()))
        sv->set_flags(sv->flags() | server::auto_rejoin);
    if ((it = sc.find("join-invite")) != sc.end() && string_util::is_boolean(it->value()))
        sv->set_flags(sv->flags() | server::join_invite);

    // Channels
    if ((it = sc.find("channels")) != sc.end()) {
        for (const auto& s : *it) {
            channel channel;

            if (auto pos = s.find(":") != std::string::npos) {
                channel.name = s.substr(0, pos);
                channel.password = s.substr(pos + 1);
            } else
                channel.name = s;

            sv->join(channel.name, channel.password);
        }
    }
    if ((it = sc.find("command-char")) != sc.end())
        sv->set_command_char(it->value());

    // Reconnect and ping timeout
    if ((it = sc.find("port")) != sc.end())
        sv->set_port(to_uint<std::uint16_t>(it->value(),
            sv->name(), server_error::invalid_port));

    if ((it = sc.find("reconnect-tries")) != sc.end())
        sv->set_reconnect_tries(to_int<std::int8_t>(it->value(),
            sv->name(), server_error::invalid_reconnect_tries));

    if ((it = sc.find("reconnect-timeout")) != sc.end())
        sv->set_reconnect_delay(to_uint<std::uint16_t>(it->value(),
            sv->name(), server_error::invalid_reconnect_timeout));

    if ((it = sc.find("ping-timeout")) != sc.end())
        sv->set_ping_timeout(to_uint<std::uint16_t>(it->value(),
            sv->name(), server_error::invalid_ping_timeout));

    return sv;
}

} // !namespace

void server_service::handle_connect(const connect_event& ev)
{
    irccd_.log().debug() << "server " << ev.server->name() << ": event onConnect" << std::endl;

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
    irccd_.log().debug() << "server " << ev.server->name() << ": event onInvite:\n";
    irccd_.log().debug() << "  origin: " << ev.origin << "\n";
    irccd_.log().debug() << "  channel: " << ev.channel << "\n";
    irccd_.log().debug() << "  target: " << ev.nickname << std::endl;

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
    irccd_.log().debug() << "server " << ev.server->name() << ": event onJoin:\n";
    irccd_.log().debug() << "  origin: " << ev.origin << "\n";
    irccd_.log().debug() << "  channel: " << ev.channel << std::endl;

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
    irccd_.log().debug() << "server " << ev.server->name() << ": event onKick:\n";
    irccd_.log().debug() << "  origin: " << ev.origin << "\n";
    irccd_.log().debug() << "  channel: " << ev.channel << "\n";
    irccd_.log().debug() << "  target: " << ev.target << "\n";
    irccd_.log().debug() << "  reason: " << ev.reason << std::endl;

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
    irccd_.log().debug() << "server " << ev.server->name() << ": event onMessage:\n";
    irccd_.log().debug() << "  origin: " << ev.origin << "\n";
    irccd_.log().debug() << "  channel: " << ev.channel << "\n";
    irccd_.log().debug() << "  message: " << ev.message << std::endl;

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
    irccd_.log().debug() << "server " << ev.server->name() << ": event onMe:\n";
    irccd_.log().debug() << "  origin: " << ev.origin << "\n";
    irccd_.log().debug() << "  target: " << ev.channel << "\n";
    irccd_.log().debug() << "  message: " << ev.message << std::endl;

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
    irccd_.log().debug() << "server " << ev.server->name() << ": event onMode\n";
    irccd_.log().debug() << "  origin: " << ev.origin << "\n";
    irccd_.log().debug() << "  channel: " << ev.channel << "\n";
    irccd_.log().debug() << "  mode: " << ev.mode << "\n";
    irccd_.log().debug() << "  limit: " << ev.limit << "\n";
    irccd_.log().debug() << "  user: " << ev.user << "\n";
    irccd_.log().debug() << "  mask: " << ev.mask << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onMode"            },
        { "server",     ev.server->name()   },
        { "origin",     ev.origin           },
        { "channel",    ev.channel          },
        { "mode",       ev.mode             },
        { "limit",      ev.limit            },
        { "user",       ev.user             },
        { "mask",       ev.mask             }
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
    irccd_.log().debug() << "server " << ev.server->name() << ": event onNames:\n";
    irccd_.log().debug() << "  channel: " << ev.channel << "\n";
    irccd_.log().debug() << "  names: " << string_util::join(ev.names.begin(), ev.names.end(), ", ") << std::endl;

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
    irccd_.log().debug() << "server " << ev.server->name() << ": event onNick:\n";
    irccd_.log().debug() << "  origin: " << ev.origin << "\n";
    irccd_.log().debug() << "  nickname: " << ev.nickname << std::endl;

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
    irccd_.log().debug() << "server " << ev.server->name() << ": event onNotice:\n";
    irccd_.log().debug() << "  origin: " << ev.origin << "\n";
    irccd_.log().debug() << "  channel: " << ev.channel << "\n";
    irccd_.log().debug() << "  message: " << ev.message << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onNotice"          },
        { "server",     ev.server->name()   },
        { "origin",     ev.origin           },
        { "channel",    ev.channel          },
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
    irccd_.log().debug() << "server " << ev.server->name() << ": event onPart:\n";
    irccd_.log().debug() << "  origin: " << ev.origin << "\n";
    irccd_.log().debug() << "  channel: " << ev.channel << "\n";
    irccd_.log().debug() << "  reason: " << ev.reason << std::endl;

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

void server_service::handle_topic(const topic_event& ev)
{
    irccd_.log().debug() << "server " << ev.server->name() << ": event onTopic:\n";
    irccd_.log().debug() << "  origin: " << ev.origin << "\n";
    irccd_.log().debug() << "  channel: " << ev.channel << "\n";
    irccd_.log().debug() << "  topic: " << ev.topic << std::endl;

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
    irccd_.log().debug() << "server " << ev.server->name() << ": event onWhois\n";
    irccd_.log().debug() << "  nickname: " << ev.whois.nick << "\n";
    irccd_.log().debug() << "  username: " << ev.whois.user << "\n";
    irccd_.log().debug() << "  host: " << ev.whois.host << "\n";
    irccd_.log().debug() << "  realname: " << ev.whois.realname << "\n";
    irccd_.log().debug() << "  channels: " << string_util::join(ev.whois.channels, ", ") << std::endl;

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

std::shared_ptr<server> server_service::from_json(boost::asio::io_service& service, const nlohmann::json& object)
{
    // TODO: move this function in server_service.
    auto sv = std::make_shared<server>(service, to_id(object));

    // Mandatory fields.
    sv->set_host(to_host(object, sv->name()));

    // Optional fields.
    if (object.count("port"))
        sv->set_port(to_uint<std::uint16_t>(object["port"], sv->name(), server_error::invalid_port));
    sv->set_password(json_util::get_string(object, "password"));
    sv->set_nickname(json_util::get_string(object, "nickname", sv->nickname()));
    sv->set_realname(json_util::get_string(object, "realname", sv->realname()));
    sv->set_username(json_util::get_string(object, "username", sv->username()));
    sv->set_ctcp_version(json_util::get_string(object, "ctcpVersion", sv->ctcp_version()));
    sv->set_command_char(json_util::get_string(object, "commandChar", sv->command_char()));

    if (json_util::get_bool(object, "ipv6"))
        sv->set_flags(sv->flags() | server::ipv6);
    if (json_util::get_bool(object, "sslVerify"))
        sv->set_flags(sv->flags() | server::ssl_verify);
    if (json_util::get_bool(object, "autoRejoin"))
        sv->set_flags(sv->flags() | server::auto_rejoin);
    if (json_util::get_bool(object, "joinInvite"))
        sv->set_flags(sv->flags() | server::join_invite);

    if (json_util::get_bool(object, "ssl"))
#if defined(HAVE_SSL)
        sv->set_flags(sv->flags() | server::ssl);
#else
        throw server_error(server_error::ssl_disabled, sv->name());
#endif

    return sv;
}

server_service::server_service(irccd &irccd)
    : irccd_(irccd)
{
}

bool server_service::has(const std::string& name) const noexcept
{
    return std::count_if(servers_.begin(), servers_.end(), [&] (const auto& server) {
        return server->name() == name;
    }) > 0;
}

void server_service::add(std::shared_ptr<server> server)
{
    assert(!has(server->name()));

    std::weak_ptr<class server> ptr(server);

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
    server->on_topic.connect(boost::bind(&server_service::handle_topic, this, _1));
    server->on_whois.connect(boost::bind(&server_service::handle_whois, this, _1));
    server->on_die.connect([this, ptr] () {
        auto server = ptr.lock();

        if (server) {
            irccd_.log().info(string_util::sprintf("server %s: removed", server->name()));
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

void server_service::load(const config& cfg) noexcept
{
    for (const auto& section : cfg.doc()) {
        if (section.key() != "server")
            continue;

        try {
            add(load_server(irccd_.service(), cfg, section));
        } catch (const std::exception& ex) {
            irccd_.log().warning() << "server " << section.get("name").value() << ": "
                << ex.what() << std::endl;
        }
    }
}

} // !irccd
