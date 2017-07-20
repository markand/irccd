/*
 * command.cpp -- implementation of plugin-config command
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

#include "command.hpp"
#include "irccd.hpp"
#include "service.hpp"
#include "transport.hpp"
#include "util.hpp"

using namespace std::string_literals;

namespace irccd {

namespace command {

namespace {

void execSet(Irccd &, TransportClient &client, Plugin &plugin, const nlohmann::json &args)
{
    assert(args.count("value") > 0);

    auto var = args.find("variable");
    auto value = args.find("value");

    if (var == args.end() || !var->is_string())
        client.error("plugin-config", "missing 'variable' property (string expected)");
    else if (!value->is_string())
        client.error("plugin-config", "invalid 'value' property (string expected)");
    else {
        auto config = plugin.config();

        config[*var] = *value;
        plugin.setConfig(config);
        client.success("plugin-config");
    }
}

void execGet(Irccd &, TransportClient &client, Plugin &plugin, const nlohmann::json &args)
{
    auto variables = nlohmann::json::object();
    auto var = args.find("variable");

    if (var != args.end() && var->is_string())
        variables[var->get<std::string>()] = plugin.config()[*var];
    else
        for (const auto &pair : plugin.config())
            variables[pair.first] = pair.second;

    /*
     * Don't put all variables into the response, put them into a sub property
     * 'variables' instead.
     *
     * It's easier for the client to iterate over all.
     */
    client.success("plugin-config", {
        { "variables", variables }
    });
}

nlohmann::json toJson(const Rule &rule)
{
    auto join = [] (const auto &set) {
        auto array = nlohmann::json::array();

        for (const auto &entry : set)
            array.push_back(entry);

        return array;
    };
    auto str = [] (auto action) {
        switch (action) {
        case RuleAction::Accept:
            return "accept";
        default:
            return "drop";
        }
    };

    return {
        { "servers",    join(rule.servers())    },
        { "channels",   join(rule.channels())   },
        { "plugins",    join(rule.plugins())    },
        { "events",     join(rule.events())     },
        { "action",     str(rule.action())      }
    };
}

Rule fromJson(const nlohmann::json &json)
{
    auto toset = [] (auto object, auto name) -> RuleSet {
        RuleSet result;

        for (const auto &s : object[name])
            if (s.is_string())
                result.insert(s.template get<std::string>());

        return result;
    };
    auto toaction = [] (auto object, auto name) -> RuleAction {
        auto v = object[name];

        if (!v.is_string())
            throw std::runtime_error("no action given");

        auto s = v.template get<std::string>();
        if (s == "accept")
            return RuleAction::Accept;
        if (s == "drop")
            return RuleAction::Drop;

        throw std::runtime_error("unknown action '"s + s + "' given");
    };

    return {
        toset(json, "servers"),
        toset(json, "channels"),
        toset(json, "origins"),
        toset(json, "plugins"),
        toset(json, "events"),
        toaction(json, "action")
    };
}

} // !namespace

PluginConfigCommand::PluginConfigCommand()
    : Command("plugin-config")
{
}

void PluginConfigCommand::exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args)
{
    auto plugin = irccd.plugins().require(util::json::requireIdentifier(args, "plugin"));

    if (args.count("value") > 0)
        execSet(irccd, client, *plugin, args);
    else
        execGet(irccd, client, *plugin, args);
}


PluginInfoCommand::PluginInfoCommand()
    : Command("plugin-info")
{
}

void PluginInfoCommand::exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args)
{
    auto plugin = irccd.plugins().require(util::json::requireIdentifier(args, "plugin"));

    client.success("plugin-info", {
        { "author",     plugin->author()    },
        { "license",    plugin->license()   },
        { "summary",    plugin->summary()   },
        { "version",    plugin->version()   }
    });
}

PluginListCommand::PluginListCommand()
    : Command("plugin-list")
{
}

void PluginListCommand::exec(Irccd &irccd, TransportClient &client, const nlohmann::json &)
{
    auto list = nlohmann::json::array();

    for (const auto &plugin : irccd.plugins().list())
        list += plugin->name();

    client.success("plugin-list", {
        { "list", list }
    });
}

PluginLoadCommand::PluginLoadCommand()
    : Command("plugin-load")
{
}

void PluginLoadCommand::exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args)
{
    irccd.plugins().load(util::json::requireIdentifier(args, "plugin"));
    client.success("plugin-load");
}

PluginReloadCommand::PluginReloadCommand()
    : Command("plugin-reload")
{
}

void PluginReloadCommand::exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args)
{
    irccd.plugins().require(util::json::requireIdentifier(args, "plugin"))->onReload(irccd);
    client.success("plugin-reload");
}

PluginUnloadCommand::PluginUnloadCommand()
    : Command("plugin-unload")
{
}

void PluginUnloadCommand::exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args)
{
    irccd.plugins().unload(util::json::requireIdentifier(args, "plugin"));
    client.success("plugin-unload");
}

ServerChannelModeCommand::ServerChannelModeCommand()
    : Command("server-cmode")
{
}

void ServerChannelModeCommand::exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args)
{
    irccd.servers().require(util::json::requireIdentifier(args, "server"))->cmode(
        util::json::requireString(args, "channel"),
        util::json::requireString(args, "mode")
    );
    client.success("server-cmode");
}

ServerChannelNoticeCommand::ServerChannelNoticeCommand()
    : Command("server-cnotice")
{
}

void ServerChannelNoticeCommand::exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args)
{
    irccd.servers().require(util::json::requireString(args, "server"))->cnotice(
        util::json::requireString(args, "channel"),
        util::json::requireString(args, "message")
    );
    client.success("server-cnotice");
}

ServerConnectCommand::ServerConnectCommand()
    : Command("server-connect")
{
}

void ServerConnectCommand::exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args)
{
    auto server = Server::fromJson(args);

    if (irccd.servers().has(server->name()))
        client.error("server-connect", "server already exists");
    else {
        irccd.servers().add(std::move(server));
        client.success("server-connect");
    }
}

ServerDisconnectCommand::ServerDisconnectCommand()
    : Command("server-disconnect")
{
}

void ServerDisconnectCommand::exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args)
{
    auto it = args.find("server");

    if (it == args.end())
        irccd.servers().clear();
    else
        irccd.servers().remove(*it);

    client.success("server-disconnect");
}

ServerInfoCommand::ServerInfoCommand()
    : Command("server-info")
{
}

void ServerInfoCommand::exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args)
{
    auto response = nlohmann::json::object();
    auto server = irccd.servers().require(util::json::requireIdentifier(args, "server"));

    // General stuff.
    response.push_back({"name", server->name()});
    response.push_back({"host", server->host()});
    response.push_back({"port", server->port()});
    response.push_back({"nickname", server->nickname()});
    response.push_back({"username", server->username()});
    response.push_back({"realname", server->realname()});
    response.push_back({"channels", server->channels()});

    // Optional stuff.
    if (server->flags() & Server::Ipv6)
        response.push_back({"ipv6", true});
    if (server->flags() & Server::Ssl)
        response.push_back({"ssl", true});
    if (server->flags() & Server::SslVerify)
        response.push_back({"sslVerify", true});

    client.success("server-info", response);
}

ServerInviteCommand::ServerInviteCommand()
    : Command("server-invite")
{
}

void ServerInviteCommand::exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args)
{
    irccd.servers().require(util::json::requireIdentifier(args, "server"))->invite(
        util::json::requireString(args, "target"),
        util::json::requireString(args, "channel")
    );
    client.success("server-invite");
}

ServerJoinCommand::ServerJoinCommand()
    : Command("server-join")
{
}

void ServerJoinCommand::exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args)
{
    irccd.servers().require(util::json::requireIdentifier(args, "server"))->join(
        util::json::requireString(args, "channel"),
        util::json::getString(args, "password")
    );
    client.success("server-join");
}

ServerKickCommand::ServerKickCommand()
    : Command("server-kick")
{
}

void ServerKickCommand::exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args)
{
    irccd.servers().require(util::json::requireIdentifier(args, "server"))->kick(
        util::json::requireString(args, "target"),
        util::json::requireString(args, "channel"),
        util::json::getString(args, "reason")
    );
    client.success("server-kick");
}

ServerListCommand::ServerListCommand()
    : Command("server-list")
{
}

void ServerListCommand::exec(Irccd &irccd, TransportClient &client, const nlohmann::json &)
{
    auto json = nlohmann::json::object();
    auto list = nlohmann::json::array();

    for (const auto &server : irccd.servers().servers())
        list.push_back(server->name());

    json.push_back({"list", std::move(list)});
    client.success("server-list", json);
}

ServerMeCommand::ServerMeCommand()
    : Command("server-me")
{
}

void ServerMeCommand::exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args)
{
    irccd.servers().require(util::json::requireIdentifier(args, "server"))->me(
        util::json::requireString(args, "target"),
        util::json::requireString(args, "message")
    );
    client.success("server-me");
}

ServerMessageCommand::ServerMessageCommand()
    : Command("server-message")
{
}

void ServerMessageCommand::exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args)
{
    irccd.servers().require(util::json::requireIdentifier(args, "server"))->message(
        util::json::requireString(args, "target"),
        util::json::requireString(args, "message")
    );
    client.success("server-message");
}

ServerModeCommand::ServerModeCommand()
    : Command("server-mode")
{
}

void ServerModeCommand::exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args)
{
    irccd.servers().require(util::json::requireIdentifier(args, "server"))->mode(
        util::json::requireString(args, "mode")
    );
    client.success("server-mode");
}

ServerNickCommand::ServerNickCommand()
    : Command("server-nick")
{
}

void ServerNickCommand::exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args)
{
    irccd.servers().require(util::json::requireIdentifier(args, "server"))->setNickname(
        util::json::requireString(args, "nickname")
    );
    client.success("server-nick");
}

ServerNoticeCommand::ServerNoticeCommand()
    : Command("server-notice")
{
}

void ServerNoticeCommand::exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args)
{
    irccd.servers().require(util::json::requireIdentifier(args, "server"))->notice(
        util::json::requireString(args, "target"),
        util::json::requireString(args, "message")
    );
    client.success("server-notice");
}

ServerPartCommand::ServerPartCommand()
    : Command("server-part")
{
}

void ServerPartCommand::exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args)
{
    irccd.servers().require(util::json::requireIdentifier(args, "server"))->part(
        util::json::requireString(args, "channel"),
        util::json::getString(args, "reason")
    );
    client.success("server-part");
}

ServerReconnectCommand::ServerReconnectCommand()
    : Command("server-reconnect")
{
}

void ServerReconnectCommand::exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args)
{
    auto server = args.find("server");

    if (server != args.end() && server->is_string())
        irccd.servers().require(*server)->reconnect();
    else
        for (auto &server : irccd.servers().servers())
            server->reconnect();

    client.success("server-reconnect");
}

ServerTopicCommand::ServerTopicCommand()
    : Command("server-topic")
{
}

void ServerTopicCommand::exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args)
{
    irccd.servers().require(util::json::requireIdentifier(args, "server"))->topic(
        util::json::requireString(args, "channel"),
        util::json::requireString(args, "topic")
    );
    client.success("server-topic");
}

RuleEditCommand::RuleEditCommand()
    : Command("rule-edit")
{
}

void RuleEditCommand::exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args)
{
    static const auto updateset = [] (auto &set, auto args, const auto &key) {
        for (const auto &v : args["remove-"s + key]) {
            if (v.is_string())
                set.erase(v.template get<std::string>());
        }
        for (const auto &v : args["add-"s + key]) {
            if (v.is_string())
                set.insert(v.template get<std::string>());
        }
    };

    // Create a copy to avoid incomplete edition in case of errors.
    auto index = util::json::requireUint(args, "index");
    auto rule = irccd.rules().require(index);

    updateset(rule.channels(), args, "channels");
    updateset(rule.events(), args, "events");
    updateset(rule.plugins(), args, "plugins");
    updateset(rule.servers(), args, "servers");

    auto action = args.find("action");

    if (action != args.end()) {
        if (!action->is_string()) {
            client.error("rule-edit", "action must be \"accept\" or \"drop\"");
            return;
        }

        if (action->get<std::string>() == "accept")
            rule.setAction(RuleAction::Accept);
        else if (action->get<std::string>() == "drop")
            rule.setAction(RuleAction::Drop);
        else {
            client.error("rule-edit", "invalid action '"s + action->get<std::string>() + "'");
            return;
        }
    }

    // All done, sync the rule.
    irccd.rules().require(index) = rule;
    client.success("rule-edit");
}

RuleListCommand::RuleListCommand()
    : Command("rule-list")
{
}

void RuleListCommand::exec(Irccd &irccd, TransportClient &client, const nlohmann::json &)
{
    auto array = nlohmann::json::array();

    for (const auto& rule : irccd.rules().list())
        array.push_back(toJson(rule));

    client.success("rule-list", {{ "list", std::move(array) }});
}

RuleInfoCommand::RuleInfoCommand()
    : Command("rule-info")
{
}

void RuleInfoCommand::exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args)
{
    client.success("rule-info", toJson(irccd.rules().require(util::json::requireUint(args, "index"))));
}

RuleRemoveCommand::RuleRemoveCommand()
    : Command("rule-remove")
{
}

void RuleRemoveCommand::exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args)
{
    unsigned position = util::json::requireUint(args, "index");

    if (irccd.rules().length() == 0)
        client.error("rule-remove", "rule list is empty");
    if (position >= irccd.rules().length())
        client.error("rule-remove", "index is out of range");
    else {
        irccd.rules().remove(position);
        client.success("rule-remove");
    }
}

RuleMoveCommand::RuleMoveCommand()
    : Command("rule-move")
{
}

void RuleMoveCommand::exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args)
{
    auto from = util::json::requireUint(args, "from");
    auto to = util::json::requireUint(args, "to");

    /*
     * Examples of moves
     * --------------------------------------------------------------
     *
     * Before: [0] [1] [2]
     *
     * from = 0
     * to   = 2
     *
     * After:  [1] [2] [0]
     *
     * --------------------------------------------------------------
     *
     * Before: [0] [1] [2]
     *
     * from = 2
     * to   = 0
     *
     * After:  [2] [0] [1]
     *
     * --------------------------------------------------------------
     *
     * Before: [0] [1] [2]
     *
     * from = 0
     * to   = 123
     *
     * After:  [1] [2] [0]
     */

    // Ignore dump input.
    if (from == to)
        client.success("rule-move");
    else if (from >= irccd.rules().length())
        client.error("rule-move", "rule source index is out of range");
    else {
        auto save = irccd.rules().list()[from];

        irccd.rules().remove(from);
        irccd.rules().insert(save, to > irccd.rules().length() ? irccd.rules().length() : to);
        client.success("rule-move");
    }
}

RuleAddCommand::RuleAddCommand()
    : Command("rule-add")
{
}

void RuleAddCommand::exec(Irccd &irccd, TransportClient &client, const nlohmann::json &args)
{
    auto index = util::json::getUint(args, "index", irccd.rules().length());
    auto rule = fromJson(args);

    if (index > irccd.rules().length())
        client.error("rule-add", "index is out of range");
    else {
        irccd.rules().insert(rule, index);
        client.success("rule-add");
    }
}

} // !command

} // !irccd
