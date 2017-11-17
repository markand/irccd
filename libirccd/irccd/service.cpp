/*
 * service.cpp -- irccd services
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

#include <algorithm>
#include <array>
#include <functional>
#include <stdexcept>

#include "config.hpp"
#include "irccd.hpp"
#include "logger.hpp"
#include "service.hpp"
#include "string_util.hpp"
#include "system.hpp"
#include "transport_service.hpp"

using namespace std::string_literals;

namespace irccd {

/*
 * command_service.
 * ------------------------------------------------------------------
 */

bool command_service::contains(const std::string& name) const noexcept
{
    return find(name) != nullptr;
}

std::shared_ptr<command> command_service::find(const std::string& name) const noexcept
{
    auto it = std::find_if(commands_.begin(), commands_.end(), [&] (const auto& cmd) {
        return cmd->name() == name;
    });

    return it == commands_.end() ? nullptr : *it;
}

void command_service::add(std::shared_ptr<command> command)
{
    auto it = std::find_if(commands_.begin(), commands_.end(), [&] (const auto& cmd) {
        return cmd->name() == command->name();
    });

    if (it != commands_.end())
        *it = std::move(command);
    else
        commands_.push_back(std::move(command));
}

/*
 * interrupt_service.
 * ------------------------------------------------------------------
 */

interrupt_service::interrupt_service()
    : in_(AF_INET, 0)
    , out_(AF_INET, 0)
{
    // Bind a socket to any port.
    in_.set(net::option::SockReuseAddress(true));
    in_.bind(net::ipv4::any(0));
    in_.listen(1);

    // Do the socket pair.
    out_.connect(net::ipv4::pton("127.0.0.1", net::ipv4::port(in_.getsockname())));
    in_ = in_.accept();
    out_.set(net::option::SockBlockMode(false));
}

void interrupt_service::prepare(fd_set& in, fd_set&, net::Handle& max)
{
    FD_SET(in_.handle(), &in);

    if (in_.handle() > max)
        max = in_.handle();
}

void interrupt_service::sync(fd_set& in, fd_set&)
{
    if (FD_ISSET(in_.handle(), &in)) {
        static std::array<char, 32> tmp;

        try {
            log::debug("irccd: interrupt service recv");
            in_.recv(tmp.data(), 32);
        } catch (const std::exception& ex) {
            log::warning() << "irccd: interrupt service error: " << ex.what() << std::endl;
        }
    }
}

void interrupt_service::interrupt() noexcept
{
    try {
        static char byte;

        log::debug("irccd: interrupt service send");
        out_.send(&byte, 1);
    } catch (const std::exception& ex) {
        log::warning() << "irccd: interrupt service error: " << ex.what() << std::endl;
    }
}

/*
 * plugin_service.
 * ------------------------------------------------------------------
 */

plugin_service::plugin_service(irccd& irccd) noexcept
    : irccd_(irccd)
{
}

plugin_service::~plugin_service()
{
    for (const auto& plugin : plugins_)
        plugin->on_unload(irccd_);
}

bool plugin_service::has(const std::string& name) const noexcept
{
    return std::count_if(plugins_.cbegin(), plugins_.cend(), [&] (const auto& plugin) {
        return plugin->name() == name;
    }) > 0;
}

std::shared_ptr<plugin> plugin_service::get(const std::string& name) const noexcept
{
    auto it = std::find_if(plugins_.begin(), plugins_.end(), [&] (const auto& plugin) {
        return plugin->name() == name;
    });

    if (it == plugins_.end())
        return nullptr;

    return *it;
}

std::shared_ptr<plugin> plugin_service::require(const std::string& name) const
{
    auto plugin = get(name);

    if (!plugin)
        throw std::invalid_argument(string_util::sprintf("plugin %s not found", name));

    return plugin;
}

void plugin_service::add(std::shared_ptr<plugin> plugin)
{
    plugins_.push_back(std::move(plugin));
}

void plugin_service::add_loader(std::unique_ptr<plugin_loader> loader)
{
    loaders_.push_back(std::move(loader));
}

namespace {

template <typename Map>
Map to_map(const config& conf, const std::string& section)
{
    Map ret;

    for (const auto& opt : conf.doc().get(section))
        ret.emplace(opt.key(), opt.value());

    return ret;
}

} // !namespace

plugin_config plugin_service::config(const std::string& id)
{
    return to_map<plugin_config>(irccd_.config(), string_util::sprintf("plugin.%s", id));
}

plugin_formats plugin_service::formats(const std::string& id)
{
    return to_map<plugin_formats>(irccd_.config(), string_util::sprintf("format.%s", id));
}

plugin_paths plugin_service::paths(const std::string& id)
{
    auto defaults = to_map<plugin_paths>(irccd_.config(), "paths");
    auto paths = to_map<plugin_paths>(irccd_.config(), string_util::sprintf("paths.%s", id));

    // Fill defaults paths.
    if (!defaults.count("cache"))
        defaults.emplace("cache", sys::cachedir() + "/plugin/" + id);
    if (!defaults.count("data"))
        paths.emplace("data", sys::datadir() + "/plugin/" + id);
    if (!defaults.count("config"))
        paths.emplace("config", sys::sysconfigdir() + "/plugin/" + id);

    // Now fill missing fields.
    if (!paths.count("cache"))
        paths.emplace("cache", defaults["cache"]);
    if (!paths.count("data"))
        paths.emplace("data", defaults["data"]);
    if (!paths.count("config"))
        paths.emplace("config", defaults["config"]);

    return paths;
}

std::shared_ptr<plugin> plugin_service::open(const std::string& id,
                                             const std::string& path)
{
    for (const auto& loader : loaders_) {
        auto plugin = loader->open(id, path);

        if (plugin)
            return plugin;
    }

    return nullptr;
}

std::shared_ptr<plugin> plugin_service::find(const std::string& id)
{
    for (const auto& loader : loaders_) {
        auto plugin = loader->find(id);

        if (plugin)
            return plugin;
    }

    return nullptr;
}

void plugin_service::load(std::string name, std::string path)
{
    if (has(name))
        return;

    try {
        std::shared_ptr<plugin> plugin;

        if (path.empty())
            plugin = find(name);
        else
            plugin = open(name, std::move(path));

        if (plugin) {
            plugin->set_config(config(name));
            plugin->set_formats(formats(name));
            plugin->set_paths(paths(name));
            plugin->on_load(irccd_);

            add(std::move(plugin));
        }
    } catch (const std::exception& ex) {
        log::warning(string_util::sprintf("plugin %s: %s", name, ex.what()));
    }
}

void plugin_service::reload(const std::string& name)
{
    auto plugin = get(name);

    if (plugin)
        plugin->on_reload(irccd_);
}

void plugin_service::unload(const std::string& name)
{
    auto it = std::find_if(plugins_.begin(), plugins_.end(), [&] (const auto& plugin) {
        return plugin->name() == name;
    });

    if (it != plugins_.end()) {
        (*it)->on_unload(irccd_);
        plugins_.erase(it);
    }
}

/*
 * rule_service.
 * ------------------------------------------------------------------
 */

void rule_service::add(rule rule)
{
    rules_.push_back(std::move(rule));
}

void rule_service::insert(rule rule, unsigned position)
{
    assert(position <= rules_.size());

    rules_.insert(rules_.begin() + position, std::move(rule));
}

void rule_service::remove(unsigned position)
{
    assert(position < rules_.size());

    rules_.erase(rules_.begin() + position);
}

const rule &rule_service::require(unsigned position) const
{
    if (position >= rules_.size())
        throw std::out_of_range("rule " + std::to_string(position) + " does not exist");

    return rules_[position];
}

rule &rule_service::require(unsigned position)
{
    if (position >= rules_.size())
        throw std::out_of_range("rule " + std::to_string(position) + " does not exist");

    return rules_[position];
}

bool rule_service::solve(const std::string& server,
                         const std::string& channel,
                         const std::string& origin,
                         const std::string& plugin,
                         const std::string& event) noexcept
{
    bool result = true;

    log::debug(string_util::sprintf("rule: solving for server=%s, channel=%s, origin=%s, plugin=%s, event=%s",
        server, channel, origin, plugin, event));

    int i = 0;
    for (const auto& rule : rules_) {
        log::debug() << "  candidate "   << i++ << ":\n"
                     << "    servers: "  << string_util::join(rule.servers().begin(), rule.servers().end()) << "\n"
                     << "    channels: " << string_util::join(rule.channels().begin(), rule.channels().end()) << "\n"
                     << "    origins: "  << string_util::join(rule.origins().begin(), rule.origins().end()) << "\n"
                     << "    plugins: "  << string_util::join(rule.plugins().begin(), rule.plugins().end()) << "\n"
                     << "    events: "   << string_util::join(rule.events().begin(), rule.events().end()) << "\n"
                     << "    action: "   << ((rule.action() == rule::action_type::accept) ? "accept" : "drop") << std::endl;

        if (rule.match(server, channel, origin, plugin, event))
            result = rule.action() == rule::action_type::accept;
    }

    return result;
}

/*
 * server_service.
 * ------------------------------------------------------------------
 */

class event_handler {
public:
    std::string server;
    std::string origin;
    std::string target;
    std::function<std::string (plugin &)> function_name;
    std::function<void (plugin &)> function_exec;

    void operator()(irccd& irccd) const
    {
        for (auto& plugin : irccd.plugins().list()) {
            auto eventname = function_name(*plugin);
            auto allowed = irccd.rules().solve(server, target, origin, plugin->name(), eventname);

            if (!allowed) {
                log::debug() << "rule: event skipped on match" << std::endl;
                continue;
            }

            log::debug() << "rule: event allowed" << std::endl;

            // TODO: this is the responsability of plugin_service.
            try {
                function_exec(*plugin);
            } catch (const std::exception& ex) {
                log::warning() << "plugin " << plugin->name() << ": error: " << ex.what() << std::endl;
            }
        }
    }
};

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

    irccd_.post(event_handler{ev.server->name(), ev.origin, ev.channel,
        [=] (plugin&) -> std::string {
            return "onChannelMode";
        },
        [=] (plugin& plugin) {
            plugin.on_channel_mode(irccd_, ev);
        }
    });
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

    irccd_.post(event_handler{ev.server->name(), ev.origin, ev.channel,
        [=] (plugin&) -> std::string {
            return "onChannelNotice";
        },
        [=] (plugin& plugin) {
            plugin.on_channel_notice(irccd_, ev);
        }
    });
}

void server_service::handle_connect(const connect_event& ev)
{
    log::debug() << "server " << ev.server->name() << ": event onConnect" << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onConnect"         },
        { "server",     ev.server->name()   }
    }));

    irccd_.post(event_handler{ev.server->name(), /* origin */ "", /* channel */ "",
        [=] (plugin&) -> std::string {
            return "onConnect";
        },
        [=] (plugin& plugin) {
            plugin.on_connect(irccd_, ev);
        }
    });
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

    irccd_.post(event_handler{ev.server->name(), ev.origin, ev.channel,
        [=] (plugin&) -> std::string {
            return "onInvite";
        },
        [=] (plugin& plugin) {
            plugin.on_invite(irccd_, ev);
        }
    });
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

    irccd_.post(event_handler{ev.server->name(), ev.origin, ev.channel,
        [=] (plugin&) -> std::string {
            return "onJoin";
        },
        [=] (plugin& plugin) {
            plugin.on_join(irccd_, ev);
        }
    });
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

    irccd_.post(event_handler{ev.server->name(), ev.origin, ev.channel,
        [=] (plugin&) -> std::string {
            return "onKick";
        },
        [=] (plugin& plugin) {
            plugin.on_kick(irccd_, ev);
        }
    });
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

    irccd_.post(event_handler{ev.server->name(), ev.origin, ev.channel,
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
    });
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

    irccd_.post(event_handler{ev.server->name(), ev.origin, ev.channel,
        [=] (plugin&) -> std::string {
            return "onMe";
        },
        [=] (plugin& plugin) {
            plugin.on_me(irccd_, ev);
        }
    });
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

    irccd_.post(event_handler{ev.server->name(), ev.origin, /* channel */ "",
        [=] (plugin &) -> std::string {
            return "onMode";
        },
        [=] (plugin &plugin) {
            plugin.on_mode(irccd_, ev);
        }
    });
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

    irccd_.post(event_handler{ev.server->name(), /* origin */ "", ev.channel,
        [=] (plugin&) -> std::string {
            return "onNames";
        },
        [=] (plugin& plugin) {
            plugin.on_names(irccd_, ev);
        }
    });
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

    irccd_.post(event_handler{ev.server->name(), ev.origin, /* channel */ "",
        [=] (plugin&) -> std::string {
            return "onNick";
        },
        [=] (plugin& plugin) {
            plugin.on_nick(irccd_, ev);
        }
    });
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

    irccd_.post(event_handler{ev.server->name(), ev.origin, /* channel */ "",
        [=] (plugin&) -> std::string {
            return "onNotice";
        },
        [=] (plugin& plugin) {
            plugin.on_notice(irccd_, ev);
        }
    });
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

    irccd_.post(event_handler{ev.server->name(), ev.origin, ev.channel,
        [=] (plugin&) -> std::string {
            return "onPart";
        },
        [=] (plugin& plugin) {
            plugin.on_part(irccd_, ev);
        }
    });
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

    irccd_.post(event_handler{ev.server->name(), ev.origin, /* channel */ "",
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
    });
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

    irccd_.post(event_handler{ev.server->name(), ev.origin, ev.channel,
        [=] (plugin&) -> std::string {
            return "onTopic";
        },
        [=] (plugin& plugin) {
            plugin.on_topic(irccd_, ev);
        }
    });
}

void server_service::handle_whois(const whois_event& ev)
{
    log::debug() << "server " << ev.server->name() << ": event onWhois\n";
    log::debug() << "  nickname: " << ev.whois.nick << "\n";
    log::debug() << "  username: " << ev.whois.user << "\n";
    log::debug() << "  host: " << ev.whois.host << "\n";
    log::debug() << "  realname: " << ev.whois.realname << "\n";
    log::debug() << "  channels: " << string_util::join(ev.whois.channels.begin(), ev.whois.channels.end()) << std::endl;

    irccd_.transports().broadcast(nlohmann::json::object({
        { "event",      "onWhois"           },
        { "server",     ev.server->name()   },
        { "nickname",   ev.whois.nick       },
        { "username",   ev.whois.user       },
        { "host",       ev.whois.host       },
        { "realname",   ev.whois.realname   }
    }));

    irccd_.post(event_handler{ev.server->name(), /* origin */ "", /* channel */ "",
        [=] (plugin&) -> std::string {
            return "onWhois";
        },
        [=] (plugin& plugin) {
            plugin.on_whois(irccd_, ev);
        }
    });
}

server_service::server_service(irccd &irccd)
    : irccd_(irccd)
{
}

void server_service::prepare(fd_set& in, fd_set& out, net::Handle& max)
{
    for (auto& server : servers_) {
        server->update();
        server->prepare(in, out, max);
    }
}

void server_service::sync(fd_set& in, fd_set& out)
{
    for (auto& server : servers_)
        server->sync(in, out);
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
        irccd_.post([=] (irccd&) {
            auto server = ptr.lock();

            if (server) {
                log::info(string_util::sprintf("server %s: removed", server->name()));
                servers_.erase(std::find(servers_.begin(), servers_.end(), server));
            }
        });
    });

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
