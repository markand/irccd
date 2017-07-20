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

#include <format.h>

#include "irccd.hpp"
#include "logger.hpp"
#include "service.hpp"
#include "transport.hpp"

using namespace fmt::literals;

namespace irccd {

/*
 * CommandService.
 * ------------------------------------------------------------------
 */

bool CommandService::contains(const std::string &name) const noexcept
{
    return find(name) != nullptr;
}

std::shared_ptr<Command> CommandService::find(const std::string &name) const noexcept
{
    auto it = std::find_if(m_commands.begin(), m_commands.end(), [&] (const auto &cmd) {
        return cmd->name() == name;
    });

    return it == m_commands.end() ? nullptr : *it;
}

void CommandService::add(std::shared_ptr<Command> command)
{
    auto it = std::find_if(m_commands.begin(), m_commands.end(), [&] (const auto &cmd) {
        return cmd->name() == command->name();
    });

    if (it != m_commands.end())
        *it = std::move(command);
    else
        m_commands.push_back(std::move(command));
}

/*
 * InterruptService.
 * ------------------------------------------------------------------
 */

InterruptService::InterruptService()
    : m_in(AF_INET, 0)
    , m_out(AF_INET, 0)
{
    // Bind a socket to any port.
    m_in.set(net::option::SockReuseAddress(true));
    m_in.bind(net::ipv4::any(0));
    m_in.listen(1);

    // Do the socket pair.
    m_out.connect(net::ipv4::pton("127.0.0.1", net::ipv4::port(m_in.getsockname())));
    m_in = m_in.accept();
    m_out.set(net::option::SockBlockMode(false));
}

void InterruptService::prepare(fd_set &in, fd_set &, net::Handle &max)
{
    FD_SET(m_in.handle(), &in);

    if (m_in.handle() > max)
        max = m_in.handle();
}

void InterruptService::sync(fd_set &in, fd_set &)
{
    if (FD_ISSET(m_in.handle(), &in)) {
        static std::array<char, 32> tmp;

        try {
            log::debug("irccd: interrupt service recv");
            m_in.recv(tmp.data(), 32);
        } catch (const std::exception &ex) {
            log::warning() << "irccd: interrupt service error: " << ex.what() << std::endl;
        }
    }
}

void InterruptService::interrupt() noexcept
{
    try {
        static char byte;

        log::debug("irccd: interrupt service send");
        m_out.send(&byte, 1);
    } catch (const std::exception &ex) {
        log::warning() << "irccd: interrupt service error: " << ex.what() << std::endl;
    }
}

/*
 * PluginService.
 * ------------------------------------------------------------------
 */

PluginService::PluginService(Irccd &irccd) noexcept
    : m_irccd(irccd)
{
}

PluginService::~PluginService()
{
    for (const auto &plugin : m_plugins)
        plugin->onUnload(m_irccd);
}

bool PluginService::has(const std::string &name) const noexcept
{
    return std::count_if(m_plugins.cbegin(), m_plugins.cend(), [&] (const auto &plugin) {
        return plugin->name() == name;
    }) > 0;
}

std::shared_ptr<Plugin> PluginService::get(const std::string &name) const noexcept
{
    auto it = std::find_if(m_plugins.begin(), m_plugins.end(), [&] (const auto &plugin) {
        return plugin->name() == name;
    });

    if (it == m_plugins.end())
        return nullptr;

    return *it;
}

std::shared_ptr<Plugin> PluginService::require(const std::string &name) const
{
    auto plugin = get(name);

    if (!plugin)
        throw std::invalid_argument("plugin {} not found"_format(name));

    return plugin;
}

void PluginService::add(std::shared_ptr<Plugin> plugin)
{
    m_plugins.push_back(std::move(plugin));
}

void PluginService::addLoader(std::unique_ptr<PluginLoader> loader)
{
    m_loaders.push_back(std::move(loader));
}

void PluginService::setConfig(const std::string &name, PluginConfig config)
{
    m_config.emplace(name, std::move(config));
}

PluginConfig PluginService::config(const std::string &name) const
{
    auto it = m_config.find(name);

    if (it != m_config.end())
        return it->second;

    return PluginConfig();
}

void PluginService::setFormats(const std::string &name, PluginFormats formats)
{
    m_formats.emplace(name, std::move(formats));
}

PluginFormats PluginService::formats(const std::string &name) const
{
    auto it = m_formats.find(name);

    if (it != m_formats.end())
        return it->second;

    return PluginFormats();
}

std::shared_ptr<Plugin> PluginService::open(const std::string &id,
                                            const std::string &path)
{
    for (const auto &loader : m_loaders) {
        auto plugin = loader->open(id, path);

        if (plugin)
            return plugin;
    }

    return nullptr;
}

std::shared_ptr<Plugin> PluginService::find(const std::string &id)
{
    for (const auto &loader : m_loaders) {
        auto plugin = loader->find(id);

        if (plugin)
            return plugin;
    }

    return nullptr;
}

void PluginService::load(std::string name, std::string path)
{
    if (has(name))
        return;

    try {
        std::shared_ptr<Plugin> plugin;

        if (path.empty())
            plugin = find(name);
        else
            plugin = open(name, std::move(path));

        if (plugin) {
            plugin->setConfig(m_config[name]);
            plugin->setFormats(m_formats[name]);
            plugin->onLoad(m_irccd);

            add(std::move(plugin));
        }
    } catch (const std::exception &ex) {
        log::warning("plugin {}: {}"_format(name, ex.what()));
    }
}

void PluginService::reload(const std::string &name)
{
    auto plugin = get(name);

    if (plugin)
        plugin->onReload(m_irccd);
}

void PluginService::unload(const std::string &name)
{
    auto it = std::find_if(m_plugins.begin(), m_plugins.end(), [&] (const auto &plugin) {
        return plugin->name() == name;
    });

    if (it != m_plugins.end()) {
        (*it)->onUnload(m_irccd);
        m_plugins.erase(it);
    }
}

/*
 * RuleService.
 * ------------------------------------------------------------------
 */

void RuleService::add(Rule rule)
{
    m_rules.push_back(std::move(rule));
}

void RuleService::insert(Rule rule, unsigned position)
{
    assert(position <= m_rules.size());

    m_rules.insert(m_rules.begin() + position, std::move(rule));
}

void RuleService::remove(unsigned position)
{
    assert(position < m_rules.size());

    m_rules.erase(m_rules.begin() + position);
}

const Rule &RuleService::require(unsigned position) const
{
    if (position >= m_rules.size())
        throw std::out_of_range("rule " + std::to_string(position) + " does not exist");

    return m_rules[position];
}

Rule &RuleService::require(unsigned position)
{
    if (position >= m_rules.size())
        throw std::out_of_range("rule " + std::to_string(position) + " does not exist");

    return m_rules[position];
}

bool RuleService::solve(const std::string &server,
                        const std::string &channel,
                        const std::string &origin,
                        const std::string &plugin,
                        const std::string &event) noexcept
{
    bool result = true;

    log::debug("rule: solving for server={}, channel={}, origin={}, plugin={}, event={}"_format(server, channel,
           origin, plugin, event));

    int i = 0;
    for (const Rule &rule : m_rules) {
        log::debug() << "  candidate " << i++ << ":\n"
                 << "    servers: " << util::join(rule.servers().begin(), rule.servers().end()) << "\n"
                 << "    channels: " << util::join(rule.channels().begin(), rule.channels().end()) << "\n"
                 << "    origins: " << util::join(rule.origins().begin(), rule.origins().end()) << "\n"
                 << "    plugins: " << util::join(rule.plugins().begin(), rule.plugins().end()) << "\n"
                 << "    events: " << util::join(rule.events().begin(), rule.events().end()) << "\n"
                 << "    action: " << ((rule.action() == RuleAction::Accept) ? "accept" : "drop") << std::endl;

        if (rule.match(server, channel, origin, plugin, event))
            result = rule.action() == RuleAction::Accept;
    }

    return result;
}

/*
 * ServerService.
 * ------------------------------------------------------------------
 */

class EventHandler {
public:
    std::string server;
    std::string origin;
    std::string target;
    std::function<std::string (Plugin &)> functionName;
    std::function<void (Plugin &)> functionExec;

    void operator()(Irccd &irccd) const
    {
        for (auto &plugin : irccd.plugins().list()) {
            auto eventname = functionName(*plugin);
            auto allowed = irccd.rules().solve(server, target, origin, plugin->name(), eventname);

            if (!allowed) {
                log::debug() << "rule: event skipped on match" << std::endl;
                continue;
            } else
                log::debug() << "rule: event allowed" << std::endl;

            // TODO: server-event must not know which type of plugin.
            // TODO: get generic error.
            // TODO: this is the responsability of service-plugin.
            try {
                functionExec(*plugin);
            } catch (const std::exception &ex) {
                log::warning() << "plugin " << plugin->name() << ": error: " << ex.what() << std::endl;
            }
        }
    }
};

void ServerService::handleChannelMode(const ChannelModeEvent &ev)
{
    log::debug() << "server " << ev.server->name() << ": event onChannelMode:\n";
    log::debug() << "  origin: " << ev.origin << "\n";
    log::debug() << "  channel: " << ev.channel << "\n";
    log::debug() << "  mode: " << ev.mode << "\n";
    log::debug() << "  argument: " << ev.argument << std::endl;

    m_irccd.transports().broadcast(nlohmann::json::object({
        { "event",      "onChannelMode"     },
        { "server",     ev.server->name()   },
        { "origin",     ev.origin           },
        { "channel",    ev.channel          },
        { "mode",       ev.mode             },
        { "argument",   ev.argument         }
    }));

    m_irccd.post(EventHandler{ev.server->name(), ev.origin, ev.channel,
        [=] (Plugin &) -> std::string {
            return "onChannelMode";
        },
        [=] (Plugin &plugin) {
            plugin.onChannelMode(m_irccd, ev);
        }
    });
}

void ServerService::handleChannelNotice(const ChannelNoticeEvent &ev)
{
    log::debug() << "server " << ev.server->name() << ": event onChannelNotice:\n";
    log::debug() << "  origin: " << ev.origin << "\n";
    log::debug() << "  channel: " << ev.channel << "\n";
    log::debug() << "  message: " << ev.message << std::endl;

    m_irccd.transports().broadcast(nlohmann::json::object({
        { "event",      "onChannelNotice"   },
        { "server",     ev.server->name()   },
        { "origin",     ev.origin           },
        { "channel",    ev.channel          },
        { "message",    ev.message          }
    }));

    m_irccd.post(EventHandler{ev.server->name(), ev.origin, ev.channel,
        [=] (Plugin &) -> std::string {
            return "onChannelNotice";
        },
        [=] (Plugin &plugin) {
            plugin.onChannelNotice(m_irccd, ev);
        }
    });
}

void ServerService::handleConnect(const ConnectEvent &ev)
{
    log::debug() << "server " << ev.server->name() << ": event onConnect" << std::endl;

    m_irccd.transports().broadcast(nlohmann::json::object({
        { "event",      "onConnect"         },
        { "server",     ev.server->name()   }
    }));

    m_irccd.post(EventHandler{ev.server->name(), /* origin */ "", /* channel */ "",
        [=] (Plugin &) -> std::string {
            return "onConnect";
        },
        [=] (Plugin &plugin) {
            plugin.onConnect(m_irccd, ev);
        }
    });
}

void ServerService::handleInvite(const InviteEvent &ev)
{
    log::debug() << "server " << ev.server->name() << ": event onInvite:\n";
    log::debug() << "  origin: " << ev.origin << "\n";
    log::debug() << "  channel: " << ev.channel << "\n";
    log::debug() << "  target: " << ev.nickname << std::endl;

    m_irccd.transports().broadcast(nlohmann::json::object({
        { "event",      "onInvite"          },
        { "server",     ev.server->name()   },
        { "origin",     ev.origin           },
        { "channel",    ev.channel          }
    }));

    m_irccd.post(EventHandler{ev.server->name(), ev.origin, ev.channel,
        [=] (Plugin &) -> std::string {
            return "onInvite";
        },
        [=] (Plugin &plugin) {
            plugin.onInvite(m_irccd, ev);
        }
    });
}

void ServerService::handleJoin(const JoinEvent &ev)
{
    log::debug() << "server " << ev.server->name() << ": event onJoin:\n";
    log::debug() << "  origin: " << ev.origin << "\n";
    log::debug() << "  channel: " << ev.channel << std::endl;

    m_irccd.transports().broadcast(nlohmann::json::object({
        { "event",      "onJoin"            },
        { "server",     ev.server->name()   },
        { "origin",     ev.origin           },
        { "channel",    ev.channel          }
    }));

    m_irccd.post(EventHandler{ev.server->name(), ev.origin, ev.channel,
        [=] (Plugin &) -> std::string {
            return "onJoin";
        },
        [=] (Plugin &plugin) {
            plugin.onJoin(m_irccd, ev);
        }
    });
}

void ServerService::handleKick(const KickEvent &ev)
{
    log::debug() << "server " << ev.server->name() << ": event onKick:\n";
    log::debug() << "  origin: " << ev.origin << "\n";
    log::debug() << "  channel: " << ev.channel << "\n";
    log::debug() << "  target: " << ev.target << "\n";
    log::debug() << "  reason: " << ev.reason << std::endl;

    m_irccd.transports().broadcast(nlohmann::json::object({
        { "event",      "onKick"            },
        { "server",     ev.server->name()   },
        { "origin",     ev.origin           },
        { "channel",    ev.channel          },
        { "target",     ev.target           },
        { "reason",     ev.reason           }
    }));

    m_irccd.post(EventHandler{ev.server->name(), ev.origin, ev.channel,
        [=] (Plugin &) -> std::string {
            return "onKick";
        },
        [=] (Plugin &plugin) {
            plugin.onKick(m_irccd, ev);
        }
    });
}

void ServerService::handleMessage(const MessageEvent &ev)
{
    log::debug() << "server " << ev.server->name() << ": event onMessage:\n";
    log::debug() << "  origin: " << ev.origin << "\n";
    log::debug() << "  channel: " << ev.channel << "\n";
    log::debug() << "  message: " << ev.message << std::endl;

    m_irccd.transports().broadcast(nlohmann::json::object({
        { "event",      "onMessage"         },
        { "server",     ev.server->name()   },
        { "origin",     ev.origin           },
        { "channel",    ev.channel          },
        { "message",    ev.message          }
    }));

    m_irccd.post(EventHandler{ev.server->name(), ev.origin, ev.channel,
        [=] (Plugin &plugin) -> std::string {
            return util::parseMessage(ev.message, ev.server->commandCharacter(), plugin.name()).second == util::MessageType::Command ? "onCommand" : "onMessage";
        },
        [=] (Plugin &plugin) mutable {
            auto copy = ev;
            auto pack = util::parseMessage(copy.message, copy.server->commandCharacter(), plugin.name());

            copy.message = pack.first;

            if (pack.second == util::MessageType::Command)
                plugin.onCommand(m_irccd, copy);
            else
                plugin.onMessage(m_irccd, copy);
        }
    });
}

void ServerService::handleMe(const MeEvent &ev)
{
    log::debug() << "server " << ev.server->name() << ": event onMe:\n";
    log::debug() << "  origin: " << ev.origin << "\n";
    log::debug() << "  target: " << ev.channel << "\n";
    log::debug() << "  message: " << ev.message << std::endl;

    m_irccd.transports().broadcast(nlohmann::json::object({
        { "event",      "onMe"              },
        { "server",     ev.server->name()   },
        { "origin",     ev.origin           },
        { "target",     ev.channel          },
        { "message",    ev.message          }
    }));

    m_irccd.post(EventHandler{ev.server->name(), ev.origin, ev.channel,
        [=] (Plugin &) -> std::string {
            return "onMe";
        },
        [=] (Plugin &plugin) {
            plugin.onMe(m_irccd, ev);
        }
    });
}

void ServerService::handleMode(const ModeEvent &ev)
{
    log::debug() << "server " << ev.server->name() << ": event onMode\n";
    log::debug() << "  origin: " << ev.origin << "\n";
    log::debug() << "  mode: " << ev.mode << std::endl;

    m_irccd.transports().broadcast(nlohmann::json::object({
        { "event",      "onMode"            },
        { "server",     ev.server->name()   },
        { "origin",     ev.origin           },
        { "mode",       ev.mode             }
    }));

    m_irccd.post(EventHandler{ev.server->name(), ev.origin, /* channel */ "",
        [=] (Plugin &) -> std::string {
            return "onMode";
        },
        [=] (Plugin &plugin) {
            plugin.onMode(m_irccd, ev);
        }
    });
}

void ServerService::handleNames(const NamesEvent &ev)
{
    log::debug() << "server " << ev.server->name() << ": event onNames:\n";
    log::debug() << "  channel: " << ev.channel << "\n";
    log::debug() << "  names: " << util::join(ev.names.begin(), ev.names.end(), ", ") << std::endl;

    auto names = nlohmann::json::array();

    for (const auto &v : ev.names)
        names.push_back(v);

    m_irccd.transports().broadcast(nlohmann::json::object({
        { "event",      "onNames"           },
        { "server",     ev.server->name()   },
        { "channel",    ev.channel          },
        { "names",      std::move(names)    }
    }));

    m_irccd.post(EventHandler{ev.server->name(), /* origin */ "", ev.channel,
        [=] (Plugin &) -> std::string {
            return "onNames";
        },
        [=] (Plugin &plugin) {
            plugin.onNames(m_irccd, ev);
        }
    });
}

void ServerService::handleNick(const NickEvent &ev)
{
    log::debug() << "server " << ev.server->name() << ": event onNick:\n";
    log::debug() << "  origin: " << ev.origin << "\n";
    log::debug() << "  nickname: " << ev.nickname << std::endl;

    m_irccd.transports().broadcast(nlohmann::json::object({
        { "event",      "onNick"            },
        { "server",     ev.server->name()   },
        { "origin",     ev.origin           },
        { "nickname",   ev.nickname         }
    }));

    m_irccd.post(EventHandler{ev.server->name(), ev.origin, /* channel */ "",
        [=] (Plugin &) -> std::string {
            return "onNick";
        },
        [=] (Plugin &plugin) {
            plugin.onNick(m_irccd, ev);
        }
    });
}

void ServerService::handleNotice(const NoticeEvent &ev)
{
    log::debug() << "server " << ev.server->name() << ": event onNotice:\n";
    log::debug() << "  origin: " << ev.origin << "\n";
    log::debug() << "  message: " << ev.message << std::endl;

    m_irccd.transports().broadcast(nlohmann::json::object({
        { "event",      "onNotice"          },
        { "server",     ev.server->name()   },
        { "origin",     ev.origin           },
        { "message",    ev.message          }
    }));

    m_irccd.post(EventHandler{ev.server->name(), ev.origin, /* channel */ "",
        [=] (Plugin &) -> std::string {
            return "onNotice";
        },
        [=] (Plugin &plugin) {
            plugin.onNotice(m_irccd, ev);
        }
    });
}

void ServerService::handlePart(const PartEvent &ev)
{
    log::debug() << "server " << ev.server->name() << ": event onPart:\n";
    log::debug() << "  origin: " << ev.origin << "\n";
    log::debug() << "  channel: " << ev.channel << "\n";
    log::debug() << "  reason: " << ev.reason << std::endl;

    m_irccd.transports().broadcast(nlohmann::json::object({
        { "event",      "onPart"            },
        { "server",     ev.server->name()   },
        { "origin",     ev.origin           },
        { "channel",    ev.channel          },
        { "reason",     ev.reason           }
    }));

    m_irccd.post(EventHandler{ev.server->name(), ev.origin, ev.channel,
        [=] (Plugin &) -> std::string {
            return "onPart";
        },
        [=] (Plugin &plugin) {
            plugin.onPart(m_irccd, ev);
        }
    });
}

void ServerService::handleQuery(const QueryEvent &ev)
{
    log::debug() << "server " << ev.server->name() << ": event onQuery:\n";
    log::debug() << "  origin: " << ev.origin << "\n";
    log::debug() << "  message: " << ev.message << std::endl;

    m_irccd.transports().broadcast(nlohmann::json::object({
        { "event",      "onQuery"           },
        { "server",     ev.server->name()   },
        { "origin",     ev.origin           },
        { "message",    ev.message          }
    }));

    m_irccd.post(EventHandler{ev.server->name(), ev.origin, /* channel */ "",
        [=] (Plugin &plugin) -> std::string {
            return util::parseMessage(ev.message, ev.server->commandCharacter(), plugin.name()).second == util::MessageType::Command ? "onQueryCommand" : "onQuery";
        },
        [=] (Plugin &plugin) mutable {
            auto copy = ev;
            auto pack = util::parseMessage(copy.message, copy.server->commandCharacter(), plugin.name());

            copy.message = pack.first;

            if (pack.second == util::MessageType::Command)
                plugin.onQueryCommand(m_irccd, copy);
            else
                plugin.onQuery(m_irccd, copy);
        }
    });
}

void ServerService::handleTopic(const TopicEvent &ev)
{
    log::debug() << "server " << ev.server->name() << ": event onTopic:\n";
    log::debug() << "  origin: " << ev.origin << "\n";
    log::debug() << "  channel: " << ev.channel << "\n";
    log::debug() << "  topic: " << ev.topic << std::endl;

    m_irccd.transports().broadcast(nlohmann::json::object({
        { "event",      "onTopic"           },
        { "server",     ev.server->name()   },
        { "origin",     ev.origin           },
        { "channel",    ev.channel          },
        { "topic",      ev.topic            }
    }));

    m_irccd.post(EventHandler{ev.server->name(), ev.origin, ev.channel,
        [=] (Plugin &) -> std::string {
            return "onTopic";
        },
        [=] (Plugin &plugin) {
            plugin.onTopic(m_irccd, ev);
        }
    });
}

void ServerService::handleWhois(const WhoisEvent &ev)
{
    log::debug() << "server " << ev.server->name() << ": event onWhois\n";
    log::debug() << "  nickname: " << ev.whois.nick << "\n";
    log::debug() << "  username: " << ev.whois.user << "\n";
    log::debug() << "  host: " << ev.whois.host << "\n";
    log::debug() << "  realname: " << ev.whois.realname << "\n";
    log::debug() << "  channels: " << util::join(ev.whois.channels.begin(), ev.whois.channels.end()) << std::endl;

    m_irccd.transports().broadcast(nlohmann::json::object({
        { "event",      "onWhois"           },
        { "server",     ev.server->name()   },
        { "nickname",   ev.whois.nick       },
        { "username",   ev.whois.user       },
        { "host",       ev.whois.host       },
        { "realname",   ev.whois.realname   }
    }));

    m_irccd.post(EventHandler{ev.server->name(), /* origin */ "", /* channel */ "",
        [=] (Plugin &) -> std::string {
            return "onWhois";
        },
        [=] (Plugin &plugin) {
            plugin.onWhois(m_irccd, ev);
        }
    });
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
    for (auto &server : m_servers)
        server->sync(in, out);
}

bool ServerService::has(const std::string &name) const noexcept
{
    return std::count_if(m_servers.cbegin(), m_servers.end(), [&] (const auto &server) {
        return server->name() == name;
    }) > 0;
}

void ServerService::add(std::shared_ptr<Server> server)
{
    assert(!has(server->name()));

    using namespace std::placeholders;

    std::weak_ptr<Server> ptr(server);

    server->onChannelMode.connect(std::bind(&ServerService::handleChannelMode, this, _1));
    server->onChannelNotice.connect(std::bind(&ServerService::handleChannelNotice, this, _1));
    server->onConnect.connect(std::bind(&ServerService::handleConnect, this, _1));
    server->onInvite.connect(std::bind(&ServerService::handleInvite, this, _1));
    server->onJoin.connect(std::bind(&ServerService::handleJoin, this, _1));
    server->onKick.connect(std::bind(&ServerService::handleKick, this, _1));
    server->onMessage.connect(std::bind(&ServerService::handleMessage, this, _1));
    server->onMe.connect(std::bind(&ServerService::handleMe, this, _1));
    server->onMode.connect(std::bind(&ServerService::handleMode, this, _1));
    server->onNames.connect(std::bind(&ServerService::handleNames, this, _1));
    server->onNick.connect(std::bind(&ServerService::handleNick, this, _1));
    server->onNotice.connect(std::bind(&ServerService::handleNotice, this, _1));
    server->onPart.connect(std::bind(&ServerService::handlePart, this, _1));
    server->onQuery.connect(std::bind(&ServerService::handleQuery, this, _1));
    server->onTopic.connect(std::bind(&ServerService::handleTopic, this, _1));
    server->onWhois.connect(std::bind(&ServerService::handleWhois, this, _1));
    server->onDie.connect([this, ptr] () {
        m_irccd.post([=] (Irccd &) {
            auto server = ptr.lock();

            if (server) {
                log::info("server {}: removed"_format(server->name()));
                m_servers.erase(std::find(m_servers.begin(), m_servers.end(), server));
            }
        });
    });

    m_servers.push_back(std::move(server));
}

std::shared_ptr<Server> ServerService::get(const std::string &name) const noexcept
{
    auto it = std::find_if(m_servers.begin(), m_servers.end(), [&] (const auto &server) {
        return server->name() == name;
    });

    if (it == m_servers.end())
        return nullptr;

    return *it;
}

std::shared_ptr<Server> ServerService::require(const std::string &name) const
{
    auto server = get(name);

    if (!server)
        throw std::invalid_argument("server {} not found"_format(name));

    return server;
}

void ServerService::remove(const std::string &name)
{
    auto it = std::find_if(m_servers.begin(), m_servers.end(), [&] (const auto &server) {
        return server->name() == name;
    });

    if (it != m_servers.end()) {
        (*it)->disconnect();
        m_servers.erase(it);
    }
}

void ServerService::clear() noexcept
{
    for (auto &server : m_servers)
        server->disconnect();

    m_servers.clear();
}

/*
 * TransportService.
 * ------------------------------------------------------------------
 */

void TransportService::handleCommand(std::weak_ptr<TransportClient> ptr, const nlohmann::json &object)
{
    assert(object.is_object());

    m_irccd.post([=] (Irccd &) {
        // 0. Be sure the object still exists.
        auto tc = ptr.lock();

        if (!tc)
            return;

        auto name = object.find("command");
        if (name == object.end() || !name->is_string()) {
            // TODO: send error.
            log::warning("invalid command object");
            return;
        }

        auto cmd = m_irccd.commands().find(*name);

        if (!cmd)
            tc->error(*name, "command does not exist");
        else {
            try {
                cmd->exec(m_irccd, *tc, object);
            } catch (const std::exception &ex) {
                tc->error(cmd->name(), ex.what());
            }
        }
    });
}

void TransportService::handleDie(std::weak_ptr<TransportClient> ptr)
{
    m_irccd.post([=] (Irccd &) {
        log::info("transport: client disconnected");

        auto tc = ptr.lock();

        if (tc)
            m_clients.erase(std::find(m_clients.begin(), m_clients.end(), tc));
    });
}

TransportService::TransportService(Irccd &irccd) noexcept
    : m_irccd(irccd)
{
}

void TransportService::prepare(fd_set &in, fd_set &out, net::Handle &max)
{
    // Add transport servers.
    for (const auto &transport : m_servers) {
        FD_SET(transport->handle(), &in);

        if (transport->handle() > max)
            max = transport->handle();
    }

    // Transport clients.
    for (const auto &client : m_clients)
        client->prepare(in, out, max);
}

void TransportService::sync(fd_set &in, fd_set &out)
{
    using namespace std::placeholders;

    // Transport clients.
    for (const auto &client : m_clients) {
        try {
            client->sync(in, out);
        } catch (const std::exception &ex) {
            log::info() << "transport: client disconnected: " << ex.what() << std::endl;
            handleDie(client);
        }
    }

    // Transport servers.
    for (const auto &transport : m_servers) {
        if (!FD_ISSET(transport->handle(), &in))
            continue;

        log::debug("transport: new client connected");

        std::shared_ptr<TransportClient> client = transport->accept();
        std::weak_ptr<TransportClient> ptr(client);

        try {
            // Connect signals.
            client->onCommand.connect(std::bind(&TransportService::handleCommand, this, ptr, _1));
            client->onDie.connect(std::bind(&TransportService::handleDie, this, ptr));

            // Register it.
            m_clients.push_back(std::move(client));
        } catch (const std::exception &ex) {
            log::info() << "transport: client disconnected: " << ex.what() << std::endl;
        }
    }
}

void TransportService::add(std::shared_ptr<TransportServer> ts)
{
    m_servers.push_back(std::move(ts));
}

void TransportService::broadcast(const nlohmann::json &json)
{
    assert(json.is_object());

    for (const auto &client : m_clients)
        if (client->state() == TransportClient::Ready)
            client->send(json);
}

} // !irccd
