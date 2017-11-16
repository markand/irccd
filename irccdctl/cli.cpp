/*
 * cli.cpp -- command line for irccdctl
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

#include <boost/system/system_error.hpp>

#include <irccd/errors.hpp>
#include <irccd/json_util.hpp>
#include <irccd/options.hpp>
#include <irccd/string_util.hpp>

#include <irccd/ctl/controller.hpp>

#include "cli.hpp"

namespace irccd {

namespace ctl {

void cli::recv_response(ctl::controller& ctl, nlohmann::json req, handler_t handler)
{
    ctl.recv([&ctl, req, handler, this] (auto code, auto message) {
        if (code)
            throw boost::system::system_error(code);

        auto c = json_util::to_string(message["command"]);

        if (c != req["command"].get<std::string>()) {
            recv_response(ctl, std::move(req), std::move(handler));
            return;
        }

        if (message["error"].is_number_integer())
            throw boost::system::system_error(static_cast<network_error>(message["error"].template get<int>()));
        if (message["error"].is_string())
            throw std::runtime_error(message["error"].template get<std::string>());

        if (handler)
            handler(std::move(message));
    });
}

void cli::request(ctl::controller& ctl, nlohmann::json req, handler_t handler)
{
    ctl.send(req, [&ctl, req, handler, this] (auto code, auto) {
        if (code)
            throw boost::system::system_error(code);

        recv_response(ctl, std::move(req), std::move(handler));
    });
}

/*
 * plugin_info_cli.
 * ------------------------------------------------------------------
 */

void plugin_config_cli::set(ctl::controller& ctl, const std::vector<std::string>&args)
{
    request(ctl, {
        { "plugin", args[0] },
        { "variable", args[1] },
        { "value", args[2] }
    });
}

void plugin_config_cli::get(ctl::controller& ctl, const std::vector<std::string>& args)
{
    auto json = nlohmann::json::object({
        { "plugin", args[0] },
        { "variable", args[1] }
    });

    request(ctl, std::move(json), [args] (auto result) {
        if (result["variables"].is_object())
            std::cout << json_util::pretty(result["variables"][args[1]]) << std::endl;
    });
}

void plugin_config_cli::getall(ctl::controller& ctl, const std::vector<std::string> &args)
{
    request(ctl, {{ "plugin", args[0] }}, [] (auto result) {
        auto variables = result["variables"];

        for (auto v = variables.begin(); v != variables.end(); ++v)
            std::cout << std::setw(16) << std::left << v.key() << " : " << json_util::pretty(v.value()) << std::endl;
    });
}

std::string plugin_config_cli::name() const
{
    return "plugin-config";
}

void plugin_config_cli::exec(ctl::controller& ctl, const std::vector<std::string> &args)
{
    switch (args.size()) {
    case 3:
        set(ctl, args);
        break;
    case 2:
        get(ctl, args);
        break;
    case 1:
        getall(ctl, args);
        break;
    default:
        throw std::invalid_argument("plugin-config requires at least 1 argument");
    }
}

/*
 * plugin_info_cli.
 * ------------------------------------------------------------------
 */

std::string plugin_info_cli::name() const
{
    return "plugin-info";
}

void plugin_info_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
    if (args.size() < 1)
        throw std::invalid_argument("plugin-info requires 1 argument");

    request(ctl, {{ "plugin", args[0] }}, [] (auto result) {
        std::cout << std::boolalpha;
        std::cout << "Author         : " << json_util::get_string(result, "author") << std::endl;
        std::cout << "License        : " << json_util::get_string(result, "license") << std::endl;
        std::cout << "Summary        : " << json_util::get_string(result, "summary") << std::endl;
        std::cout << "Version        : " << json_util::get_string(result, "version") << std::endl;
    });
}

/*
 * plugin_list_cli.
 * ------------------------------------------------------------------
 */

std::string plugin_list_cli::name() const
{
    return "plugin-list";
}

void plugin_list_cli::exec(ctl::controller& ctl, const std::vector<std::string>&)
{
    request(ctl, {{ "command", "plugin-list" }}, [] (auto result) {
        for (const auto &value : result["list"])
            if (value.is_string())
                std::cout << value.template get<std::string>() << std::endl;
    });
}

/*
 * plugin_load_cli.
 * ------------------------------------------------------------------
 */

std::string plugin_load_cli::name() const
{
    return "plugin-load";
}

void plugin_load_cli::exec(ctl::controller& ctl, const std::vector<std::string> &args)
{
    if (args.size() < 1)
        throw std::invalid_argument("plugin-load requires 1 argument");

    request(ctl, {{ "plugin", args[0] }});
}

/*
 * plugin_reload_cli.
 * ------------------------------------------------------------------
 */

std::string plugin_reload_cli::name() const
{
    return "plugin-reload";
}

void plugin_reload_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
    if (args.size() < 1)
        throw std::invalid_argument("plugin-reload requires 1 argument");

    request(ctl, {{ "plugin", args[0] }});
}

/*
 * plugin_unload_cli.
 * ------------------------------------------------------------------
 */

std::string plugin_unload_cli::name() const
{
    return "plugin-unload";
}

void plugin_unload_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
    if (args.size() < 1)
        throw std::invalid_argument("plugin-unload requires 1 argument");

    request(ctl, {{ "plugin", args[0] }});
}

/*
 * server_channel_cli.
 * ------------------------------------------------------------------
 */

std::string server_channel_mode_cli::name() const
{
    return "server-cmode";
}

void server_channel_mode_cli::exec(ctl::controller& ctl, const std::vector<std::string> &args)
{
    if (args.size() < 3)
        throw std::invalid_argument("server-cmode requires 3 arguments");

    request(ctl, {
        { "command",    "server-cmode"  },
        { "server",     args[0]         },
        { "channel",    args[1]         },
        { "mode",       args[2]         }
    });
}

/*
 * server_channel_notice_cli.
 * ------------------------------------------------------------------
 */

std::string server_channel_notice_cli::name() const
{
    return "server-cnotice";
}

void server_channel_notice_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
    if (args.size() < 3)
        throw std::invalid_argument("server-cnotice requires 3 arguments");

    request(ctl, {
        { "command",    "server-cnotice"    },
        { "server",     args[0]             },
        { "channel",    args[1]             },
        { "message",    args[2]             }
    });
}

/*
 * server_connect_cli.
 * ------------------------------------------------------------------
 */

namespace {

option::result parse(std::vector<std::string> &args)
{
    option::options options{
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

std::string server_connect_cli::name() const
{
    return "server-connect";
}

void server_connect_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
    std::vector<std::string> copy(args);

    option::result result = parse(copy);
    option::result::const_iterator it;

    if (copy.size() < 2)
        throw std::invalid_argument("server-connect requires at least 2 arguments");

    auto object = nlohmann::json::object({
        { "name", copy[0] },
        { "host", copy[1] }
    });

    if (copy.size() == 3) {
        if (!string_util::is_int(copy[2]))
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

    request(ctl, object);
}

/*
 * server_disconnect_cli.
 * ------------------------------------------------------------------
 */

std::string server_disconnect_cli::name() const
{
    return "server-disconnect";
}

void server_disconnect_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
    auto object = nlohmann::json::object({
        { "command", "server-disconnect" }
    });

    if (args.size() > 0)
        object["server"] = args[0];

    request(ctl, object);
}

/*
 * server_info_cli.
 * ------------------------------------------------------------------
 */

std::string server_info_cli::name() const
{
    return "server-info";
}

void server_info_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
    if (args.size() < 1)
        throw std::invalid_argument("server-info requires 1 argument");

    auto json = nlohmann::json::object({
        { "command",    "server-info"   },
        { "server",     args[0]         }
    });

    request(ctl, std::move(json), [] (auto result) {
        std::cout << std::boolalpha;
        std::cout << "Name           : " << json_util::pretty(result["name"]) << std::endl;
        std::cout << "Host           : " << json_util::pretty(result["host"]) << std::endl;
        std::cout << "Port           : " << json_util::pretty(result["port"]) << std::endl;
        std::cout << "Ipv6           : " << json_util::pretty(result["ipv6"]) << std::endl;
        std::cout << "SSL            : " << json_util::pretty(result["ssl"]) << std::endl;
        std::cout << "SSL verified   : " << json_util::pretty(result["sslVerify"]) << std::endl;
        std::cout << "Channels       : ";

        for (const auto &v : result["channels"])
            if (v.is_string())
                std::cout << v.template get<std::string>() << " ";

        std::cout << std::endl;

        std::cout << "Nickname       : " << json_util::pretty(result["nickname"]) << std::endl;
        std::cout << "User name      : " << json_util::pretty(result["username"]) << std::endl;
        std::cout << "Real name      : " << json_util::pretty(result["realname"]) << std::endl;
    });
}

/*
 * server_invite_cli.
 * ------------------------------------------------------------------
 */

std::string server_invite_cli::name() const
{
    return "server-invite";
}

void server_invite_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
    if (args.size() < 3)
        throw std::invalid_argument("server-invite requires 3 arguments");

    request(ctl, {
        { "command",    "server-invite" },
        { "server",     args[0]         },
        { "target",     args[1]         },
        { "channel",    args[2]         }
    });
}

/*
 * server_join_cli.
 * ------------------------------------------------------------------
 */

std::string server_join_cli::name() const
{
    return "server-join";
}

void server_join_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
    if (args.size() < 2)
        throw std::invalid_argument("server-join requires at least 2 arguments");

    auto object = nlohmann::json::object({
        { "server",     args[0]         },
        { "channel",    args[1]         }
    });

    if (args.size() == 3)
        object["password"] = args[2];

    request(ctl, object);
}

/*
 * server_kick_cli.
 * ------------------------------------------------------------------
 */

std::string server_kick_cli::name() const
{
    return "server-kick";
}

void server_kick_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
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

    request(ctl, object);
}

/*
 * server_list_cli.
 * ------------------------------------------------------------------
 */

std::string server_list_cli::name() const
{
    return "server-list";
}

void server_list_cli::exec(ctl::controller& ctl, const std::vector<std::string>&)
{
    request(ctl, {{ "command", "server-list" }}, [] (auto result) {
        for (const auto &n : result["list"])
            if (n.is_string())
                std::cout << n.template get<std::string>() << std::endl;
    });
}

/*
 * server_me_cli.
 * ------------------------------------------------------------------
 */

std::string server_me_cli::name() const
{
    return "server-me";
}

void server_me_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
    if (args.size() < 3)
        throw std::runtime_error("server-me requires 3 arguments");

    request(ctl, {
        { "server",     args[0]     },
        { "target",     args[1]     },
        { "message",    args[2]     }
    });
}

/*
 * server_message_cli.
 * ------------------------------------------------------------------
 */

std::string server_message_cli::name() const
{
    return "server-message";
}

void server_message_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
    if (args.size() < 3)
        throw std::invalid_argument("server-message requires 3 arguments");

    request(ctl, {
        { "server",     args[0] },
        { "target",     args[1] },
        { "message",    args[2] }
    });
}

/*
 * server_mode_cli.
 * ------------------------------------------------------------------
 */

std::string server_mode_cli::name() const
{
    return "server-mode";
}

void server_mode_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
    if (args.size() < 2)
        throw std::invalid_argument("server-mode requires 2 arguments");

    request(ctl, {
        { "server", args[0] },
        { "mode",   args[1] }
    });
}

/*
 * server_nick_cli.
 * ------------------------------------------------------------------
 */

std::string server_nick_cli::name() const
{
    return "server-nick";
}

void server_nick_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
    if (args.size() < 2)
        throw std::invalid_argument("server-nick requires 2 arguments");

    request(ctl, {
        { "server",     args[0] },
        { "nickname",   args[1] }
    });
}

/*
 * server_notice_cli.
 * ------------------------------------------------------------------
 */

std::string server_notice_cli::name() const
{
    return "server-notice";
}

void server_notice_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
    if (args.size() < 3)
        throw std::invalid_argument("server-notice requires 3 arguments");

    request(ctl, {
        { "server",     args[0] },
        { "target",     args[1] },
        { "message",    args[2] }
    });
}

/*
 * server_part_cli.
 * ------------------------------------------------------------------
 */

std::string server_part_cli::name() const
{
    return "server-part";
}

void server_part_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
    if (args.size() < 2)
        throw std::invalid_argument("server-part requires at least 2 arguments");

    auto object = nlohmann::json::object({
        { "server",     args[0] },
        { "channel",    args[1] }
    });

    if (args.size() >= 3)
        object["reason"] = args[2];

    request(ctl, object);
}

/*
 * server_reconnect_cli.
 * ------------------------------------------------------------------
 */

std::string server_reconnect_cli::name() const
{
    return "server-reconnect";
}

void server_reconnect_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
    auto object = nlohmann::json::object({
        { "command", "server-reconnect" }
    });

    if (args.size() >= 1)
        object["server"] = args[0];

    request(ctl, object);
}

/*
 * server_topic_cli.
 * ------------------------------------------------------------------
 */

std::string server_topic_cli::name() const
{
    return "server-topic";
}

void server_topic_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
    if (args.size() < 3)
        throw std::invalid_argument("server-topic requires 3 arguments");

    request(ctl, {
        { "server",     args[0] },
        { "channel",    args[1] },
        { "topic",      args[2] }
    });
}

/*
 * rule_add_cli.
 * ------------------------------------------------------------------
 */

std::string rule_add_cli::name() const
{
    return "rule-add";
}

void rule_add_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
    static const option::options options{
        { "-c",             true },
        { "--add-channel",  true },
        { "-e",             true },
        { "--add-event",    true },
        { "-i",             true },
        { "--index",        true },
        { "-p",             true },
        { "--add-plugin",   true },
        { "-s",             true },
        { "--add-server",   true }
    };

    auto copy = args;
    auto result = option::read(copy, options);

    if (copy.size() < 1)
        throw std::invalid_argument("rule-add requires at least 1 argument");

    auto json = nlohmann::json::object({
        { "command",    "rule-add"              },
        { "channels",   nlohmann::json::array() },
        { "events",     nlohmann::json::array() },
        { "plugins",    nlohmann::json::array() },
        { "servers",    nlohmann::json::array() }
    });

    // All sets.
    for (const auto& pair : result) {
        if (pair.first == "-c" || pair.first == "--add-channel")
            json["channels"].push_back(pair.second);
        if (pair.first == "-e" || pair.first == "--add-event")
            json["events"].push_back(pair.second);
        if (pair.first == "-p" || pair.first == "--add-plugin")
            json["plugins"].push_back(pair.second);
        if (pair.first == "-s" || pair.first == "--add-server")
            json["servers"].push_back(pair.second);
    }

    // Index.
    if (result.count("-i") > 0)
        json["index"] = string_util::to_number<unsigned>(result.find("-i")->second);
    if (result.count("--index") > 0)
        json["index"] = string_util::to_number<unsigned>(result.find("--index")->second);

    // And action.
    if (copy[0] != "accept" && copy[0] != "drop")
        throw std::runtime_error(string_util::sprintf("invalid action '%s'", copy[0]));

    json["action"] = copy[0];

    request(ctl, json);
}

/*
 * rule_edit_cli.
 * ------------------------------------------------------------------
 */

std::string rule_edit_cli::name() const
{
    return "rule-edit";
}

void rule_edit_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
    static const option::options options{
        { "-a",                 true },
        { "--action",           true },
        { "-c",                 true },
        { "--add-channel",      true },
        { "-C",                 true },
        { "--remove-channel",   true },
        { "-e",                 true },
        { "--add-event",        true },
        { "-E",                 true },
        { "--remove-event",     true },
        { "-p",                 true },
        { "--add-plugin",       true },
        { "-P",                 true },
        { "--remove-plugin",    true },
        { "-s",                 true },
        { "--add-server",       true },
        { "-S",                 true },
        { "--remove-server",    true },
    };

    auto copy = args;
    auto result = option::read(copy, options);

    if (copy.size() < 1)
        throw std::invalid_argument("rule-edit requires at least 1 argument");

    auto json = nlohmann::json::object({
        { "command",    "rule-edit"             },
        { "channels",   nlohmann::json::array() },
        { "events",     nlohmann::json::array() },
        { "plugins",    nlohmann::json::array() },
        { "servers",    nlohmann::json::array() }
    });

    for (const auto& pair : result) {
        // Action.
        if (pair.first == "-a" || pair.first == "--action")
            json["action"] = pair.second;

        // Additions.
        if (pair.first == "-c" || pair.first == "--add-channel")
            json["add-channels"].push_back(pair.second);
        if (pair.first == "-e" || pair.first == "--add-event")
            json["add-events"].push_back(pair.second);
        if (pair.first == "-p" || pair.first == "--add-plugin")
            json["add-plugins"].push_back(pair.second);
        if (pair.first == "-s" || pair.first == "--add-server")
            json["add-servers"].push_back(pair.second);

        // Removals.
        if (pair.first == "-C" || pair.first == "--remove-channel")
            json["remove-channels"].push_back(pair.second);
        if (pair.first == "-E" || pair.first == "--remove-event")
            json["remove-events"].push_back(pair.second);
        if (pair.first == "-P" || pair.first == "--remove-plugin")
            json["remove-plugins"].push_back(pair.second);
        if (pair.first == "-S" || pair.first == "--remove-server")
            json["remove-servers"].push_back(pair.second);
    }

    // Index.
    json["index"] = string_util::to_number<unsigned>(copy[0]);

    request(ctl, json);
}

/*
 * rule_list_cli.
 * ------------------------------------------------------------------
 */

namespace {

void show_rule(const nlohmann::json& json, int index)
{
    assert(json.is_object());

    auto unjoin = [] (auto array) {
        std::ostringstream oss;

        for (auto it = array.begin(); it != array.end(); ++it) {
            if (!it->is_string())
                continue;

            oss << it->template get<std::string>() << " ";
        }

        return oss.str();
    };
    auto unstr = [] (auto action) {
        if (action.is_string() && action == "accept")
            return "accept";
        else
            return "drop";
    };

    std::cout << "rule:        " << index << std::endl;
    std::cout << "servers:     " << unjoin(json["servers"]) << std::endl;
    std::cout << "channels:    " << unjoin(json["channels"]) << std::endl;
    std::cout << "plugins:     " << unjoin(json["plugins"]) << std::endl;
    std::cout << "events:      " << unjoin(json["events"]) << std::endl;
    std::cout << "action:      " << unstr(json["action"]) << std::endl;
    std::cout << std::endl;
}

} // !namespace

std::string rule_list_cli::name() const
{
    return "rule-list";
}

void rule_list_cli::exec(ctl::controller& ctl, const std::vector<std::string>&)
{
    request(ctl, {{ "command", "rule-list" }}, [] (auto result) {
        auto pos = 0;

        for (const auto& obj : result["list"]) {
            if (obj.is_object())
                show_rule(obj, pos++);
        }
    });
}

/*
 * rule_info_cli.
 * ------------------------------------------------------------------
 */

std::string rule_info_cli::name() const
{
    return "rule-info";
}

void rule_info_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
    if (args.size() < 1)
        throw std::invalid_argument("rule-info requires 1 argument");

    int index = 0;

    try {
        index = std::stoi(args[0]);
    } catch (...) {
        throw std::invalid_argument("invalid number '" + args[0] + "'");
    }

    auto json = nlohmann::json::object({
        { "command",    "rule-info" },
        { "index",      index       }
    });

    request(ctl, std::move(json), [] (auto result) {
        show_rule(result, 0);
    });
}

/*
 * rule_remove_cli.
 * ------------------------------------------------------------------
 */

std::string rule_remove_cli::name() const
{
    return "rule-remove";
}

void rule_remove_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
    if (args.size() < 1)
        throw std::invalid_argument("rule-remove requires 1 argument");

    int index = 0;

    try {
        index = std::stoi(args[0]);
    } catch (...) {
        throw std::invalid_argument("invalid number '" + args[0] + "'");
    }

    request(ctl, {
        { "command",    "rule-remove"   },
        { "index",      index           }
    });
}

/*
 * rule_move_cli.
 * ------------------------------------------------------------------
 */

std::string rule_move_cli::name() const
{
    return "rule-move";
}

void rule_move_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
    if (args.size() < 2)
        throw std::invalid_argument("rule-move requires 2 arguments");

    int from = 0;
    int to = 0;

    try {
        from = std::stoi(args[0]);
        to = std::stoi(args[1]);
    } catch (...) {
        throw std::invalid_argument("invalid number");
    }

    request(ctl, {
        { "command",    "rule-move" },
        { "from",       from        },
        { "to",         to          }
    });
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
    std::cout << "server:      " << json_util::pretty(v, "server") << "\n";
    std::cout << "origin:      " << json_util::pretty(v, "origin") << "\n";
    std::cout << "mode:        " << json_util::pretty(v, "mode") << "\n";
    std::cout << "argument:    " << json_util::pretty(v, "argument") << "\n";
}

void onChannelNotice(const nlohmann::json &v)
{
    std::cout << "event:       onChannelNotice\n";
    std::cout << "server:      " << json_util::pretty(v, "server") << "\n";
    std::cout << "origin:      " << json_util::pretty(v, "origin") << "\n";
    std::cout << "channel:     " << json_util::pretty(v, "channel") << "\n";
    std::cout << "message:     " << json_util::pretty(v, "message") << "\n";
}

void onConnect(const nlohmann::json &v)
{
    std::cout << "event:       onConnect\n";
    std::cout << "server:      " << json_util::pretty(v, "server") << "\n";
}

void onInvite(const nlohmann::json &v)
{
    std::cout << "event:       onInvite\n";
    std::cout << "server:      " << json_util::pretty(v, "server") << "\n";
    std::cout << "origin:      " << json_util::pretty(v, "origin") << "\n";
    std::cout << "channel:     " << json_util::pretty(v, "channel") << "\n";
}

void onJoin(const nlohmann::json &v)
{
    std::cout << "event:       onJoin\n";
    std::cout << "server:      " << json_util::pretty(v, "server") << "\n";
    std::cout << "origin:      " << json_util::pretty(v, "origin") << "\n";
    std::cout << "channel:     " << json_util::pretty(v, "channel") << "\n";
}

void onKick(const nlohmann::json &v)
{
    std::cout << "event:       onKick\n";
    std::cout << "server:      " << json_util::pretty(v, "server") << "\n";
    std::cout << "origin:      " << json_util::pretty(v, "origin") << "\n";
    std::cout << "channel:     " << json_util::pretty(v, "channel") << "\n";
    std::cout << "target:      " << json_util::pretty(v, "target") << "\n";
    std::cout << "reason:      " << json_util::pretty(v, "reason") << "\n";
}

void onMessage(const nlohmann::json &v)
{
    std::cout << "event:       onMessage\n";
    std::cout << "server:      " << json_util::pretty(v, "server") << "\n";
    std::cout << "origin:      " << json_util::pretty(v, "origin") << "\n";
    std::cout << "channel:     " << json_util::pretty(v, "channel") << "\n";
    std::cout << "message:     " << json_util::pretty(v, "message") << "\n";
}

void onMe(const nlohmann::json &v)
{
    std::cout << "event:       onMe\n";
    std::cout << "server:      " << json_util::pretty(v, "server") << "\n";
    std::cout << "origin:      " << json_util::pretty(v, "origin") << "\n";
    std::cout << "target:      " << json_util::pretty(v, "target") << "\n";
    std::cout << "message:     " << json_util::pretty(v, "message") << "\n";
}

void onMode(const nlohmann::json &v)
{
    std::cout << "event:       onMode\n";
    std::cout << "server:      " << json_util::pretty(v, "server") << "\n";
    std::cout << "origin:      " << json_util::pretty(v, "origin") << "\n";
    std::cout << "mode:        " << json_util::pretty(v, "mode") << "\n";
}

void onNames(const nlohmann::json &v)
{
    std::cout << "event:       onNames\n";
    std::cout << "server:      " << json_util::pretty(v, "server") << "\n";
    std::cout << "channel:     " << json_util::pretty(v, "channel") << "\n";
    std::cout << "names:       " << json_util::pretty(v, "names") << "\n";
}

void onNick(const nlohmann::json &v)
{
    std::cout << "event:       onNick\n";
    std::cout << "server:      " << json_util::pretty(v, "server") << "\n";
    std::cout << "origin:      " << json_util::pretty(v, "origin") << "\n";
    std::cout << "nickname:    " << json_util::pretty(v, "nickname") << "\n";
}

void onNotice(const nlohmann::json &v)
{
    std::cout << "event:       onNotice\n";
    std::cout << "server:      " << json_util::pretty(v, "server") << "\n";
    std::cout << "origin:      " << json_util::pretty(v, "origin") << "\n";
    std::cout << "message:     " << json_util::pretty(v, "message") << "\n";
}

void onPart(const nlohmann::json &v)
{
    std::cout << "event:       onPart\n";
    std::cout << "server:      " << json_util::pretty(v, "server") << "\n";
    std::cout << "origin:      " << json_util::pretty(v, "origin") << "\n";
    std::cout << "channel:     " << json_util::pretty(v, "channel") << "\n";
    std::cout << "reason:      " << json_util::pretty(v, "reason") << "\n";
}

void onQuery(const nlohmann::json &v)
{
    std::cout << "event:       onQuery\n";
    std::cout << "server:      " << json_util::pretty(v, "server") << "\n";
    std::cout << "origin:      " << json_util::pretty(v, "origin") << "\n";
    std::cout << "message:     " << json_util::pretty(v, "message") << "\n";
}

void onTopic(const nlohmann::json &v)
{
    std::cout << "event:       onTopic\n";
    std::cout << "server:      " << json_util::pretty(v, "server") << "\n";
    std::cout << "origin:      " << json_util::pretty(v, "origin") << "\n";
    std::cout << "channel:     " << json_util::pretty(v, "channel") << "\n";
    std::cout << "topic:       " << json_util::pretty(v, "topic") << "\n";
}

void onWhois(const nlohmann::json &v)
{
    std::cout << "event:       onWhois\n";
    std::cout << "server:      " << json_util::pretty(v, "server") << "\n";
    std::cout << "nickname:    " << json_util::pretty(v, "nickname") << "\n";
    std::cout << "username:    " << json_util::pretty(v, "username") << "\n";
    std::cout << "host:        " << json_util::pretty(v, "host") << "\n";
    std::cout << "realname:    " << json_util::pretty(v, "realname") << "\n";
}

const std::unordered_map<std::string, std::function<void (const nlohmann::json&)>> events{
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

void get_event(ctl::controller& ctl, std::string fmt)
{
    ctl.recv([&ctl, fmt] (auto code, auto message) {
        if (code)
            throw boost::system::system_error(code);

        auto it = events.find(json_util::to_string(message["event"]));

        if (it != events.end()) {
            if (fmt == "json")
                std::cout << message.dump(4) << std::endl;
            else {
                it->second(message);
                std::cout << std::endl;
            }
        }

        get_event(ctl, std::move(fmt));
    });
}

} // !namespace

std::string watch_cli::name() const
{
    return "watch";
}

void watch_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
    auto fmt = format(args);

    if (fmt != "native" && fmt != "json")
        throw std::invalid_argument("invalid format given: " + fmt);

    get_event(ctl, fmt);
}

} // !cli

} // !irccd
