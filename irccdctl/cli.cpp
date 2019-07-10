/*
 * cli.cpp -- command line for irccdctl
 *
 * Copyright (c) 2013-2019 David Demelier <markand@malikania.fr>
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

#include <iomanip>
#include <ios>
#include <iostream>

#include <irccd/json_util.hpp>
#include <irccd/options.hpp>
#include <irccd/string_util.hpp>

#include <irccd/ctl/controller.hpp>

#include <irccd/daemon/rule_service.hpp>

#include "cli.hpp"

using irccd::json_util::deserializer;
using irccd::json_util::pretty;

using irccd::daemon::rule_error;

namespace irccd::ctl {

// {{{ helpers

namespace {

template <typename T>
auto operator<<(std::ostream& out, const std::optional<T>& value) -> std::ostream&
{
	if (value)
		out << pretty(*value);

	return out;
}

template <typename T>
auto bind() noexcept -> cli::constructor
{
	return [] () noexcept {
		return std::make_unique<T>();
	};
}

auto format(std::vector<std::string> args) -> std::string
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

auto align(std::string_view topic) -> std::ostream&
{
	assert(topic.size() <= 16);

	return std::cout << std::setw(16) << std::left << topic;
}

void onConnect(const deserializer& v)
{
	align("event:")  << "onConnect\n";
	align("server:") << v.get<std::string>("server") << "\n";
}

void onInvite(const deserializer& v)
{
	align("event:")   << "onInvite\n";
	align("server:")  << v.get<std::string>("server") << "\n";
	align("origin:")  << v.get<std::string>("origin") << "\n";
	align("channel:") << v.get<std::string>("channel") << "\n";
}

void onJoin(const deserializer& v)
{
	align("event:")   << "onJoin\n";
	align("server:")  << v.get<std::string>("server") << "\n";
	align("origin:")  << v.get<std::string>("origin") << "\n";
	align("channel:") << v.get<std::string>("channel") << "\n";
}

void onKick(const deserializer& v)
{
	align("event:")   << "onKick\n";
	align("server:")  << v.get<std::string>("server") << "\n";
	align("origin:")  << v.get<std::string>("origin") << "\n";
	align("channel:") << v.get<std::string>("channel") << "\n";
	align("target:")  << v.get<std::string>("target") << "\n";
	align("reason:")  << v.get<std::string>("reason") << "\n";
}

void onMessage(const deserializer& v)
{
	align("event:")   << "onMessage\n";
	align("server:")  << v.get<std::string>("server") << "\n";
	align("origin:")  << v.get<std::string>("origin") << "\n";
	align("channel:") << v.get<std::string>("channel") << "\n";
	align("message:") << v.get<std::string>("message") << "\n";
}

void onMe(const deserializer& v)
{
	align("event:")   << "onMe\n";
	align("server:")  << v.get<std::string>("server") << "\n";
	align("origin:")  << v.get<std::string>("origin") << "\n";
	align("target:")  << v.get<std::string>("target") << "\n";
	align("message:") << v.get<std::string>("message") << "\n";
}

void onMode(const deserializer& v)
{
	align("event:")  << "onMode\n";
	align("server:") << v.get<std::string>("server") << "\n";
	align("origin:") << v.get<std::string>("origin") << "\n";
	align("mode:")   << v.get<std::string>("mode") << "\n";
}

void onNames(const deserializer& v)
{
	align("event:")   << "onNames\n";
	align("server:")  << v.get<std::string>("server") << "\n";
	align("channel:") << v.get<std::string>("channel") << "\n";
	align("names:")   << v.get<std::string>("names") << "\n";
}

void onNick(const deserializer& v)
{
	align("event:")    << "onNick\n";
	align("server:")   << v.get<std::string>("server") << "\n";
	align("origin:")   << v.get<std::string>("origin") << "\n";
	align("nickname:") << v.get<std::string>("nickname") << "\n";
}

void onNotice(const deserializer& v)
{
	align("event:")   << "onNotice\n";
	align("server:")  << v.get<std::string>("server") << "\n";
	align("origin:")  << v.get<std::string>("origin") << "\n";
	align("message:") << v.get<std::string>("message") << "\n";
}

void onPart(const deserializer& v)
{
	align("event:")   << "onPart\n";
	align("server:")  << v.get<std::string>("server") << "\n";
	align("origin:")  << v.get<std::string>("origin") << "\n";
	align("channel:") << v.get<std::string>("channel") << "\n";
	align("reason:")  << v.get<std::string>("reason") << "\n";
}

void onTopic(const deserializer& v)
{
	align("event:")   << "onTopic\n";
	align("server:")  << v.get<std::string>("server") << "\n";
	align("origin:")  << v.get<std::string>("origin") << "\n";
	align("channel:") << v.get<std::string>("channel") << "\n";
	align("topic:")   << v.get<std::string>("topic") << "\n";
}

void onWhois(const deserializer& v)
{
	align("event:")    << "onWhois\n";
	align("server:")   << v.get<std::string>("server") << "\n";
	align("nickname:") << v.get<std::string>("nickname") << "\n";
	align("username:") << v.get<std::string>("username") << "\n";
	align("hostname:") << v.get<std::string>("hostname") << "\n";
	align("realname:") << v.get<std::string>("realname") << "\n";
}

const std::unordered_map<std::string_view, std::function<void (const nlohmann::json&)>> events{
	{ "onConnect",  onConnect       },
	{ "onInvite",   onInvite        },
	{ "onJoin",     onJoin          },
	{ "onKick",     onKick          },
	{ "onMessage",  onMessage       },
	{ "onMe",       onMe            },
	{ "onMode",     onMode          },
	{ "onNames",    onNames         },
	{ "onNick",     onNick          },
	{ "onNotice",   onNotice        },
	{ "onPart",     onPart          },
	{ "onTopic",    onTopic         },
	{ "onWhois",    onWhois         }
};

void get_event(ctl::controller& ctl, std::string fmt)
{
	ctl.recv([&ctl, fmt] (auto code, auto message) {
		const deserializer doc(message);

		if (code)
			throw std::system_error(code);

		const auto event = doc.get<std::string>("event");
		const auto it = events.find(event ? *event : "");

		if (it != events.end()) {
			if (fmt == "json")
				std::cout << message.dump(4) << std::endl;
			else {
				it->second(doc);
				std::cout << std::endl;
			}
		}

		get_event(ctl, std::move(fmt));
	});
}

auto parse(std::vector<std::string> &args) -> option::result
{
	option::options options{
		{ "-4",                 false   },
                { "-6",                 false   },
		{ "-c",                 true    },
		{ "--command",          true    },
		{ "-n",                 true    },
		{ "--nickname",         true    },
		{ "-r",                 true    },
		{ "--realname",         true    },
		{ "-s",                 false   },
		{ "--ssl",              false   },
		{ "-u",                 true    },
		{ "--username",         true    }
	};

	return option::read(args, options);
}

} // !namespace

// }}}

// {{{ cli

const std::vector<cli::constructor> cli::registry{
	bind<plugin_config_cli>(),
	bind<plugin_info_cli>(),
	bind<plugin_list_cli>(),
	bind<plugin_load_cli>(),
	bind<plugin_reload_cli>(),
	bind<plugin_unload_cli>(),
	bind<rule_add_cli>(),
	bind<rule_edit_cli>(),
	bind<rule_info_cli>(),
	bind<rule_list_cli>(),
	bind<rule_move_cli>(),
	bind<rule_remove_cli>(),
	bind<server_connect_cli>(),
	bind<server_disconnect_cli>(),
	bind<server_info_cli>(),
	bind<server_invite_cli>(),
	bind<server_join_cli>(),
	bind<server_kick_cli>(),
	bind<server_list_cli>(),
	bind<server_me_cli>(),
	bind<server_message_cli>(),
	bind<server_mode_cli>(),
	bind<server_nick_cli>(),
	bind<server_notice_cli>(),
	bind<server_part_cli>(),
	bind<server_reconnect_cli>(),
	bind<server_topic_cli>(),
	bind<watch_cli>()
};

void cli::recv_response(ctl::controller& ctl, nlohmann::json req, handler_t handler)
{
	ctl.recv([&ctl, req, handler, this] (auto code, auto message) {
		if (code)
			throw std::system_error(code);

		const auto c = deserializer(message).get<std::string>("command");

		if (!c)
			recv_response(ctl, std::move(req), std::move(handler));
		else if (handler)
			handler(std::move(message));
	});
}

void cli::request(ctl::controller& ctl, nlohmann::json req, handler_t handler)
{
	ctl.send(req, [&ctl, req, handler, this] (auto code) {
		if (code)
			throw std::system_error(code);

		recv_response(ctl, std::move(req), std::move(handler));
	});
}

// }}}

// {{{ plugin_config_cli

void plugin_config_cli::set(ctl::controller& ctl, const std::vector<std::string>&args)
{
	request(ctl, {
		{ "command",    "plugin-config" },
		{ "plugin",     args[0]         },
		{ "variable",   args[1]         },
		{ "value",      args[2]         }
	});
}

void plugin_config_cli::get(ctl::controller& ctl, const std::vector<std::string>& args)
{
	auto json = nlohmann::json::object({
		{ "command",    "plugin-config" },
		{ "plugin",     args[0]         },
		{ "variable",   args[1]         }
	});

	request(ctl, std::move(json), [args] (auto result) {
		if (result["variables"].is_object())
			std::cout << pretty(result["variables"][args[1]]) << std::endl;
	});
}

void plugin_config_cli::getall(ctl::controller& ctl, const std::vector<std::string> &args)
{
	const auto json = nlohmann::json::object({
		{ "command",    "plugin-config" },
		{ "plugin",     args[0]         }
	});

	request(ctl, json, [] (auto result) {
		const auto variables = result["variables"];

		for (auto v = variables.begin(); v != variables.end(); ++v)
			std::cout << std::setw(16) << std::left << v.key() << " : " << pretty(v.value()) << std::endl;
	});
}

auto plugin_config_cli::get_name() const noexcept -> std::string_view
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

// }}}

// {{{ plugin_info_cli

auto plugin_info_cli::get_name() const noexcept -> std::string_view
{
	return "plugin-info";
}

void plugin_info_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
	if (args.size() < 1)
		throw std::invalid_argument("plugin-info requires 1 argument");

	const auto json = nlohmann::json::object({
		{ "command",    "plugin-info"   },
		{ "plugin",     args[0]         }
	});

	request(ctl, json, [] (auto result) {
		const deserializer doc(result);

		align("author:")  << doc.get<std::string>("author") << std::endl;
		align("license:") << doc.get<std::string>("license") << std::endl;
		align("summary:") << doc.get<std::string>("summary") << std::endl;
		align("version:") << doc.get<std::string>("version") << std::endl;
	});
}

// }}}

// {{{ plugin_list_cli

auto plugin_list_cli::get_name() const noexcept -> std::string_view
{
	return "plugin-list";
}

void plugin_list_cli::exec(ctl::controller& ctl, const std::vector<std::string>&)
{
	request(ctl, {{ "command", "plugin-list" }}, [] (auto result) {
		for (const auto& value : result["list"])
			if (value.is_string())
				std::cout << value.template get<std::string>() << std::endl;
	});
}

// }}}

// {{{ plugin_load_cli

auto plugin_load_cli::get_name() const noexcept -> std::string_view
{
	return "plugin-load";
}

void plugin_load_cli::exec(ctl::controller& ctl, const std::vector<std::string> &args)
{
	if (args.size() < 1)
		throw std::invalid_argument("plugin-load requires 1 argument");

	request(ctl, {
		{ "command",    "plugin-load"   },
		{ "plugin",     args[0]         }
	});
}

// }}}

// {{{ plugin_reload_cli

auto plugin_reload_cli::get_name() const noexcept -> std::string_view
{
	return "plugin-reload";
}

void plugin_reload_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
	if (args.size() < 1)
		throw std::invalid_argument("plugin-reload requires 1 argument");

	request(ctl, {
		{ "command",    "plugin-reload" },
		{ "plugin",     args[0]         }
	});
}

// }}}

// {{{ plugin_unload_cli

auto plugin_unload_cli::get_name() const noexcept -> std::string_view
{
	return "plugin-unload";
}

void plugin_unload_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
	if (args.size() < 1)
		throw std::invalid_argument("plugin-unload requires 1 argument");

	request(ctl, {
		{ "command",    "plugin-unload" },
		{ "plugin",     args[0]         }
	});
}

// }}}

// {{{ rule_add_cli

auto rule_add_cli::get_name() const noexcept -> std::string_view
{
	return "rule-add";
}

void rule_add_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
	static const option::options options{
		{ "-c",                 true },
		{ "--add-channel",      true },
		{ "-e",                 true },
		{ "--add-event",        true },
		{ "-i",                 true },
		{ "--index",            true },
		{ "-p",                 true },
		{ "--add-plugin",       true },
		{ "-s",                 true },
		{ "--add-server",       true }
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
	std::optional<std::size_t> index;

	if (result.count("-i") > 0 && !(index = string_util::to_uint(result.find("-i")->second)))
		throw std::invalid_argument("invalid index argument");
	if (result.count("--index") > 0 && !(index = string_util::to_uint(result.find("--index")->second)))
		throw std::invalid_argument("invalid index argument");

	if (index)
		json["index"] = *index;

	json["action"] = copy[0];

	request(ctl, json);
}

// }}}

// {{{ rule_edit_cli

auto rule_edit_cli::get_name() const noexcept -> std::string_view
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
	const auto index = string_util::to_uint(copy[0]);

	if (!index)
		throw rule_error(rule_error::invalid_index);

	json["index"] = *index;

	request(ctl, json);
}

// }}}

// {{{ rule_info_cli

void rule_info_cli::print(const nlohmann::json& json, int index)
{
	assert(json.is_object());

	const auto unjoin = [] (auto array) {
		std::ostringstream oss;

		for (auto it = array.begin(); it != array.end(); ++it) {
			if (!it->is_string())
				continue;

			oss << it->template get<std::string>() << " ";
		}

		return oss.str();
	};
	const auto unstr = [] (auto action) {
		if (action.is_string() && action == "accept")
			return "accept";
		else
			return "drop";
	};

	align("rule:") << index << std::endl;
	align("servers:") << unjoin(json["servers"]) << std::endl;
	align("channels:") << unjoin(json["channels"]) << std::endl;
	align("plugins:") << unjoin(json["plugins"]) << std::endl;
	align("events:") << unjoin(json["events"]) << std::endl;
	align("action:") << unstr(json["action"]) << std::endl;
}

auto rule_info_cli::get_name() const noexcept -> std::string_view
{
	return "rule-info";
}

void rule_info_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
	if (args.size() < 1)
		throw std::invalid_argument("rule-info requires 1 argument");

	const auto index = string_util::to_int(args[0]);

	if (!index)
		throw rule_error(rule_error::invalid_index);

	const auto json = nlohmann::json::object({
		{ "command",    "rule-info"     },
		{ "index",      *index          }
	});

	request(ctl, json, [index] (auto result) {
		print(result, *index);
	});
}

// }}}

// {{{ rule_list_cli

auto rule_list_cli::get_name() const noexcept -> std::string_view
{
	return "rule-list";
}

void rule_list_cli::exec(ctl::controller& ctl, const std::vector<std::string>&)
{
	request(ctl, {{ "command", "rule-list" }}, [] (auto result) {
		auto pos = 0U;
		auto length = result["list"].size();

		for (const auto& obj : result["list"]) {
			if (!obj.is_object())
				continue;

			rule_info_cli::print(obj, pos++);

			if (pos < length)
				std::cout << std::endl;
		}
	});
}

// }}}

// {{{ rule_move_cli

auto rule_move_cli::get_name() const noexcept -> std::string_view
{
	return "rule-move";
}

void rule_move_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
	if (args.size() < 2)
		throw std::invalid_argument("rule-move requires 2 arguments");

	const auto from = string_util::to_int<int>(args[0]);
	const auto to = string_util::to_int<int>(args[1]);

	if (!from)
		throw rule_error(rule_error::invalid_index);
	if (!to)
		throw rule_error(rule_error::invalid_index);

	request(ctl, {
		{ "command",    "rule-move"     },
		{ "from",       *from           },
		{ "to",         *to             }
	});
}

// }}}

// {{{ rule_remove_cli

auto rule_remove_cli::get_name() const noexcept -> std::string_view
{
	return "rule-remove";
}

void rule_remove_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
	if (args.size() < 1)
		throw std::invalid_argument("rule-remove requires 1 argument");

	const auto index = string_util::to_int(args[0]);

	if (!index)
		throw rule_error(rule_error::invalid_index);

	request(ctl, {
		{ "command",    "rule-remove"   },
		{ "index",      *index          }
	});
}

// }}}

// {{{ server_connect_cli

auto server_connect_cli::get_name() const noexcept -> std::string_view
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
		{ "command",    "server-connect"        },
		{ "name",       copy[0]                 },
		{ "hostname",   copy[1]                 },
	});

	if (copy.size() == 3) {
		const auto port = string_util::to_int(copy[2]);

		if (!port)
			throw std::invalid_argument("invalid port given");

		object["port"] = *port;
	}

	if (result.count("-s") > 0 || result.count("--ssl") > 0)
		object["ssl"] = true;
	if ((it = result.find("-n")) != result.end() || (it = result.find("--nickname")) != result.end())
		object["nickname"] = it->second;
	if ((it = result.find("-r")) != result.end() || (it = result.find("--realname")) != result.end())
		object["realname"] = it->second;
	if ((it = result.find("-u")) != result.end() || (it = result.find("--username")) != result.end())
		object["username"] = it->second;

        // Mutually exclusive.
        if ((it = result.find("-4")) != result.end())
                object["family"] = nlohmann::json::array({ "ipv4" });
        if ((it = result.find("-6")) != result.end())
                object["family"] = nlohmann::json::array({ "ipv6" });

	request(ctl, object);
}

// }}}

// {{{ server_disconnect_cli

auto server_disconnect_cli::get_name() const noexcept -> std::string_view
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

// }}}

// {{{ server_info_cli

auto server_info_cli::get_name() const noexcept -> std::string_view
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
		const deserializer doc(result);

		align("name:")     << doc.get<std::string>("name") << std::endl;
		align("hostname:") << doc.get<std::string>("hostname") << std::endl;
		align("port:")     << doc.get<std::uint64_t>("port") << std::endl;
		align("nickname:") << doc.get<std::string>("nickname") << std::endl;
		align("username:") << doc.get<std::string>("username") << std::endl;
		align("realname:") << doc.get<std::string>("realname") << std::endl;
		align("ipv4:")     << doc.get<bool>("ipv4") << std::endl;
		align("ipv6:")     << doc.get<bool>("ipv6") << std::endl;
		align("ssl:")      << doc.get<bool>("ssl") << std::endl;
		align("channels:");

		for (const auto& v : result["channels"])
			if (v.is_string())
				std::cout << v.template get<std::string>() << " ";

		std::cout << std::endl;
	});
}

// }}}

// {{{ server_invite_cli

auto server_invite_cli::get_name() const noexcept -> std::string_view
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

// }}}

// {{{ server_join_cli

auto server_join_cli::get_name() const noexcept -> std::string_view
{
	return "server-join";
}

void server_join_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
	if (args.size() < 2)
		throw std::invalid_argument("server-join requires at least 2 arguments");

	auto object = nlohmann::json::object({
		{ "command",    "server-join"   },
		{ "server",     args[0]         },
		{ "channel",    args[1]         }
	});

	if (args.size() == 3)
		object["password"] = args[2];

	request(ctl, object);
}

// }}}

// {{{ server_kick_cli

auto server_kick_cli::get_name() const noexcept -> std::string_view
{
	return "server-kick";
}

void server_kick_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
	if (args.size() < 3)
		throw std::invalid_argument("server-kick requires at least 3 arguments ");

	auto object = nlohmann::json::object({
		{ "command",    "server-kick"   },
		{ "server",     args[0]         },
		{ "target",     args[1]         },
		{ "channel",    args[2]         }
	});

	if (args.size() == 4)
		object["reason"] = args[3];

	request(ctl, object);
}

// }}}

// {{{ server_list_cli

auto server_list_cli::get_name() const noexcept -> std::string_view
{
	return "server-list";
}

void server_list_cli::exec(ctl::controller& ctl, const std::vector<std::string>&)
{
	request(ctl, {{ "command", "server-list" }}, [] (auto result) {
		for (const auto& n : result["list"])
			if (n.is_string())
				std::cout << n.template get<std::string>() << std::endl;
	});
}

// }}}

// {{{ server_me_cli

auto server_me_cli::get_name() const noexcept -> std::string_view
{
	return "server-me";
}

void server_me_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
	if (args.size() < 3)
		throw std::runtime_error("server-me requires 3 arguments");

	request(ctl, {
		{ "command",    "server-me"     },
		{ "server",     args[0]         },
		{ "target",     args[1]         },
		{ "message",    args[2]         }
	});
}

// }}}

// {{{ server_message_cli

auto server_message_cli::get_name() const noexcept -> std::string_view
{
	return "server-message";
}

void server_message_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
	if (args.size() < 3)
		throw std::invalid_argument("server-message requires 3 arguments");

	request(ctl, {
		{ "command",    "server-message"        },
		{ "server",     args[0]                 },
		{ "target",     args[1]                 },
		{ "message",    args[2]                 }
	});
}

// }}}

// {{{ server_mode_cli

auto server_mode_cli::get_name() const noexcept -> std::string_view
{
	return "server-mode";
}

void server_mode_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
	if (args.size() < 2)
		throw std::invalid_argument("server-mode requires at least 3 arguments");

	auto json = nlohmann::json({
		{ "command",    "server-mode"   },
		{ "server",     args[0]         },
		{ "channel",    args[1]         },
		{ "mode",       args[2]         }
	});

	if (args.size() >= 4)
		json["limit"] = args[3];
	if (args.size() >= 5)
		json["user"] = args[4];
	if (args.size() >= 6)
		json["mask"] = args[5];

	request(ctl, std::move(json));
}

// }}}

// {{{ server_nick_cli

auto server_nick_cli::get_name() const noexcept -> std::string_view
{
	return "server-nick";
}

void server_nick_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
	if (args.size() < 2)
		throw std::invalid_argument("server-nick requires 2 arguments");

	request(ctl, {
		{ "command",    "server-nick"   },
		{ "server",     args[0]         },
		{ "nickname",   args[1]         }
	});
}

// }}}

// {{{ server_notice_cli

auto server_notice_cli::get_name() const noexcept -> std::string_view
{
	return "server-notice";
}

void server_notice_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
	if (args.size() < 3)
		throw std::invalid_argument("server-notice requires 3 arguments");

	request(ctl, {
		{ "command",    "server-notice" },
		{ "server",     args[0]         },
		{ "target",     args[1]         },
		{ "message",    args[2]         }
	});
}

// }}}

// {{{ server_part_cli

auto server_part_cli::get_name() const noexcept -> std::string_view
{
	return "server-part";
}

void server_part_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
	if (args.size() < 2)
		throw std::invalid_argument("server-part requires at least 2 arguments");

	auto object = nlohmann::json::object({
		{ "command",    "server-part"   },
		{ "server",     args[0]         },
		{ "channel",    args[1]         }
	});

	if (args.size() >= 3)
		object["reason"] = args[2];

	request(ctl, object);
}

// }}}

// {{{ server_reconnect_cli

auto server_reconnect_cli::get_name() const noexcept -> std::string_view
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

// }}}

// {{{ server_topic_cli

auto server_topic_cli::get_name() const noexcept -> std::string_view
{
	return "server-topic";
}

void server_topic_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
	if (args.size() < 3)
		throw std::invalid_argument("server-topic requires 3 arguments");

	request(ctl, {
		{ "command",    "server-topic"  },
		{ "server",     args[0]         },
		{ "channel",    args[1]         },
		{ "topic",      args[2]         }
	});
}

// }}}

// {{{ watch_cli

auto watch_cli::get_name() const noexcept -> std::string_view
{
	return "watch";
}

void watch_cli::exec(ctl::controller& ctl, const std::vector<std::string>& args)
{
	const auto fmt = format(args);

	if (fmt != "native" && fmt != "json")
		throw std::invalid_argument("invalid format given: " + fmt);

	get_event(ctl, fmt);
}

// }}}

} // !irccd::ctl
