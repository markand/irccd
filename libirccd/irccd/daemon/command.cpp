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

#include <irccd/json_util.hpp>
#include <irccd/string_util.hpp>
#include <irccd/util.hpp>

#include "command.hpp"
#include "irccd.hpp"
#include "rule_service.hpp"
#include "plugin_service.hpp"
#include "server_service.hpp"
#include "transport_client.hpp"

using namespace std::string_literals;

namespace irccd {

namespace {

void exec_set(transport_client& client, plugin& plugin, const nlohmann::json& args)
{
    assert(args.count("value") > 0);

    auto var = args.find("variable");
    auto value = args.find("value");

    if (var == args.end() || !var->is_string())
        throw irccd_error(irccd_error::error::incomplete_message);
    if (value == args.end() || !value->is_string())
        throw irccd_error(irccd_error::error::incomplete_message);

    auto config = plugin.config();

    config[*var] = *value;
    plugin.set_config(config);
    client.success("plugin-config");
}

void exec_get(transport_client& client, plugin& plugin, const nlohmann::json& args)
{
    auto variables = nlohmann::json::object();
    auto var = args.find("variable");

    if (var != args.end() && var->is_string())
        variables[var->get<std::string>()] = plugin.config()[*var];
    else
        for (const auto& pair : plugin.config())
            variables[pair.first] = pair.second;

    /*
     * Don't put all variables into the response, put them into a sub property
     * 'variables' instead.
     *
     * It's easier for the client to iterate over all.
     */
    client.send({
        { "command",    "plugin-config" },
        { "variables",  variables       }
    });
}

nlohmann::json to_json(const rule& rule)
{
    auto join = [] (const auto& set) {
        auto array = nlohmann::json::array();

        for (const auto& entry : set)
            array.push_back(entry);

        return array;
    };
    auto str = [] (auto action) {
        switch (action) {
        case rule::action_type::accept:
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

rule from_json(const nlohmann::json& json)
{
    auto toset = [] (auto object, auto name) {
        rule::set result;

        for (const auto& s : object[name])
            if (s.is_string())
                result.insert(s.template get<std::string>());

        return result;
    };
    auto toaction = [] (auto object, auto name) {
        auto v = object[name];

        if (!v.is_string())
            throw rule_error(rule_error::invalid_action);

        auto s = v.template get<std::string>();
        if (s == "accept")
            return rule::action_type::accept;
        if (s == "drop")
            return rule::action_type::drop;

        throw rule_error(rule_error::invalid_action);
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

unsigned get_rule_index(const nlohmann::json& json, const std::string& key = "index")
{
    auto index = json.find(key);

    if (index == json.end() || !index->is_number_integer() || index->get<int>() < 0)
        throw rule_error(rule_error::invalid_index);

    return index->get<int>();
}

std::shared_ptr<server> get_server(irccd& daemon, const nlohmann::json& args)
{
    auto id = json_util::get_string(args, "server");

    if (!string_util::is_identifier(id))
        throw server_error(server_error::invalid_identifier, "");

    auto server = daemon.servers().get(id);

    if (!server)
        throw server_error(server_error::not_found, id);

    return server;
}

} // !namespace

plugin_config_command::plugin_config_command()
    : command("plugin-config")
{
}

void plugin_config_command::exec(irccd& irccd, transport_client& client, const nlohmann::json& args)
{
    auto plugin = irccd.plugins().require(json_util::require_identifier(args, "plugin"));

    if (args.count("value") > 0)
        exec_set(client, *plugin, args);
    else
        exec_get(client, *plugin, args);
}

plugin_info_command::plugin_info_command()
    : command("plugin-info")
{
}

void plugin_info_command::exec(irccd& irccd, transport_client& client, const nlohmann::json& args)
{
    auto plugin = irccd.plugins().require(json_util::require_identifier(args, "plugin"));

    client.send({
        { "command",    "plugin-info"       },
        { "author",     plugin->author()    },
        { "license",    plugin->license()   },
        { "summary",    plugin->summary()   },
        { "version",    plugin->version()   }
    });
}

plugin_list_command::plugin_list_command()
    : command("plugin-list")
{
}

void plugin_list_command::exec(irccd& irccd, transport_client& client, const nlohmann::json&)
{
    auto list = nlohmann::json::array();

    for (const auto& plugin : irccd.plugins().list())
        list += plugin->name();

    client.send({
        { "command",    "plugin-list"   },
        { "list",       list            }
    });
}

plugin_load_command::plugin_load_command()
    : command("plugin-load")
{
}

void plugin_load_command::exec(irccd& irccd, transport_client& client, const nlohmann::json& args)
{
    irccd.plugins().load(json_util::require_identifier(args, "plugin"));
    client.success("plugin-load");
}

plugin_reload_command::plugin_reload_command()
    : command("plugin-reload")
{
}

void plugin_reload_command::exec(irccd& irccd, transport_client& client, const nlohmann::json& args)
{
    irccd.plugins().reload(json_util::require_identifier(args, "plugin"));
    client.success("plugin-reload");
}

plugin_unload_command::plugin_unload_command()
    : command("plugin-unload")
{
}

void plugin_unload_command::exec(irccd& irccd, transport_client& client, const nlohmann::json& args)
{
    irccd.plugins().unload(json_util::require_identifier(args, "plugin"));
    client.success("plugin-unload");
}

server_connect_command::server_connect_command()
    : command("server-connect")
{
}

void server_connect_command::exec(irccd& irccd, transport_client& client, const nlohmann::json& args)
{
    auto server = server_service::from_json(irccd.service(), args);

    if (irccd.servers().has(server->name()))
        throw server_error(server_error::error::already_exists, server->name());

    irccd.servers().add(std::move(server));
    client.success("server-connect");
}

server_disconnect_command::server_disconnect_command()
    : command("server-disconnect")
{
}

void server_disconnect_command::exec(irccd& irccd, transport_client& client, const nlohmann::json& args)
{
    auto it = args.find("server");

    if (it == args.end())
        irccd.servers().clear();
    else {
        if (!it->is_string())
            throw server_error(server_error::invalid_identifier, "");

        auto name = it->get<std::string>();
        auto s = irccd.servers().get(name);

        if (!s)
            throw server_error(server_error::not_found, name);

        irccd.servers().remove(name);
    }

    client.success("server-disconnect");
}

server_info_command::server_info_command()
    : command("server-info")
{
}

void server_info_command::exec(irccd& irccd, transport_client& client, const nlohmann::json& args)
{
    auto response = nlohmann::json::object();
    auto server = get_server(irccd, args);

    // General stuff.
    response.push_back({"command", "server-info"});
    response.push_back({"name", server->name()});
    response.push_back({"host", server->host()});
    response.push_back({"port", server->port()});
    response.push_back({"nickname", server->nickname()});
    response.push_back({"username", server->username()});
    response.push_back({"realname", server->realname()});
    response.push_back({"channels", server->channels()});

    // Optional stuff.
    if (server->flags() & server::ipv6)
        response.push_back({"ipv6", true});
    if (server->flags() & server::ssl)
        response.push_back({"ssl", true});
    if (server->flags() & server::ssl_verify)
        response.push_back({"sslVerify", true});

    client.send(response);
}

server_invite_command::server_invite_command()
    : command("server-invite")
{
}

void server_invite_command::exec(irccd& irccd, transport_client& client, const nlohmann::json& args)
{
    auto server = get_server(irccd, args);
    auto target = json_util::get_string(args, "target");
    auto channel = json_util::get_string(args, "channel");

    if (target.empty())
        throw server_error(server_error::invalid_nickname, server->name());
    if (channel.empty())
        throw server_error(server_error::invalid_channel, server->name());

    server->invite(target, channel);
    client.success("server-invite");
}

server_join_command::server_join_command()
    : command("server-join")
{
}

void server_join_command::exec(irccd& irccd, transport_client& client, const nlohmann::json& args)
{
    auto server = get_server(irccd, args);
    auto channel = json_util::get_string(args, "channel");
    auto password = json_util::get_string(args, "password");

    if (channel.empty())
        throw server_error(server_error::invalid_channel, server->name());

    server->join(channel, password);
    client.success("server-join");
}

server_kick_command::server_kick_command()
    : command("server-kick")
{
}

void server_kick_command::exec(irccd& irccd, transport_client& client, const nlohmann::json& args)
{
    auto server = get_server(irccd, args);
    auto target = json_util::get_string(args, "target");
    auto channel = json_util::get_string(args, "channel");
    auto reason = json_util::get_string(args, "reason");

    if (target.empty())
        throw server_error(server_error::invalid_nickname, server->name());
    if (channel.empty())
        throw server_error(server_error::invalid_channel, server->name());

    server->kick(target, channel, reason);
    client.success("server-kick");
}

server_list_command::server_list_command()
    : command("server-list")
{
}

void server_list_command::exec(irccd& irccd, transport_client& client, const nlohmann::json&)
{
    auto json = nlohmann::json::object();
    auto list = nlohmann::json::array();

    for (const auto& server : irccd.servers().servers())
        list.push_back(server->name());

    client.send({
        { "command",    "server-list"   },
        { "list",       std::move(list) }
    });
}

server_me_command::server_me_command()
    : command("server-me")
{
}

void server_me_command::exec(irccd& irccd, transport_client& client, const nlohmann::json& args)
{
    auto server = get_server(irccd, args);
    auto channel = json_util::get_string(args, "target");
    auto message = json_util::get_string(args, "message");

    if (channel.empty())
        throw server_error(server_error::invalid_channel, server->name());

    server->me(channel, message);
    client.success("server-me");
}

server_message_command::server_message_command()
    : command("server-message")
{
}

void server_message_command::exec(irccd& irccd, transport_client& client, const nlohmann::json& args)
{
    auto server = get_server(irccd, args);
    auto channel = json_util::get_string(args, "target");
    auto message = json_util::get_string(args, "message");

    if (channel.empty())
        throw server_error(server_error::invalid_channel, server->name());

    server->message(channel, message);
    client.success("server-message");
}

server_mode_command::server_mode_command()
    : command("server-mode")
{
}

void server_mode_command::exec(irccd& irccd, transport_client& client, const nlohmann::json& args)
{
    auto server = get_server(irccd, args);
    auto channel = json_util::get_string(args, "channel");
    auto mode = json_util::get_string(args, "mode");

    if (channel.empty())
        throw server_error(server_error::invalid_channel, server->name());
    if (mode.empty())
        throw server_error(server_error::invalid_mode, server->name());

    auto limit = json_util::get_string(args, "limit");
    auto user = json_util::get_string(args, "user");
    auto mask = json_util::get_string(args, "mask");

    server->mode(channel, mode, limit, user, mask);
    client.success("server-mode");
}

server_nick_command::server_nick_command()
    : command("server-nick")
{
}

void server_nick_command::exec(irccd& irccd, transport_client& client, const nlohmann::json& args)
{
    auto server = get_server(irccd, args);
    auto nick = json_util::get_string(args, "nickname");

    if (nick.empty())
        throw server_error(server_error::invalid_nickname, server->name());

    server->set_nickname(nick);
    client.success("server-nick");
}

server_notice_command::server_notice_command()
    : command("server-notice")
{
}

void server_notice_command::exec(irccd& irccd, transport_client& client, const nlohmann::json& args)
{
    auto server = get_server(irccd, args);
    auto channel = json_util::get_string(args, "target");
    auto message = json_util::get_string(args, "message");

    if (channel.empty())
        throw server_error(server_error::invalid_channel, server->name());

    server->notice(channel, message);
    client.success("server-notice");
}

server_part_command::server_part_command()
    : command("server-part")
{
}

void server_part_command::exec(irccd& irccd, transport_client& client, const nlohmann::json& args)
{
    auto server = get_server(irccd, args);
    auto channel = json_util::get_string(args, "channel");
    auto reason = json_util::get_string(args, "reason");

    if (channel.empty())
        throw server_error(server_error::invalid_channel, server->name());

    server->part(channel, reason);
    client.success("server-part");
}

server_reconnect_command::server_reconnect_command()
    : command("server-reconnect")
{
}

void server_reconnect_command::exec(irccd& irccd, transport_client& client, const nlohmann::json& args)
{
    auto server = args.find("server");

    if (server == args.end()) {
        for (auto& server : irccd.servers().servers())
            server->reconnect();
    } else {
        if (!server->is_string() || !string_util::is_identifier(server->get<std::string>()))
            throw server_error(server_error::invalid_identifier, "");

        auto name = server->get<std::string>();
        auto s = irccd.servers().get(name);

        if (!s)
            throw server_error(server_error::not_found, name);

        s->reconnect();
    }

    client.success("server-reconnect");
}

server_topic_command::server_topic_command()
    : command("server-topic")
{
}

void server_topic_command::exec(irccd& irccd, transport_client& client, const nlohmann::json& args)
{
    auto server = get_server(irccd, args);
    auto channel = json_util::get_string(args, "channel");
    auto topic = json_util::get_string(args, "topic");

    if (channel.empty())
        throw server_error(server_error::invalid_channel, server->name());

    server->topic(channel, topic);
    client.success("server-topic");
}

rule_edit_command::rule_edit_command()
    : command("rule-edit")
{
}

void rule_edit_command::exec(irccd& irccd, transport_client& client, const nlohmann::json& args)
{
    static const auto updateset = [] (auto& set, auto args, const auto& key) {
        for (const auto& v : args["remove-"s + key]) {
            if (v.is_string())
                set.erase(v.template get<std::string>());
        }
        for (const auto& v : args["add-"s + key]) {
            if (v.is_string())
                set.insert(v.template get<std::string>());
        }
    };

    // Create a copy to avoid incomplete edition in case of errors.
    auto index = get_rule_index(args);
    auto rule = irccd.rules().require(index);

    updateset(rule.channels(), args, "channels");
    updateset(rule.events(), args, "events");
    updateset(rule.plugins(), args, "plugins");
    updateset(rule.servers(), args, "servers");

    auto action = args.find("action");

    if (action != args.end()) {
        if (!action->is_string())
            throw rule_error(rule_error::error::invalid_action);

        if (action->get<std::string>() == "accept")
            rule.set_action(rule::action_type::accept);
        else if (action->get<std::string>() == "drop")
            rule.set_action(rule::action_type::drop);
        else
            throw rule_error(rule_error::invalid_action);
    }

    // All done, sync the rule.
    irccd.rules().require(index) = rule;
    client.success("rule-edit");
}

rule_list_command::rule_list_command()
    : command("rule-list")
{
}

void rule_list_command::exec(irccd& irccd, transport_client& client, const nlohmann::json&)
{
    auto array = nlohmann::json::array();

    for (const auto& rule : irccd.rules().list())
        array.push_back(to_json(rule));

    client.send({
        { "command",    "rule-list"         },
        { "list",       std::move(array)    }
    });
}

rule_info_command::rule_info_command()
    : command("rule-info")
{
}

void rule_info_command::exec(irccd& irccd, transport_client& client, const nlohmann::json& args)
{
    auto json = to_json(irccd.rules().require(get_rule_index(args)));

    json.push_back({"command", "rule-info"});
    client.send(std::move(json));
}

rule_remove_command::rule_remove_command()
    : command("rule-remove")
{
}

void rule_remove_command::exec(irccd& irccd, transport_client& client, const nlohmann::json& args)
{
    auto index = get_rule_index(args);

    if (index >= irccd.rules().length())
        throw rule_error(rule_error::invalid_index);

    irccd.rules().remove(index);
    client.success("rule-remove");
}

rule_move_command::rule_move_command()
    : command("rule-move")
{
}

void rule_move_command::exec(irccd& irccd, transport_client& client, const nlohmann::json& args)
{
    auto from = get_rule_index(args, "from");
    auto to = get_rule_index(args, "to");

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

    // Ignore dumb input.
    if (from == to) {
        client.success("rule-move");
        return;
    }

    if (from >= irccd.rules().length())
        throw rule_error(rule_error::error::invalid_index);

    auto save = irccd.rules().list()[from];

    irccd.rules().remove(from);
    irccd.rules().insert(save, to > irccd.rules().length() ? irccd.rules().length() : to);
    client.success("rule-move");
}

rule_add_command::rule_add_command()
    : command("rule-add")
{
}

void rule_add_command::exec(irccd& irccd, transport_client& client, const nlohmann::json& args)
{
    auto index = json_util::get_uint(args, "index", irccd.rules().length());
    auto rule = from_json(args);

    if (index > irccd.rules().length())
        throw rule_error(rule_error::error::invalid_index);

    irccd.rules().insert(rule, index);
    client.success("rule-add");
}

} // !irccd
