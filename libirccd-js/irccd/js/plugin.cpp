/*
 * plugin.cpp -- Javascript plugins for irccd
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

#include <cstring>
#include <cerrno>
#include <fstream>
#include <iterator>
#include <stdexcept>

#include <irccd/daemon/bot.hpp>

#include "api.hpp"
#include "plugin.hpp"
#include "server_api.hpp"

using irccd::daemon::bot;
using irccd::daemon::connect_event;
using irccd::daemon::disconnect_event;
using irccd::daemon::invite_event;
using irccd::daemon::join_event;
using irccd::daemon::kick_event;
using irccd::daemon::me_event;
using irccd::daemon::message_event;
using irccd::daemon::mode_event;
using irccd::daemon::names_event;
using irccd::daemon::nick_event;
using irccd::daemon::notice_event;
using irccd::daemon::part_event;
using irccd::daemon::plugin;
using irccd::daemon::plugin_error;
using irccd::daemon::topic_event;
using irccd::daemon::whois_event;
using irccd::daemon::whois_info;

namespace irccd::js {

namespace {

auto get_metadata(duk::context& ctx, std::string_view name) -> std::string_view
{
	std::string_view ret("unknown");

	duk::stack_guard guard(ctx);
	duk_get_global_string(ctx, "info");

	if (duk_get_type(ctx, -1) == DUK_TYPE_OBJECT) {
		duk_get_prop_string(ctx, -1, name.data());

		if (duk_get_type(ctx, -1) == DUK_TYPE_STRING)
			ret = duk_get_string(ctx, -1);

		duk_pop(ctx);
	}

	duk_pop(ctx);

	return ret;
}

auto get_table(duk::context& ctx, std::string_view name) -> plugin::map
{
	plugin::map result;

	duk::stack_guard sa(ctx);
	duk_get_global_string(ctx, name.data());
	duk_enum(ctx, -1, 0);

	while (duk_next(ctx, -1, true)) {
		result.emplace(duk_to_string(ctx, -2), duk_to_string(ctx, -1));
		duk_pop_n(ctx, 2);
	}

	duk_pop_n(ctx, 2);

	return result;
}

void set_table(duk::context& ctx, std::string_view name, const plugin::map& vars)
{
	duk::stack_guard sa(ctx);
	duk_get_global_string(ctx, name.data());

	for (const auto& pair : vars) {
		duk::push(ctx, pair.second);
		duk_put_prop_string(ctx, -2, pair.first.c_str());
	}

	duk_pop(ctx);
}

} // !namespace

void plugin::push() noexcept
{
}

template <typename Value, typename... Args>
void plugin::push(Value&& value, Args&&... args)
{
	duk::push(context_, std::forward<Value>(value));
	push(std::forward<Args>(args)...);
}

template <typename... Args>
void plugin::call(const std::string& func, Args&&... args)
{
	duk::stack_guard sa(context_);

	duk_get_global_string(context_, func.c_str());

	if (duk_get_type(context_, -1) == DUK_TYPE_UNDEFINED) {
		duk_pop(context_);
		return;
	}

	push(std::forward<Args>(args)...);

	if (duk_pcall(context_, sizeof... (Args)) != 0)
		throw plugin_error(plugin_error::exec_error, get_name(), duk::get_stack(context_, -1).get_stack());

	duk_pop(context_);
}

plugin::plugin(std::string id, std::string path)
	: daemon::plugin(std::move(id))
	, path_(path)
{
	duk::stack_guard sa(context_);

	/*
	 * Create two special tables for configuration and templates, they are
	 * referenced later as
	 *
	 *   - Irccd.Plugin.config
	 *   - Irccd.Plugin.templates
	 *   - Irccd.Plugin.paths
	 *
	 * In plugin_module.cpp.
	 */
	duk_push_object(context_);
	duk_put_global_string(context_, config_property.data());
	duk_push_object(context_);
	duk_put_global_string(context_, templates_property.data());
	duk_push_object(context_);
	duk_put_global_string(context_, paths_property.data());

	duk_push_pointer(context_, this);
	duk_put_global_string(context_, DUK_HIDDEN_SYMBOL("plugin"));
	duk::push(context_, path);
	duk_put_global_string(context_, DUK_HIDDEN_SYMBOL("path"));
}

auto plugin::get_context() noexcept -> duk::context&
{
	return context_;
}

auto plugin::get_name() const noexcept -> std::string_view
{
	return get_metadata(context_, "name");
}

auto plugin::get_author() const noexcept -> std::string_view
{
	return get_metadata(context_, "author");
}

auto plugin::get_license() const noexcept -> std::string_view
{
	return get_metadata(context_, "license");
}

auto plugin::get_summary() const noexcept -> std::string_view
{
	return get_metadata(context_, "summary");
}

auto plugin::get_version() const noexcept -> std::string_view
{
	return get_metadata(context_, "version");
}

auto plugin::get_options() const -> map
{
	return get_table(context_, config_property);
}

void plugin::set_options(const map& map)
{
	set_table(context_, config_property, map);
}

auto plugin::get_templates() const -> map
{
	return get_table(context_, templates_property);
}

void plugin::set_templates(const map& map)
{
	set_table(context_, templates_property, map);
}

auto plugin::get_paths() const -> map
{
	return get_table(context_, paths_property);
}

void plugin::set_paths(const map& map)
{
	set_table(context_, paths_property, map);
}

void plugin::open()
{
	std::ifstream input(path_);

	if (!input)
		throw plugin_error(plugin_error::exec_error, get_name(), std::strerror(errno));

	std::string data(
		std::istreambuf_iterator<char>(input.rdbuf()),
		std::istreambuf_iterator<char>()
	);

	if (duk_peval_string(context_, data.c_str()))
		throw plugin_error(plugin_error::exec_error, get_name(), duk::get_stack(context_, -1).get_stack());
}

void plugin::handle_command(bot&, const message_event& event)
{
	call("onCommand", event.server, event.origin, event.channel, event.message);
}

void plugin::handle_connect(bot&, const connect_event& event)
{
	call("onConnect", event.server);
}

void plugin::handle_disconnect(bot&, const disconnect_event& event)
{
	call("onDisconnect", event.server);
}

void plugin::handle_invite(bot&, const invite_event& event)
{
	call("onInvite", event.server, event.origin, event.channel);
}

void plugin::handle_join(bot&, const join_event& event)
{
	call("onJoin", event.server, event.origin, event.channel);
}

void plugin::handle_kick(bot&, const kick_event& event)
{
	call("onKick", event.server, event.origin, event.channel, event.target, event.reason);
}

void plugin::handle_load(bot&)
{
	call("onLoad");
}

void plugin::handle_message(bot&, const message_event& event)
{
	call("onMessage", event.server, event.origin, event.channel, event.message);
}

void plugin::handle_me(bot&, const me_event& event)
{
	call("onMe", event.server, event.origin, event.channel, event.message);
}

void plugin::handle_mode(bot&, const mode_event& event)
{
	call("onMode", event.server, event.origin, event.channel, event.mode,
		event.limit, event.user, event.mask);
}

void plugin::handle_names(bot&, const names_event& event)
{
	call("onNames", event.server, event.channel, event.names);
}

void plugin::handle_nick(bot&, const nick_event& event)
{
	call("onNick", event.server, event.origin, event.nickname);
}

void plugin::handle_notice(bot&, const notice_event& event)
{
	call("onNotice", event.server, event.origin, event.channel, event.message);
}

void plugin::handle_part(bot&, const part_event& event)
{
	call("onPart", event.server, event.origin, event.channel, event.reason);
}

void plugin::handle_reload(bot&)
{
	call("onReload");
}

void plugin::handle_topic(bot&, const topic_event& event)
{
	call("onTopic", event.server, event.origin, event.channel, event.topic);
}

void plugin::handle_unload(bot&)
{
	call("onUnload");
}

void plugin::handle_whois(bot&, const whois_event& event)
{
	call("onWhois", event.server, event.whois);
}

plugin_loader::plugin_loader(bot& bot,
                             std::vector<std::string> directories,
                             std::vector<std::string> extensions) noexcept
	: daemon::plugin_loader(std::move(directories), std::move(extensions))
	, bot_(bot)
{
}

plugin_loader::~plugin_loader() noexcept = default;

auto plugin_loader::get_modules() const noexcept -> const modules&
{
	return modules_;
}

auto plugin_loader::get_modules() noexcept -> modules&
{
	return modules_;
}

auto plugin_loader::open(std::string_view id, std::string_view path) -> std::shared_ptr<daemon::plugin>
{
	auto plg = std::make_shared<plugin>(std::string(id), std::string(path));

	for (const auto& mod : modules_)
		mod->load(bot_, *plg);

	plg->open();

	return plg;
}

void duk::type_traits<whois_info>::push(duk_context* ctx, const whois_info& whois)
{
	duk_push_object(ctx);
	duk::push(ctx, whois.nick);
	duk_put_prop_string(ctx, -2, "nickname");
	duk::push(ctx, whois.user);
	duk_put_prop_string(ctx, -2, "username");
	duk::push(ctx, whois.realname);
	duk_put_prop_string(ctx, -2, "realname");
	duk::push(ctx, whois.hostname);
	duk_put_prop_string(ctx, -2, "hostname");
	duk::push(ctx, whois.channels);
	duk_put_prop_string(ctx, -2, "channels");
}

} // !irccd::js
