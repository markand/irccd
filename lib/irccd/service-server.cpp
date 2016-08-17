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

#include <json.hpp>
#include <format.h>

#include "irccd.hpp"
#include "logger.hpp"
#include "plugin.hpp"
#include "server.hpp"
#include "service-plugin.hpp"
#include "service-rule.hpp"
#include "service-server.hpp"
#include "service-transport.hpp"
#include "util.hpp"

using namespace fmt::literals;
using namespace nlohmann;

namespace irccd {

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
            try {
                functionExec(*plugin);
            } catch (const Exception &info) {
                log::warning() << "plugin " << plugin->name() << ": error: " << info.what() << std::endl;

                if (!info.fileName.empty())
                    log::warning() << "    " << info.fileName << ":" << info.lineNumber << std::endl;
                if (!info.stack.empty())
                    log::warning() << "    " << info.stack << std::endl;
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

    auto names = json::array();

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

} // !irccd
