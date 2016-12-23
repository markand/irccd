/*
 * cli.cpp -- command line for irccdctl
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

#include <iostream>

#include <json.hpp>

#include "cli.hpp"
#include "elapsed-timer.hpp"
#include "irccdctl.hpp"
#include "logger.hpp"
#include "options.hpp"
#include "util.hpp"

namespace irccd {

/*
 * Cli.
 * ------------------------------------------------------------------
 */

void Cli::check(const nlohmann::json &response)
{
    if (!util::json::getBool(response, "status", false)) {
        auto error = util::json::getString(response, "error");

        if (error.empty())
            throw std::runtime_error("command failed with an unknown error");

        throw std::runtime_error(error);
    }
}

nlohmann::json Cli::request(Irccdctl &irccdctl, nlohmann::json args)
{
    auto msg = nlohmann::json();

    if (!args.is_object())
        args = nlohmann::json::object();

    args.push_back({"command", m_name});
    irccdctl.client().request(args);

    auto id = irccdctl.client().onMessage.connect([&] (auto input) {
        msg = std::move(input);
    });

    try {
        ElapsedTimer timer;

        while (!msg.is_object() && timer.elapsed() < 3000)
            util::poller::poll(3000 - timer.elapsed(), irccdctl);
    } catch (const std::exception &) {
        irccdctl.client().onMessage.disconnect(id);
        throw;
    }

    irccdctl.client().onMessage.disconnect(id);

    if (!msg.is_object())
        throw std::runtime_error("no response received");
    if (util::json::getString(msg, "command") != m_name)
        throw std::runtime_error("unexpected command result received");

    check(msg);

    return msg;
}

void Cli::call(Irccdctl &irccdctl, nlohmann::json args)
{
    check(request(irccdctl, args));
}

namespace cli {

/*
 * PluginConfigCli.
 * ------------------------------------------------------------------
 */

void PluginConfigCli::set(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    check(request(irccdctl, nlohmann::json::object({
        { "plugin", args[0] },
        { "variable", args[1] },
        { "value", args[2] }
    })));
}

void PluginConfigCli::get(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    auto result = request(irccdctl, nlohmann::json::object({
        { "plugin", args[0] },
        { "variable", args[1] }
    }));

    check(result);

    if (result["variables"].is_object())
        std::cout << util::json::pretty(result["variables"][args[1]]) << std::endl;
}

void PluginConfigCli::getall(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    auto result = request(irccdctl, nlohmann::json::object({{ "plugin", args[0] }}));

    check(result);

    auto variables = result["variables"];

    for (auto v = variables.begin(); v != variables.end(); ++v)
        std::cout << std::setw(16) << std::left << v.key() << " : " << util::json::pretty(v.value()) << std::endl;
}

PluginConfigCli::PluginConfigCli()
    : Cli("plugin-config",
          "configure a plugin",
          "plugin-config plugin [variable] [value]",
          "Get or set a plugin configuration variable.\n\n"
          "If both variable and value are provided, sets the plugin configuration "
          "to the\nrespective variable name and value.\n\n"
          "If only variable is specified, shows its current value. Otherwise, list "
          "all\nvariables and their values.\n\n"
          "Examples:\n"
          "\tirccdctl plugin-config ask")
{
}

void PluginConfigCli::exec(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    switch (args.size()) {
    case 3:
        set(irccdctl, args);
        break;
    case 2:
        get(irccdctl, args);
        break;
    case 1:
        getall(irccdctl, args);
        break;
    default:
        throw std::invalid_argument("plugin-config requires at least 1 argument");
    }
}

/*
 * PluginInfoCli.
 * ------------------------------------------------------------------
 */

PluginInfoCli::PluginInfoCli()
    : Cli("plugin-info",
          "get plugin information",
          "plugin-info plugin",
          "Get plugin information.\n\n"
          "Example:\n"
          "\tirccdctl plugin-info ask"
    )
{
}

void PluginInfoCli::exec(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    if (args.size() < 1)
        throw std::invalid_argument("plugin-info requires 1 argument");

    auto result = request(irccdctl, {{ "plugin", args[0] }});

    std::cout << std::boolalpha;
    std::cout << "Author         : " << util::json::getString(result, "author") << std::endl;
    std::cout << "License        : " << util::json::getString(result, "license") << std::endl;
    std::cout << "Summary        : " << util::json::getString(result, "summary") << std::endl;
    std::cout << "Version        : " << util::json::getString(result, "version") << std::endl;
}

/*
 * PluginListCli.
 * ------------------------------------------------------------------
 */

PluginListCli::PluginListCli()
    : Cli("plugin-list",
          "list loaded plugins",
          "plugin-list",
          "Get the list of all loaded plugins.\n\n"
          "Example:\n"
          "\tirccdctl plugin-list")
{
}

void PluginListCli::exec(Irccdctl &irccdctl, const std::vector<std::string> &)
{
    auto result = request(irccdctl);

    for (const auto &value : result["list"])
        if (value.is_string())
            std::cout << value.get<std::string>() << std::endl;
}

/*
 * PluginLoadCli.
 * ------------------------------------------------------------------
 */

PluginLoadCli::PluginLoadCli()
    : Cli("plugin-load",
          "load a plugin",
          "plugin-load logger",
          "Load a plugin into the irccd instance.\n\n"
          "Example:\n"
          "\tirccdctl plugin-load logger")
{
}

void PluginLoadCli::exec(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    if (args.size() < 1)
        throw std::invalid_argument("plugin-load requires 1 argument");

    check(request(irccdctl, {{"plugin", args[0]}}));
}

/*
 * PluginReloadCli.
 * ------------------------------------------------------------------
 */

PluginReloadCli::PluginReloadCli()
    : Cli("plugin-reload",
          "reload a plugin",
          "plugin-reload plugin",
          "Reload a plugin by calling the appropriate onReload event, the plugin is not\n"
          "unloaded and must be already loaded.\n\n"
          "Example:\n"
          "\tirccdctl plugin-reload logger")
{
}

void PluginReloadCli::exec(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    if (args.size() < 1)
        throw std::invalid_argument("plugin-reload requires 1 argument");

    check(request(irccdctl, {{ "plugin", args[0] }}));
}

/*
 * PluginUnloadCli.
 * ------------------------------------------------------------------
 */

PluginUnloadCli::PluginUnloadCli()
    : Cli("plugin-unload",
          "unload a plugin",
          "plugin-unload plugin",
          "Unload a loaded plugin from the irccd instance.\n\n"
          "Example:\n"
          "\tirccdctl plugin-unload logger")
{
}

void PluginUnloadCli::exec(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    if (args.size() < 1)
        throw std::invalid_argument("plugin-unload requires 1 argument");

    check(request(irccdctl, {{ "plugin", args[0] }}));
}

/*
 * ServerChannelCli.
 * ------------------------------------------------------------------
 */

ServerChannelMode::ServerChannelMode()
    : Cli("server-cmode",
          "change channel mode",
          "server-cmode server channel mode",
          "Change the mode of the specified channel.\n\n"
          "Example:\n"
          "\tirccdctl server-cmode freenode #staff +t")
{
}

void ServerChannelMode::exec(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    if (args.size() < 3)
        throw std::invalid_argument("server-cmode requires 3 arguments");

    check(request(irccdctl, {
        { "command",    "server-cmode"  },
        { "server",     args[0]         },
        { "channel",    args[1]         },
        { "mode",       args[2]         }
    }));
}

/*
 * ServerChannelNoticeCli.
 * ------------------------------------------------------------------
 */

ServerChannelNoticeCli::ServerChannelNoticeCli()
    : Cli("server-cnotice",
          "send a channel notice",
          "server-cnotice server channel message",
          "Send a notice to a public channel. This is a notice that everyone on the channel\n"
          "will receive.\n\n"
          "Example:\n"
          "\tirccdctl server-cnotice freenode #staff \"Don't flood!\"")
{
}

void ServerChannelNoticeCli::exec(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    if (args.size() < 3)
        throw std::invalid_argument("server-cnotice requires 3 arguments");

    check(request(irccdctl, {
        { "command",    "server-cnotice"    },
        { "server",     args[0]             },
        { "channel",    args[1]             },
        { "message",    args[2]             }
    }));
}

/*
 * ServerConnectCli.
 * ------------------------------------------------------------------
 */

namespace {

option::Result parse(std::vector<std::string> &args)
{
    option::Options options{
        { "-c",             true    },
        { "--command",      true    },
        { "-n",             true    },
        { "--nickname",     true    },
        { "-r",             true    },
        { "--realname",     true    },
        { "-S",             false   },
        { "--ssl-verify",   false   },
        { "-s",             false   },
        { "--ssl",          false   },
        { "-u",             true    },
        { "--username",     true    }
    };

    return option::read(args, options);
}

} // !namespace

ServerConnectCli::ServerConnectCli()
    : Cli("server-connect",
          "add a server",
          "server-connect [options] id host [port]",
          "Connect to a new IRC server.\n\n"
          "Available options:\n"
          "  -c, --command\t\tspecify the command char\n"
          "  -n, --nickname\tspecify a nickname\n"
          "  -r, --realname\tspecify a real name\n"
          "  -S, --ssl-verify\tverify SSL\n"
          "  -s, --ssl\t\tconnect using SSL\n"
          "  -u, --username\tspecify a user name\n\n"
          "Example:\n"
          "\tirccdctl server-connect -n jean example irc.example.org\n"
          "\tirccdctl server-connect --ssl example irc.example.org 6697")
{
}

void ServerConnectCli::exec(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    std::vector<std::string> copy(args);

    option::Result result = parse(copy);
    option::Result::const_iterator it;

    if (copy.size() < 2)
        throw std::invalid_argument("server-connect requires at least 2 arguments");

    auto object = nlohmann::json::object({
        { "name", copy[0] },
        { "host", copy[1] }
    });

    if (copy.size() == 3) {
        if (!util::isNumber(copy[2]))
            throw std::invalid_argument("invalid port number");

        object["port"] = std::stoi(copy[2]);
    }

    if (result.count("-S") > 0 || result.count("--ssl-verify") > 0)
        object["sslVerify"] = true;
    if (result.count("-s") > 0 || result.count("--ssl") > 0)
        object["ssl"] = true;
    if ((it = result.find("-n")) != result.end() || (it = result.find("--nickname")) != result.end())
        object["nickname"] = it->second;
    if ((it = result.find("-r")) != result.end() || (it = result.find("--realname")) != result.end())
        object["realname"] = it->second;
    if ((it = result.find("-u")) != result.end() || (it = result.find("--username")) != result.end())
        object["username"] = it->second;

    check(request(irccdctl, object));
}

/*
 * ServerDisconnectCli.
 * ------------------------------------------------------------------
 */

ServerDisconnectCli::ServerDisconnectCli()
    : Cli("server-disconnect",
          "disconnect server",
          "server-disconnect [server]",
          "Disconnect from a server.\n\n"
          "If server is not specified, irccd disconnects all servers.\n\n"
          "Example:\n"
          "\tirccdctl server-disconnect localhost")
{
}

void ServerDisconnectCli::exec(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    auto object = nlohmann::json::object({
        { "command", "server-disconnect" }
    });

    if (args.size() > 0)
        object["server"] = args[0];

    check(request(irccdctl, object));
}

/*
 * ServerInfoCli.
 * ------------------------------------------------------------------
 */

ServerInfoCli::ServerInfoCli()
    : Cli("server-info",
          "get server information",
          "server-info server",
          "Get information about a server.\n\n"
          "Example:\n"
          "\tirccdctl server-info freenode")
{
}

void ServerInfoCli::exec(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    if (args.size() < 1)
        throw std::invalid_argument("server-info requires 1 argument");

    auto result = request(irccdctl, {
        { "command",    "server-info"   },
        { "server",     args[0]         }
    });

    check(result);

    std::cout << std::boolalpha;
    std::cout << "Name           : " << util::json::pretty(result["name"]) << std::endl;
    std::cout << "Host           : " << util::json::pretty(result["host"]) << std::endl;
    std::cout << "Port           : " << util::json::pretty(result["port"]) << std::endl;
    std::cout << "Ipv6           : " << util::json::pretty(result["ipv6"]) << std::endl;
    std::cout << "SSL            : " << util::json::pretty(result["ssl"]) << std::endl;
    std::cout << "SSL verified   : " << util::json::pretty(result["sslVerify"]) << std::endl;
    std::cout << "Channels       : ";

    for (const auto &v : result["channels"])
        if (v.is_string())
            std::cout << v.get<std::string>() << " ";

    std::cout << std::endl;

    std::cout << "Nickname       : " << util::json::pretty(result["nickname"]) << std::endl;
    std::cout << "User name      : " << util::json::pretty(result["username"]) << std::endl;
    std::cout << "Real name      : " << util::json::pretty(result["realname"]) << std::endl;
}

/*
 * ServerInviteCli.
 * ------------------------------------------------------------------
 */

ServerInviteCli::ServerInviteCli()
    : Cli("server-invite",
          "invite someone",
          "server-invite server nickname channel",
          "Invite the specified target on the channel.\n\n"
          "Example:\n"
          "\tirccdctl server-invite freenode xorg62 #staff")
{
}

void ServerInviteCli::exec(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    if (args.size() < 3)
        throw std::invalid_argument("server-invite requires 3 arguments");

    check(request(irccdctl, {
        { "command",    "server-invite" },
        { "server",     args[0]         },
        { "target",     args[1]         },
        { "channel",    args[2]         }
    }));
}

/*
 * ServerJoinCli.
 * ------------------------------------------------------------------
 */

ServerJoinCli::ServerJoinCli()
    : Cli("server-join",
          "join a channel",
          "server-join server channel [password]",
          "Join the specified channel, the password is optional.\n\n"
          "Example:\n"
          "\tirccdctl server-join freenode #test\n"
          "\tirccdctl server-join freenode #private-club secret")
{
}

void ServerJoinCli::exec(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    if (args.size() < 2)
        throw std::invalid_argument("server-join requires at least 2 arguments");

    auto object = nlohmann::json::object({
        { "server",     args[0]         },
        { "channel",    args[1]         }
    });

    if (args.size() == 3)
        object["password"] = args[2];

    check(request(irccdctl, object));
}

/*
 * ServerKickCli.
 * ------------------------------------------------------------------
 */

ServerKickCli::ServerKickCli()
    : Cli("server-kick",
          "kick someone from a channel",
          "server-kick server target channel [reason]",
          "Kick the specified target from the channel, the reason is optional.\n\n"
          "Example:\n"
          "\tirccdctl server-kick freenode jean #staff \"Stop flooding\"")
{
}

void ServerKickCli::exec(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    if (args.size() < 3)
        throw std::invalid_argument("server-kick requires at least 3 arguments ");

    auto object = nlohmann::json::object({
        { "server",     args[0] },
        { "target",     args[1] },
        { "channel",    args[2] }
    });

    if (args.size() == 4)
        object["reason"] = args[3];

    check(request(irccdctl, object));
}

/*
 * ServerListCli.
 * ------------------------------------------------------------------
 */

ServerListCli::ServerListCli()
    : Cli("server-list",
          "get list of servers",
          "server-list",
          "Get the list of all connected servers.\n\n"
          "Example:\n"
          "\tirccdctl server-list")
{
}

void ServerListCli::exec(Irccdctl &irccdctl, const std::vector<std::string> &)
{
    auto response = request(irccdctl);

    check(response);

    for (const auto &n : response["list"])
        if (n.is_string())
            std::cout << n.get<std::string>() << std::endl;
}

/*
 * ServerMeCli.
 * ------------------------------------------------------------------
 */

ServerMeCli::ServerMeCli()
    : Cli("server-me",
          "send an action emote",
          "server-me server target message",
          "Send an action emote.\n\n"
          "Example:\n"
          "\tirccdctl server-me freenode #staff \"going back soon\"")
{
}

void ServerMeCli::exec(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    if (args.size() < 3)
        throw std::runtime_error("server-me requires 3 arguments");

    check(request(irccdctl, {
        { "server",     args[0]     },
        { "target",     args[1]     },
        { "message",    args[2]     }
    }));
}

/*
 * ServerMessageCli.
 * ------------------------------------------------------------------
 */

ServerMessageCli::ServerMessageCli()
    : Cli("server-message",
          "send a message",
          "server-message server target message",
          "Send a message to the specified target or channel.\n\n"
          "Example:\n"
          "\tirccdctl server-message freenode #staff \"Hello from irccd\"")
{
}

void ServerMessageCli::exec(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    if (args.size() < 3)
        throw std::invalid_argument("server-message requires 3 arguments");

    check(request(irccdctl, {
        { "server",     args[0] },
        { "target",     args[1] },
        { "message",    args[2] }
    }));
}

/*
 * ServerModeCli.
 * ------------------------------------------------------------------
 */

ServerModeCli::ServerModeCli()
    : Cli("server-mode",
          "the the user mode",
          "server-mode server mode",
          "Set the irccd's user mode.\n\n"
          "Example:\n"
          "\tirccdctl server-mode +i")
{
}

void ServerModeCli::exec(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    if (args.size() < 2)
        throw std::invalid_argument("server-mode requires 2 arguments");

    check(request(irccdctl, {
        { "server", args[0] },
        { "mode",   args[1] }
    }));
}

/*
 * ServerNickCli.
 * ------------------------------------------------------------------
 */

ServerNickCli::ServerNickCli()
    : Cli("server-nick",
          "change your nickname",
          "server-nick server nickname",
          "Change irccd's nickname.\n\n"
          "Example:\n"
          "\tirccdctl server-nick freenode david")
{
}

void ServerNickCli::exec(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    if (args.size() < 2)
        throw std::invalid_argument("server-nick requires 2 arguments");

    check(request(irccdctl, {
        { "server",     args[0] },
        { "nickname",   args[1] }
    }));
}

/*
 * ServerNoticeCli.
 * ------------------------------------------------------------------
 */

ServerNoticeCli::ServerNoticeCli()
    : Cli("server-notice",
          "send a private notice",
          "server-notice server target message",
          "Send a private notice to the specified target.\n\n"
          "Example:\n"
          "\tirccdctl server-notice freenode jean \"I know you are here.\"")
{
}

void ServerNoticeCli::exec(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    if (args.size() < 3)
        throw std::invalid_argument("server-notice requires 3 arguments");

    check(request(irccdctl, {
        { "server",     args[0] },
        { "target",     args[1] },
        { "message",    args[2] }
    }));
}

/*
 * ServerPartCli.
 * ------------------------------------------------------------------
 */

ServerPartCli::ServerPartCli()
    : Cli("server-part",
          "leave a channel",
          "server-part server channel [reason]",
          "Leave the specified channel, the reason is optional.\n\n"
          "Not all IRC servers support giving a reason to leave a channel, do not specify\n"
          "it if this is a concern.\n\n"
          "Example:\n"
          "\tirccdctl server-part freenode #staff\n"
          "\tirccdctl server-part freenode #botwar \"too noisy\"")
{
}

void ServerPartCli::exec(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    if (args.size() < 2)
        throw std::invalid_argument("server-part requires at least 2 arguments");

    auto object = nlohmann::json::object({
        { "server",     args[0] },
        { "channel",    args[1] }
    });

    if (args.size() >= 3)
        object["reason"] = args[2];

    check(request(irccdctl, object));
}

/*
 * ServerReconnectCli.
 * ------------------------------------------------------------------
 */

ServerReconnectCli::ServerReconnectCli()
    : Cli("server-reconnect",
          "force reconnection of a server",
          "server-reconnect [server]",
          "Force reconnection of one or all servers.\n\n"
          "If server is not specified, all servers will try to reconnect.\n\n"
          "Example:\n"
          "\tirccdctl server-reconnect\n"
          "\tirccdctl server-reconnect wanadoo")
{
}

void ServerReconnectCli::exec(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    auto object = nlohmann::json::object({
        { "command", "server-reconnect" }
    });

    if (args.size() >= 1)
        object["server"] = args[0];

    check(request(irccdctl, object));
}

/*
 * ServerTopicCli.
 * ------------------------------------------------------------------
 */

ServerTopicCli::ServerTopicCli()
    : Cli("server-topic",
          "change channel topic",
          "server-topic server channel topic",
          "Change the topic of the specified channel.\n\n"
          "Example:\n"
          "\tirccdctl server-topic freenode #wmfs \"This is the best channel\"")
{
}

void ServerTopicCli::exec(Irccdctl &irccdctl, const std::vector<std::string> &args)
{
    if (args.size() < 3)
        throw std::invalid_argument("server-topic requires 3 arguments");

    check(request(irccdctl, {
        { "server",     args[0] },
        { "channel",    args[1] },
        { "topic",      args[2] }
    }));
}

/*
 * WatchCli.
 * ------------------------------------------------------------------
 */

namespace {

std::string format(std::vector<std::string> args)
{
    auto result = option::read(args, {
        { "-f",         true },
        { "--format",   true }
    });

    if (result.count("-f") > 0)
        return result.find("-f")->second;
    if (result.count("--format") > 0)
        return result.find("--format")->second;

    return "native";
}

void onChannelMode(const nlohmann::json &v)
{
    std::cout << "event:       onChannelMode\n";
    std::cout << "server:      " << util::json::pretty(v, "server") << "\n";
    std::cout << "origin:      " << util::json::pretty(v, "origin") << "\n";
    std::cout << "mode:        " << util::json::pretty(v, "mode") << "\n";
    std::cout << "argument:    " << util::json::pretty(v, "argument") << "\n";
}

void onChannelNotice(const nlohmann::json &v)
{
    std::cout << "event:       onChannelNotice\n";
    std::cout << "server:      " << util::json::pretty(v, "server") << "\n";
    std::cout << "origin:      " << util::json::pretty(v, "origin") << "\n";
    std::cout << "channel:     " << util::json::pretty(v, "channel") << "\n";
    std::cout << "message:     " << util::json::pretty(v, "message") << "\n";
}

void onConnect(const nlohmann::json &v)
{
    std::cout << "event:       onConnect\n";
    std::cout << "server:      " << util::json::pretty(v, "server") << "\n";
}

void onInvite(const nlohmann::json &v)
{
    std::cout << "event:       onInvite\n";
    std::cout << "server:      " << util::json::pretty(v, "server") << "\n";
    std::cout << "origin:      " << util::json::pretty(v, "origin") << "\n";
    std::cout << "channel:     " << util::json::pretty(v, "channel") << "\n";
}

void onJoin(const nlohmann::json &v)
{
    std::cout << "event:       onJoin\n";
    std::cout << "server:      " << util::json::pretty(v, "server") << "\n";
    std::cout << "origin:      " << util::json::pretty(v, "origin") << "\n";
    std::cout << "channel:     " << util::json::pretty(v, "channel") << "\n";
}

void onKick(const nlohmann::json &v)
{
    std::cout << "event:       onKick\n";
    std::cout << "server:      " << util::json::pretty(v, "server") << "\n";
    std::cout << "origin:      " << util::json::pretty(v, "origin") << "\n";
    std::cout << "channel:     " << util::json::pretty(v, "channel") << "\n";
    std::cout << "target:      " << util::json::pretty(v, "target") << "\n";
    std::cout << "reason:      " << util::json::pretty(v, "reason") << "\n";
}

void onMessage(const nlohmann::json &v)
{
    std::cout << "event:       onMessage\n";
    std::cout << "server:      " << util::json::pretty(v, "server") << "\n";
    std::cout << "origin:      " << util::json::pretty(v, "origin") << "\n";
    std::cout << "channel:     " << util::json::pretty(v, "channel") << "\n";
    std::cout << "message:     " << util::json::pretty(v, "message") << "\n";
}

void onMe(const nlohmann::json &v)
{
    std::cout << "event:       onMe\n";
    std::cout << "server:      " << util::json::pretty(v, "server") << "\n";
    std::cout << "origin:      " << util::json::pretty(v, "origin") << "\n";
    std::cout << "target:      " << util::json::pretty(v, "target") << "\n";
    std::cout << "message:     " << util::json::pretty(v, "message") << "\n";
}

void onMode(const nlohmann::json &v)
{
    std::cout << "event:       onMode\n";
    std::cout << "server:      " << util::json::pretty(v, "server") << "\n";
    std::cout << "origin:      " << util::json::pretty(v, "origin") << "\n";
    std::cout << "mode:        " << util::json::pretty(v, "mode") << "\n";
}

void onNames(const nlohmann::json &v)
{
    std::cout << "event:       onNames\n";
    std::cout << "server:      " << util::json::pretty(v, "server") << "\n";
    std::cout << "channel:     " << util::json::pretty(v, "channel") << "\n";
    std::cout << "names:       " << util::json::pretty(v, "names") << "\n";
}

void onNick(const nlohmann::json &v)
{
    std::cout << "event:       onNick\n";
    std::cout << "server:      " << util::json::pretty(v, "server") << "\n";
    std::cout << "origin:      " << util::json::pretty(v, "origin") << "\n";
    std::cout << "nickname:    " << util::json::pretty(v, "nickname") << "\n";
}

void onNotice(const nlohmann::json &v)
{
    std::cout << "event:       onNotice\n";
    std::cout << "server:      " << util::json::pretty(v, "server") << "\n";
    std::cout << "origin:      " << util::json::pretty(v, "origin") << "\n";
    std::cout << "message:     " << util::json::pretty(v, "message") << "\n";
}

void onPart(const nlohmann::json &v)
{
    std::cout << "event:       onPart\n";
    std::cout << "server:      " << util::json::pretty(v, "server") << "\n";
    std::cout << "origin:      " << util::json::pretty(v, "origin") << "\n";
    std::cout << "channel:     " << util::json::pretty(v, "channel") << "\n";
    std::cout << "reason:      " << util::json::pretty(v, "reason") << "\n";
}

void onQuery(const nlohmann::json &v)
{
    std::cout << "event:       onQuery\n";
    std::cout << "server:      " << util::json::pretty(v, "server") << "\n";
    std::cout << "origin:      " << util::json::pretty(v, "origin") << "\n";
    std::cout << "message:     " << util::json::pretty(v, "message") << "\n";
}

void onTopic(const nlohmann::json &v)
{
    std::cout << "event:       onTopic\n";
    std::cout << "server:      " << util::json::pretty(v, "server") << "\n";
    std::cout << "origin:      " << util::json::pretty(v, "origin") << "\n";
    std::cout << "channel:     " << util::json::pretty(v, "channel") << "\n";
    std::cout << "topic:       " << util::json::pretty(v, "topic") << "\n";
}

void onWhois(const nlohmann::json &v)
{
    std::cout << "event:       onWhois\n";
    std::cout << "server:      " << util::json::pretty(v, "server") << "\n";
    std::cout << "nickname:    " << util::json::pretty(v, "nickname") << "\n";
    std::cout << "username:    " << util::json::pretty(v, "username") << "\n";
    std::cout << "host:        " << util::json::pretty(v, "host") << "\n";
    std::cout << "realname:    " << util::json::pretty(v, "realname") << "\n";
}

const std::unordered_map<std::string, std::function<void (const nlohmann::json &)>> events{
    { "onChannelMode",      onChannelMode   },
    { "onChannelNotice",    onChannelNotice },
    { "onConnect",          onConnect       },
    { "onInvite",           onInvite        },
    { "onJoin",             onJoin          },
    { "onKick",             onKick          },
    { "onMessage",          onMessage       },
    { "onMe",               onMe            },
    { "onMode",             onMode          },
    { "onNames",            onNames         },
    { "onNick",             onNick          },
    { "onNotice",           onNotice        },
    { "onPart",             onPart          },
    { "onQuery",            onQuery         },
    { "onTopic",            onTopic         },
    { "onWhois",            onWhois         }
};

} // !namespace

WatchCli::WatchCli()
    : Cli("watch",
          "watch irccd events",
          "watch [-f|--format json|native]",
          "Start watching irccd events.\n\n"
          "You can use different output formats, native is human readable format, json is\n"
          "pretty formatted json.\n\n"
          "Example:\n"
          "\tirccdctl watch\n"
          "\tirccdctl watch -f json")
{
}

void WatchCli::exec(Irccdctl& client, const std::vector<std::string>& args)
{
    std::string fmt = format(args);

    if (fmt != "native" && fmt != "json")
        throw std::invalid_argument("invalid format given: " + fmt);

    auto id = client.client().onEvent.connect([&] (auto ev) {
        try {
            auto name = ev.find("event");

            if (name == ev.end() || !name->is_string())
                return;

            auto it = events.find(*name);

            // Silently ignore to avoid breaking user output.
            if (it == events.end())
                return;

            if (fmt == "json")
                std::cout << ev.dump(4) << std::endl;
            else {
                it->second(ev);
                std::cout << std::endl;
            }
        } catch (...) {
        }
    });

    try {
        while (client.client().isConnected()) {
            util::poller::poll(500, client);
        }
    } catch (const std::exception &ex) {
        log::warning() << ex.what() << std::endl;
    }

    client.client().onEvent.disconnect(id);
}

} // !cli

} // !irccd
