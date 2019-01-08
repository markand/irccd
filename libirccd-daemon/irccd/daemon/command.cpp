/*
 * command.cpp -- remote command
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

#include <irccd/sysconfig.hpp>

#include <irccd/string_util.hpp>

#include "bot.hpp"
#include "command.hpp"
#include "plugin.hpp"
#include "plugin_service.hpp"
#include "rule.hpp"
#include "rule_service.hpp"
#include "rule_util.hpp"
#include "server.hpp"
#include "server_service.hpp"
#include "server_util.hpp"
#include "transport_client.hpp"

using namespace std::string_literals;

namespace irccd::daemon {

namespace {

void exec_set(transport_client& client, plugin& plugin, const nlohmann::json& args)
{
	assert(args.count("value") > 0);

	const auto var = args.find("variable");
	const auto value = args.find("value");

	if (var == args.end() || !var->is_string())
		throw bot_error(bot_error::error::incomplete_message);
	if (value == args.end() || !value->is_string())
		throw bot_error(bot_error::error::incomplete_message);

	auto config = plugin.get_options();

	config[var->get<std::string>()] = value->get<std::string>();
	plugin.set_options(config);
	client.success("plugin-config");
}

void exec_get(transport_client& client, plugin& plugin, const nlohmann::json& args)
{
	auto variables = nlohmann::json::object();
	auto var = args.find("variable");

	if (var != args.end() && var->is_string())
		variables[var->get<std::string>()] = plugin.get_options()[*var];
	else
		for (const auto& pair : plugin.get_options())
			variables[pair.first] = pair.second;

	/*
	 * Don't put all variables into the response, put them into a sub
	 * property 'variables' instead.
	 *
	 * It's easier for the client to iterate over all.
	 */
	client.write({
		{ "command",    "plugin-config" },
		{ "variables",  variables       }
	});
}

template <typename T>
auto bind() noexcept -> command::constructor
{
	return [] () noexcept {
		return std::make_unique<T>();
	};
}

} // !namespace

auto command::registry() noexcept -> const std::vector<constructor>&
{
	static const std::vector<command::constructor> list{
		bind<plugin_config_command>(),
		bind<plugin_info_command>(),
		bind<plugin_list_command>(),
		bind<plugin_load_command>(),
		bind<plugin_reload_command>(),
		bind<plugin_unload_command>(),
		bind<rule_add_command>(),
		bind<rule_edit_command>(),
		bind<rule_info_command>(),
		bind<rule_info_command>(),
		bind<rule_list_command>(),
		bind<rule_move_command>(),
		bind<rule_remove_command>(),
		bind<server_connect_command>(),
		bind<server_disconnect_command>(),
		bind<server_info_command>(),
		bind<server_invite_command>(),
		bind<server_join_command>(),
		bind<server_kick_command>(),
		bind<server_list_command>(),
		bind<server_me_command>(),
		bind<server_message_command>(),
		bind<server_mode_command>(),
		bind<server_nick_command>(),
		bind<server_notice_command>(),
		bind<server_part_command>(),
		bind<server_reconnect_command>(),
		bind<server_topic_command>()
	};

	return list;
};

// {{{ plugin_config_command

auto plugin_config_command::get_name() const noexcept -> std::string_view
{
	return "plugin-config";
}

void plugin_config_command::exec(bot& bot, transport_client& client, const document& args)
{
	const auto id = args.get<std::string>("plugin");

	if (!id || !string_util::is_identifier(*id))
		throw plugin_error(plugin_error::invalid_identifier);

	const auto plugin = bot.plugins().require(*id);

	if (args.count("value") > 0)
		exec_set(client, *plugin, args);
	else
		exec_get(client, *plugin, args);
}

// }}}

// {{{ plugin_info_command

auto plugin_info_command::get_name() const noexcept -> std::string_view
{
	return "plugin-info";
}

void plugin_info_command::exec(bot& bot, transport_client& client, const document& args)
{
	const auto id = args.get<std::string>("plugin");

	if (!id || !string_util::is_identifier(*id))
		throw plugin_error(plugin_error::invalid_identifier);

	const auto plugin = bot.plugins().require(*id);

	client.write({
		{ "command",    "plugin-info"                           },
		{ "author",     std::string(plugin->get_author())       },
		{ "license",    std::string(plugin->get_license())      },
		{ "summary",    std::string(plugin->get_summary())      },
		{ "version",    std::string(plugin->get_version())      }
	});
}

// }}}

// {{{ plugin_list_command

auto plugin_list_command::get_name() const noexcept -> std::string_view
{
	return "plugin-list";
}

void plugin_list_command::exec(bot& bot, transport_client& client, const document&)
{
	auto list = nlohmann::json::array();

	for (const auto& plg : bot.plugins().list())
		list += plg->get_id();

	client.write({
		{ "command",    "plugin-list"   },
		{ "list",       list            }
	});
}

// }}}

// {{{ plugin_load_command

auto plugin_load_command::get_name() const noexcept -> std::string_view
{
	return "plugin-load";
}

void plugin_load_command::exec(bot& bot, transport_client& client, const document& args)
{
	const auto id = args.get<std::string>("plugin");

	if (!id || !string_util::is_identifier(*id))
		throw plugin_error(plugin_error::invalid_identifier);

	bot.plugins().load(*id, "");
	client.success("plugin-load");
}

// }}}

// {{{ plugin_reload_command

auto plugin_reload_command::get_name() const noexcept -> std::string_view
{
	return "plugin-reload";
}

void plugin_reload_command::exec(bot& bot, transport_client& client, const document& args)
{
	const auto id = args.get<std::string>("plugin");

	if (!id || !string_util::is_identifier(*id))
		throw plugin_error(plugin_error::invalid_identifier);

	bot.plugins().reload(*id);
	client.success("plugin-reload");
}

// }}}

// {{{ plugin_unload_command

auto plugin_unload_command::get_name() const noexcept -> std::string_view
{
	return "plugin-unload";
}

void plugin_unload_command::exec(bot& bot, transport_client& client, const document& args)
{
	const auto id = args.get<std::string>("plugin");

	if (!id || !string_util::is_identifier(*id))
		throw plugin_error(plugin_error::invalid_identifier);

	bot.plugins().unload(*id);
	client.success("plugin-unload");
}

// }}}

// {{{ rule_add_command

auto rule_add_command::get_name() const noexcept -> std::string_view
{
	return "rule-add";
}

void rule_add_command::exec(bot& bot, transport_client& client, const document& args)
{
	const auto index = args.optional<unsigned>("index", bot.rules().list().size());

	if (!index || *index > bot.rules().list().size())
		throw rule_error(rule_error::error::invalid_index);

	bot.rules().insert(rule_util::from_json(args), *index);
	client.success("rule-add");
}

// }}}

// {{{ rule_edit_command

auto rule_edit_command::get_name() const noexcept -> std::string_view
{
	return "rule-edit";
}

void rule_edit_command::exec(bot& bot, transport_client& client, const document& args)
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

	const auto index = args.get<unsigned>("index");

	if (!index)
		throw rule_error(rule_error::invalid_index);

	// Create a copy to avoid incomplete edition in case of errors.
	auto rule = bot.rules().require(*index);

	updateset(rule.channels, args, "channels");
	updateset(rule.events, args, "events");
	updateset(rule.plugins, args, "plugins");
	updateset(rule.servers, args, "servers");

	auto action = args.find("action");

	if (action != args.end()) {
		if (!action->is_string())
			throw rule_error(rule_error::error::invalid_action);

		if (action->get<std::string>() == "accept")
			rule.action = rule::action_type::accept;
		else if (action->get<std::string>() == "drop")
			rule.action = rule::action_type::drop;
		else
			throw rule_error(rule_error::invalid_action);
	}

	// All done, sync the rule.
	bot.rules().require(*index) = rule;
	client.success("rule-edit");
}

// }}}

// {{{ rule_info_command

auto rule_info_command::get_name() const noexcept -> std::string_view
{
	return "rule-info";
}

void rule_info_command::exec(bot& bot, transport_client& client, const document& args)
{
	const auto index = args.get<unsigned>("index");

	if (!index)
		throw rule_error(rule_error::invalid_index);

	auto json = rule_util::to_json(bot.rules().require(*index));

	json.push_back({"command", "rule-info"});
	client.write(std::move(json));
}

// }}}

// {{{ rule_list_command

auto rule_list_command::get_name() const noexcept -> std::string_view
{
	return "rule-list";
}

void rule_list_command::exec(bot& bot, transport_client& client, const document&)
{
	auto array = nlohmann::json::array();

	for (const auto& rule : bot.rules().list())
		array.push_back(rule_util::to_json(rule));

	client.write({
		{ "command",    "rule-list"             },
		{ "list",       std::move(array)        }
	});
}

// }}}

// {{{ rule_move_command

auto rule_move_command::get_name() const noexcept -> std::string_view
{
	return "rule-move";
}

void rule_move_command::exec(bot& bot, transport_client& client, const document& args)
{
	const auto from = args.get<unsigned>("from");
	const auto to = args.get<unsigned>("to");

	if (!from || !to)
		throw rule_error(rule_error::invalid_index);

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
	if (*from == *to) {
		client.success("rule-move");
		return;
	}

	if (*from >= bot.rules().list().size())
		throw rule_error(rule_error::error::invalid_index);

	const auto save = bot.rules().list()[*from];

	bot.rules().remove(*from);
	bot.rules().insert(save, *to > bot.rules().list().size() ? bot.rules().list().size() : *to);
	client.success("rule-move");
}

// }}}

// {{{ rule_remove_command

auto rule_remove_command::get_name() const noexcept -> std::string_view
{
	return "rule-remove";
}

void rule_remove_command::exec(bot& bot, transport_client& client, const document& args)
{
	const auto index = args.get<unsigned>("index");

	if (!index || *index >= bot.rules().list().size())
		throw rule_error(rule_error::invalid_index);

	bot.rules().remove(*index);
	client.success("rule-remove");
}

// }}}

// {{{ server_connect_command

auto server_connect_command::get_name() const noexcept -> std::string_view
{
	return "server-connect";
}

void server_connect_command::exec(bot& bot, transport_client& client, const document& args)
{
	auto server = server_util::from_json(bot.get_service(), args);

	if (bot.servers().has(server->get_id()))
		throw server_error(server_error::already_exists);

	bot.servers().add(std::move(server));
	client.success("server-connect");
}

// }}}

// {{{ server_disconnect_command

auto server_disconnect_command::get_name() const noexcept -> std::string_view
{
	return "server-disconnect";
}

void server_disconnect_command::exec(bot& bot, transport_client& client, const document& args)
{
	const auto it = args.find("server");

	if (it == args.end())
		bot.servers().clear();
	else {
		if (!it->is_string() || !string_util::is_identifier(it->get<std::string>()))
			throw server_error(server_error::invalid_identifier);

		const auto name = it->get<std::string>();

		bot.servers().require(name);
		bot.servers().remove(name);
	}

	client.success("server-disconnect");
}

// }}}

// {{{ server_info_command

auto server_info_command::get_name() const noexcept -> std::string_view
{
	return "server-info";
}

void server_info_command::exec(bot& bot, transport_client& client, const document& args)
{
	const auto id = args.get<std::string>("server");

	if (!id || !string_util::is_identifier(*id))
		throw server_error(server_error::invalid_identifier);

	const auto server = bot.servers().require(*id);

	// Construct the JSON response.
	auto response = document::object();

	// General stuff.
	response.push_back({"command", "server-info"});
	response.push_back({"name", server->get_id()});
	response.push_back({"hostname", server->get_hostname()});
	response.push_back({"port", server->get_port()});
	response.push_back({"nickname", server->get_nickname()});
	response.push_back({"username", server->get_username()});
	response.push_back({"realname", server->get_realname()});
	response.push_back({"channels", server->get_channels()});

	// Optional stuff.
	response.push_back({"ipv4", static_cast<bool>(server->get_options() & server::options::ipv4)});
	response.push_back({"ipv6", static_cast<bool>(server->get_options() & server::options::ipv6)});
	response.push_back({"ssl", static_cast<bool>(server->get_options() & server::options::ssl)});

	client.write(response);
}

// }}}

// {{{ server_invite_command

auto server_invite_command::get_name() const noexcept -> std::string_view
{
	return "server-invite";
}

void server_invite_command::exec(bot& bot, transport_client& client, const document& args)
{
	const auto id = args.get<std::string>("server");
	const auto target = args.get<std::string>("target");
	const auto channel = args.get<std::string>("channel");

	if (!id || !string_util::is_identifier(*id))
		throw server_error(server_error::invalid_identifier);
	if (!target || target->empty())
		throw server_error(server_error::invalid_nickname);
	if (!channel || channel->empty())
		throw server_error(server_error::invalid_channel);

	bot.servers().require(*id)->invite(*target, *channel);
	client.success("server-invite");
}

// }}}

// {{{ server_join_command

auto server_join_command::get_name() const noexcept -> std::string_view
{
	return "server-join";
}

void server_join_command::exec(bot& bot, transport_client& client, const document& args)
{
	const auto id = args.get<std::string>("server");
	const auto channel = args.get<std::string>("channel");
	const auto password = args.optional<std::string>("password", "");

	if (!id || !string_util::is_identifier(*id))
		throw server_error(server_error::invalid_identifier);
	if (!channel || channel->empty())
		throw server_error(server_error::invalid_channel);
	if (!password)
		throw server_error(server_error::invalid_password);

	bot.servers().require(*id)->join(*channel, *password);
	client.success("server-join");
}

// }}}

// {{{ server_kick_command

auto server_kick_command::get_name() const noexcept -> std::string_view
{
	return "server-kick";
}

void server_kick_command::exec(bot& bot, transport_client& client, const document& args)
{
	const auto id = args.get<std::string>("server");
	const auto target = args.get<std::string>("target");
	const auto channel = args.get<std::string>("channel");
	const auto reason = args.optional<std::string>("reason", "");

	if (!id || !string_util::is_identifier(*id))
		throw server_error(server_error::invalid_identifier);
	if (!target || target->empty())
		throw server_error(server_error::invalid_nickname);
	if (!channel || channel->empty())
		throw server_error(server_error::invalid_channel);
	if (!reason)
		throw server_error(server_error::invalid_message);

	bot.servers().require(*id)->kick(*target, *channel, *reason);
	client.success("server-kick");
}

// }}}

// {{{ server_list_command

auto server_list_command::get_name() const noexcept -> std::string_view
{
	return "server-list";
}

void server_list_command::exec(bot& bot, transport_client& client, const document&)
{
	auto json = nlohmann::json::object();
	auto list = nlohmann::json::array();

	for (const auto& server : bot.servers().list())
		list.push_back(server->get_id());

	client.write({
		{ "command",    "server-list"   },
		{ "list",       std::move(list) }
	});
}

// }}}

// {{{ server_me_command

auto server_me_command::get_name() const noexcept -> std::string_view
{
	return "server-me";
}

void server_me_command::exec(bot& bot, transport_client& client, const document& args)
{
	const auto id = args.get<std::string>("server");
	const auto channel = args.get<std::string>("target");
	const auto message = args.optional<std::string>("message", "");

	if (!id || !string_util::is_identifier(*id))
		throw server_error(server_error::invalid_identifier);
	if (!channel || channel->empty())
		throw server_error(server_error::invalid_channel);
	if (!message)
		throw server_error(server_error::invalid_message);

	bot.servers().require(*id)->me(*channel, *message);
	client.success("server-me");
}

// }}}

// {{{ server_message_command

auto server_message_command::get_name() const noexcept -> std::string_view
{
	return "server-message";
}

void server_message_command::exec(bot& bot, transport_client& client, const document& args)
{
	const auto id = args.get<std::string>("server");
	const auto channel = args.get<std::string>("target");
	const auto message = args.optional<std::string>("message", "");

	if (!id || !string_util::is_identifier(*id))
		throw server_error(server_error::invalid_identifier);
	if (!channel || channel->empty())
		throw server_error(server_error::invalid_channel);
	if (!message)
		throw server_error(server_error::invalid_message);

	bot.servers().require(*id)->message(*channel, *message);
	client.success("server-message");
}

// }}}

// {{{ server_mode_command

auto server_mode_command::get_name() const noexcept -> std::string_view
{
	return "server-mode";
}

void server_mode_command::exec(bot& bot, transport_client& client, const document& args)
{
	const auto id = args.get<std::string>("server");
	const auto channel = args.get<std::string>("channel");
	const auto mode = args.get<std::string>("mode");
	const auto limit = args.optional<std::string>("limit", "");
	const auto user = args.optional<std::string>("user", "");
	const auto mask = args.optional<std::string>("mask", "");

	if (!id || !string_util::is_identifier(*id))
		throw server_error(server_error::invalid_identifier);
	if (!channel || channel->empty())
		throw server_error(server_error::invalid_channel);
	if (!mode || mode->empty())
		throw server_error(server_error::invalid_mode);
	if (!limit || !user || !mask)
		throw server_error(server_error::invalid_mode);

	bot.servers().require(*id)->mode(*channel, *mode, *limit, *user, *mask);
	client.success("server-mode");
}

// }}}

// {{{ server_nick_command

auto server_nick_command::get_name() const noexcept -> std::string_view
{
	return "server-nick";
}

void server_nick_command::exec(bot& bot, transport_client& client, const document& args)
{
	const auto id = args.get<std::string>("server");
	const auto nick = args.get<std::string>("nickname");

	if (!id || !string_util::is_identifier(*id))
		throw server_error(server_error::invalid_identifier);
	if (!nick || nick->empty())
		throw server_error(server_error::invalid_nickname);

	bot.servers().require(*id)->set_nickname(*nick);
	client.success("server-nick");
}

// }}}

// {{{ server_notice_command

auto server_notice_command::get_name() const noexcept -> std::string_view
{
	return "server-notice";
}

void server_notice_command::exec(bot& bot, transport_client& client, const document& args)
{
	const auto id = args.get<std::string>("server");
	const auto channel = args.get<std::string>("target");
	const auto message = args.optional<std::string>("message", "");

	if (!id || !string_util::is_identifier(*id))
		throw server_error(server_error::invalid_identifier);
	if (!channel || channel->empty())
		throw server_error(server_error::invalid_channel);
	if (!message)
		throw server_error(server_error::invalid_message);

	bot.servers().require(*id)->notice(*channel, *message);
	client.success("server-notice");
}

// }}}

// {{{ server_part_command

auto server_part_command::get_name() const noexcept -> std::string_view
{
	return "server-part";
}

void server_part_command::exec(bot& bot, transport_client& client, const document& args)
{
	const auto id = args.get<std::string>("server");
	const auto channel = args.get<std::string>("channel");
	const auto reason = args.optional<std::string>("reason", "");

	if (!id || !string_util::is_identifier(*id))
		throw server_error(server_error::invalid_identifier);
	if (!channel || channel->empty())
		throw server_error(server_error::invalid_channel);
	if (!reason)
		throw server_error(server_error::invalid_message);

	bot.servers().require(*id)->part(*channel, *reason);
	client.success("server-part");
}

// }}}

// {{{ server_reconnect_command

auto server_reconnect_command::get_name() const noexcept -> std::string_view
{
	return "server-reconnect";
}

void server_reconnect_command::exec(bot& bot, transport_client& client, const document& args)
{
	const auto it = args.find("server");

	if (it == args.end())
		bot.servers().reconnect();
	else {
		if (!it->is_string() || !string_util::is_identifier(it->get<std::string>()))
			throw server_error(server_error::invalid_identifier);

		bot.servers().reconnect(it->get<std::string>());
	}

	client.success("server-reconnect");
}

// }}}

// {{{ server_topic_command

auto server_topic_command::get_name() const noexcept -> std::string_view
{
	return "server-topic";
}

void server_topic_command::exec(bot& bot, transport_client& client, const document& args)
{
	const auto id = args.get<std::string>("server");
	const auto channel = args.get<std::string>("channel");
	const auto topic = args.optional<std::string>("topic", "");

	if (!id || !string_util::is_identifier(*id))
		throw server_error(server_error::invalid_identifier);
	if (!channel || channel->empty())
		throw server_error(server_error::invalid_channel);
	if (!topic)
		throw server_error(server_error::invalid_message);

	bot.servers().require(*id)->topic(*channel, *topic);
	client.success("server-topic");
}

// }}}

} // !irccd::daemon
