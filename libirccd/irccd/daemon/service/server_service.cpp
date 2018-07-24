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

class dispatcher {
private:
    irccd& irccd_;

    template <typename EventNameFunc, typename ExecFunc>
    void dispatch(std::string_view, std::string_view, std::string_view, EventNameFunc&&, ExecFunc);

public:
    dispatcher(irccd& irccd);
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
    for (const auto& plugin : irccd_.plugins().all()) {
        const auto eventname = name_func(*plugin);
        const auto allowed = irccd_.rules().solve(server, target, origin, plugin->get_name(), eventname);

        if (!allowed) {
            irccd_.get_log().debug("rule", "") << "event skipped on match" << std::endl;
            continue;
        }

        irccd_.get_log().debug("rule", "") << "event allowed" << std::endl;

        try {
            exec_func(*plugin);
        } catch (const std::exception& ex) {
            irccd_.get_log().warning(*plugin) << ex.what() << std::endl;
        }
    }
}

dispatcher::dispatcher(irccd& irccd)
    : irccd_(irccd)
{
}

void dispatcher::operator()(const std::monostate&)
{
}

void dispatcher::operator()(const connect_event& ev)
{
    irccd_.get_log().debug(*ev.server) << "event onConnect" << std::endl;
    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onConnect"         },
        { "server",     ev.server->get_id() }
    }));

    dispatch(ev.server->get_id(), /* origin */ "", /* channel */ "",
        [=] (plugin&) -> std::string {
            return "onConnect";
        },
        [=] (plugin& plugin) {
            plugin.handle_connect(irccd_, ev);
        }
    );
}

void dispatcher::operator()(const disconnect_event& ev)
{
    irccd_.get_log().debug(*ev.server) << "event onDisconnect" << std::endl;
    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onDisconnect"      },
        { "server",     ev.server->get_id() }
    }));

    dispatch(ev.server->get_id(), /* origin */ "", /* channel */ "",
        [=] (plugin&) -> std::string {
            return "onDisconnect";
        },
        [=] (plugin& plugin) {
            plugin.handle_disconnect(irccd_, ev);
        }
    );
}

void dispatcher::operator()(const invite_event& ev)
{
    irccd_.get_log().debug(*ev.server) << "event onInvite:" << std::endl;
    irccd_.get_log().debug(*ev.server) << "  origin: " << ev.origin << std::endl;
    irccd_.get_log().debug(*ev.server) << "  channel: " << ev.channel << std::endl;
    irccd_.get_log().debug(*ev.server) << "  target: " << ev.nickname << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onInvite"          },
        { "server",     ev.server->get_id() },
        { "origin",     ev.origin           },
        { "channel",    ev.channel          }
    }));

    dispatch(ev.server->get_id(), ev.origin, ev.channel,
        [=] (plugin&) -> std::string {
            return "onInvite";
        },
        [=] (plugin& plugin) {
            plugin.handle_invite(irccd_, ev);
        }
    );
}

void dispatcher::operator()(const join_event& ev)
{
    irccd_.get_log().debug(*ev.server) << "event onJoin:" << std::endl;
    irccd_.get_log().debug(*ev.server) << "  origin: " << ev.origin << std::endl;
    irccd_.get_log().debug(*ev.server) << "  channel: " << ev.channel << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onJoin"            },
        { "server",     ev.server->get_id() },
        { "origin",     ev.origin           },
        { "channel",    ev.channel          }
    }));

    dispatch(ev.server->get_id(), ev.origin, ev.channel,
        [=] (plugin&) -> std::string {
            return "onJoin";
        },
        [=] (plugin& plugin) {
            plugin.handle_join(irccd_, ev);
        }
    );
}

void dispatcher::operator()(const kick_event& ev)
{
    irccd_.get_log().debug(*ev.server) << "event onKick:" << std::endl;
    irccd_.get_log().debug(*ev.server) << "  origin: " << ev.origin << std::endl;
    irccd_.get_log().debug(*ev.server) << "  channel: " << ev.channel << std::endl;
    irccd_.get_log().debug(*ev.server) << "  target: " << ev.target << std::endl;
    irccd_.get_log().debug(*ev.server) << "  reason: " << ev.reason << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onKick"            },
        { "server",     ev.server->get_id() },
        { "origin",     ev.origin           },
        { "channel",    ev.channel          },
        { "target",     ev.target           },
        { "reason",     ev.reason           }
    }));

    dispatch(ev.server->get_id(), ev.origin, ev.channel,
        [=] (plugin&) -> std::string {
            return "onKick";
        },
        [=] (plugin& plugin) {
            plugin.handle_kick(irccd_, ev);
        }
    );
}

void dispatcher::operator()(const message_event& ev)
{
    irccd_.get_log().debug(*ev.server) << "event onMessage:" << std::endl;
    irccd_.get_log().debug(*ev.server) << "  origin: " << ev.origin << std::endl;
    irccd_.get_log().debug(*ev.server) << "  channel: " << ev.channel << std::endl;
    irccd_.get_log().debug(*ev.server) << "  message: " << ev.message << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onMessage"         },
        { "server",     ev.server->get_id() },
        { "origin",     ev.origin           },
        { "channel",    ev.channel          },
        { "message",    ev.message          }
    }));

    dispatch(ev.server->get_id(), ev.origin, ev.channel,
        [=] (plugin& plugin) -> std::string {
            return server_util::parse_message(
                ev.message,
                ev.server->get_command_char(),
                plugin.get_id()
            ).type == server_util::message_pack::type::command ? "onCommand" : "onMessage";
        },
        [=] (plugin& plugin) mutable {
            auto copy = ev;
            auto pack = server_util::parse_message(
                copy.message,
                copy.server->get_command_char(),
                plugin.get_id()
            );

            copy.message = pack.message;

            if (pack.type == server_util::message_pack::type::command)
                plugin.handle_command(irccd_, copy);
            else
                plugin.handle_message(irccd_, copy);
        }
    );
}

void dispatcher::operator()(const me_event& ev)
{
    irccd_.get_log().debug(*ev.server) << "event onMe:" << std::endl;
    irccd_.get_log().debug(*ev.server) << "  origin: " << ev.origin << std::endl;
    irccd_.get_log().debug(*ev.server) << "  target: " << ev.channel << std::endl;
    irccd_.get_log().debug(*ev.server) << "  message: " << ev.message << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onMe"              },
        { "server",     ev.server->get_id() },
        { "origin",     ev.origin           },
        { "target",     ev.channel          },
        { "message",    ev.message          }
    }));

    dispatch(ev.server->get_id(), ev.origin, ev.channel,
        [=] (plugin&) -> std::string {
            return "onMe";
        },
        [=] (plugin& plugin) {
            plugin.handle_me(irccd_, ev);
        }
    );
}

void dispatcher::operator()(const mode_event& ev)
{
    irccd_.get_log().debug(*ev.server) << "event onMode" << std::endl;
    irccd_.get_log().debug(*ev.server) << "  origin: " << ev.origin << std::endl;
    irccd_.get_log().debug(*ev.server) << "  channel: " << ev.channel << std::endl;
    irccd_.get_log().debug(*ev.server) << "  mode: " << ev.mode << std::endl;
    irccd_.get_log().debug(*ev.server) << "  limit: " << ev.limit << std::endl;
    irccd_.get_log().debug(*ev.server) << "  user: " << ev.user << std::endl;
    irccd_.get_log().debug(*ev.server) << "  mask: " << ev.mask << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onMode"            },
        { "server",     ev.server->get_id() },
        { "origin",     ev.origin           },
        { "channel",    ev.channel          },
        { "mode",       ev.mode             },
        { "limit",      ev.limit            },
        { "user",       ev.user             },
        { "mask",       ev.mask             }
    }));

    dispatch(ev.server->get_id(), ev.origin, /* channel */ "",
        [=] (plugin &) -> std::string {
            return "onMode";
        },
        [=] (plugin &plugin) {
            plugin.handle_mode(irccd_, ev);
        }
    );
}

void dispatcher::operator()(const names_event& ev)
{
    irccd_.get_log().debug(*ev.server) << "event onNames:" << std::endl;
    irccd_.get_log().debug(*ev.server) << "  channel: " << ev.channel << std::endl;
    irccd_.get_log().debug(*ev.server) << "  names: " << string_util::join(ev.names.begin(), ev.names.end(), ", ") << std::endl;

    auto names = nlohmann::json::array();

    for (const auto& v : ev.names)
        names.push_back(v);

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onNames"           },
        { "server",     ev.server->get_id() },
        { "channel",    ev.channel          },
        { "names",      std::move(names)    }
    }));

    dispatch(ev.server->get_id(), /* origin */ "", ev.channel,
        [=] (plugin&) -> std::string {
            return "onNames";
        },
        [=] (plugin& plugin) {
            plugin.handle_names(irccd_, ev);
        }
    );
}

void dispatcher::operator()(const nick_event& ev)
{
    irccd_.get_log().debug(*ev.server) << "event onNick:" << std::endl;
    irccd_.get_log().debug(*ev.server) << "  origin: " << ev.origin << std::endl;
    irccd_.get_log().debug(*ev.server) << "  nickname: " << ev.nickname << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onNick"            },
        { "server",     ev.server->get_id() },
        { "origin",     ev.origin           },
        { "nickname",   ev.nickname         }
    }));

    dispatch(ev.server->get_id(), ev.origin, /* channel */ "",
        [=] (plugin&) -> std::string {
            return "onNick";
        },
        [=] (plugin& plugin) {
            plugin.handle_nick(irccd_, ev);
        }
    );
}

void dispatcher::operator()(const notice_event& ev)
{
    irccd_.get_log().debug(*ev.server) << "event onNotice:" << std::endl;
    irccd_.get_log().debug(*ev.server) << "  origin: " << ev.origin << std::endl;
    irccd_.get_log().debug(*ev.server) << "  channel: " << ev.channel << std::endl;
    irccd_.get_log().debug(*ev.server) << "  message: " << ev.message << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onNotice"          },
        { "server",     ev.server->get_id() },
        { "origin",     ev.origin           },
        { "channel",    ev.channel          },
        { "message",    ev.message          }
    }));

    dispatch(ev.server->get_id(), ev.origin, /* channel */ "",
        [=] (plugin&) -> std::string {
            return "onNotice";
        },
        [=] (plugin& plugin) {
            plugin.handle_notice(irccd_, ev);
        }
    );
}

void dispatcher::operator()(const part_event& ev)
{
    irccd_.get_log().debug(*ev.server) << "event onPart:" << std::endl;
    irccd_.get_log().debug(*ev.server) << "  origin: " << ev.origin << std::endl;
    irccd_.get_log().debug(*ev.server) << "  channel: " << ev.channel << std::endl;
    irccd_.get_log().debug(*ev.server) << "  reason: " << ev.reason << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onPart"            },
        { "server",     ev.server->get_id() },
        { "origin",     ev.origin           },
        { "channel",    ev.channel          },
        { "reason",     ev.reason           }
    }));

    dispatch(ev.server->get_id(), ev.origin, ev.channel,
        [=] (plugin&) -> std::string {
            return "onPart";
        },
        [=] (plugin& plugin) {
            plugin.handle_part(irccd_, ev);
        }
    );
}

void dispatcher::operator()(const topic_event& ev)
{
    irccd_.get_log().debug(*ev.server) << "event onTopic:" << std::endl;
    irccd_.get_log().debug(*ev.server) << "  origin: " << ev.origin << std::endl;
    irccd_.get_log().debug(*ev.server) << "  channel: " << ev.channel << std::endl;
    irccd_.get_log().debug(*ev.server) << "  topic: " << ev.topic << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onTopic"           },
        { "server",     ev.server->get_id() },
        { "origin",     ev.origin           },
        { "channel",    ev.channel          },
        { "topic",      ev.topic            }
    }));

    dispatch(ev.server->get_id(), ev.origin, ev.channel,
        [=] (plugin&) -> std::string {
            return "onTopic";
        },
        [=] (plugin& plugin) {
            plugin.handle_topic(irccd_, ev);
        }
    );
}

void dispatcher::operator()(const whois_event& ev)
{
    irccd_.get_log().debug(*ev.server) << "event onWhois" << std::endl;
    irccd_.get_log().debug(*ev.server) << "  nickname: " << ev.whois.nick << std::endl;
    irccd_.get_log().debug(*ev.server) << "  username: " << ev.whois.user << std::endl;
    irccd_.get_log().debug(*ev.server) << "  host: " << ev.whois.host << std::endl;
    irccd_.get_log().debug(*ev.server) << "  realname: " << ev.whois.realname << std::endl;
    irccd_.get_log().debug(*ev.server) << "  channels: " << string_util::join(ev.whois.channels, ", ") << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onWhois"           },
        { "server",     ev.server->get_id() },
        { "nickname",   ev.whois.nick       },
        { "username",   ev.whois.user       },
        { "host",       ev.whois.host       },
        { "realname",   ev.whois.realname   }
    }));

    dispatch(ev.server->get_id(), /* origin */ "", /* channel */ "",
        [=] (plugin&) -> std::string {
            return "onWhois";
        },
        [=] (plugin& plugin) {
            plugin.handle_whois(irccd_, ev);
        }
    );
}

} // !namespace

void server_service::handle_error(const std::shared_ptr<server>& server,
                                  const std::error_code& code)
{
    assert(server);

    irccd_.get_log().warning(*server) << code.message() << std::endl;

    irccd_.get_log().warning(*server) << int(server->get_options()) << std::endl;

    if ((server->get_options() & server::options::auto_reconnect) != server::options::auto_reconnect)
        remove(server->get_id());
    else {
        irccd_.get_log().info(*server) << "reconnecting in "
            << server->get_reconnect_delay() << " second(s)" << std::endl;
        wait(server);
    }
}

void server_service::handle_wait(const std::shared_ptr<server>& server, const std::error_code& code)
{
    /*
     * The timer runs on his own control, it will complete either if the delay
     * was reached, there was an error or if the io_context was called to cancel
     * all pending operations.
     *
     * This means while the timer is running someone may already have ask a
     * server for explicit reconnection (e.g. remote command, plugin). Thus we
     * check for server state and if it is still present in service.
     */
    if (code && code != std::errc::operation_canceled) {
        irccd_.get_log().warning(*server) << code.message() << std::endl;
        return;
    }

    if (server->get_state() == server::state::connected || !has(server->get_id()))
        return;

    connect(server);
}

void server_service::handle_recv(const std::shared_ptr<server>& server,
                                 const std::error_code& code,
                                 const event& event)
{
    assert(server);

    if (code)
        handle_error(server, code);
    else {
        recv(server);
        std::visit(dispatcher(irccd_), event);
    }
}

void server_service::handle_connect(const std::shared_ptr<server>& server, const std::error_code& code)
{
    if (code)
        handle_error(server, code);
    else
        recv(server);
}

void server_service::wait(const std::shared_ptr<server>& server)
{
    assert(server);

    auto timer = std::make_shared<boost::asio::deadline_timer>(irccd_.get_service());

    timer->expires_from_now(boost::posix_time::seconds(server->get_reconnect_delay()));
    timer->async_wait([this, server, timer] (auto code) {
        handle_wait(server, code);
    });
}

void server_service::recv(const std::shared_ptr<server>& server)
{
    assert(server);

    server->recv([this, server] (auto code, auto event) {
        handle_recv(server, code, event);
    });
}

void server_service::connect(const std::shared_ptr<server>& server)
{
    assert(server);

    server->connect([this, server] (auto code) {
        handle_connect(server, code);
    });
}

server_service::server_service(irccd &irccd)
    : irccd_(irccd)
{
}

auto server_service::all() const noexcept -> const std::vector<std::shared_ptr<server>>&
{
    return servers_;
}

auto server_service::has(const std::string& name) const noexcept -> bool
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
    const auto s = require(id);

    s->disconnect();
    dispatcher{irccd_}(disconnect_event{s});
}

void server_service::reconnect(std::string_view id)
{
    disconnect(id);
    connect(require(id));
}

void server_service::reconnect()
{
    for (const auto& s : servers_) {
        try {
            s->disconnect();
            dispatcher{irccd_}(disconnect_event{s});
            connect(s);
        } catch (const server_error& ex) {
            irccd_.get_log().warning(*s) << ex.what() << std::endl;
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

            if (has(server->get_id()))
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
    return sv.get_id();
}

} // !logger

} // !irccd
