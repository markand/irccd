/*
 * js_plugin.cpp -- Javascript plugins for irccd
 *
 * Copyright (c) 2013-2018 David Demelier <markand@malikania.fr>
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

#include <irccd/daemon/irccd.hpp>

#include "js_api.hpp"
#include "js_plugin.hpp"
#include "server_js_api.hpp"

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

void js_plugin::push() noexcept
{
}

template <typename Value, typename... Args>
void js_plugin::push(Value&& value, Args&&... args)
{
	duk::push(context_, std::forward<Value>(value));
	push(std::forward<Args>(args)...);
}

template <typename... Args>
void js_plugin::call(const std::string& func, Args&&... args)
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

js_plugin::js_plugin(std::string id, std::string path)
	: plugin(std::move(id))
	, path_(path)
{
	duk::stack_guard sa(context_);

	/*
	 * Create two special tables for configuration and formats, they are
	 * referenced later as
	 *
	 *   - Irccd.Plugin.config
	 *   - Irccd.Plugin.format
	 *   - Irccd.Plugin.paths
	 *
	 * In js_plugin_module.cpp.
	 */
	duk_push_object(context_);
	duk_put_global_string(context_, config_property.data());
	duk_push_object(context_);
	duk_put_global_string(context_, format_property.data());
	duk_push_object(context_);
	duk_put_global_string(context_, paths_property.data());

	duk_push_pointer(context_, this);
	duk_put_global_string(context_, "\xff""\xff""plugin");
	duk::push(context_, path);
	duk_put_global_string(context_, "\xff""\xff""path");
}

auto js_plugin::get_context() noexcept -> duk::context&
{
	return context_;
}

auto js_plugin::get_name() const noexcept -> std::string_view
{
	return get_metadata(context_, "name");
}

auto js_plugin::get_author() const noexcept -> std::string_view
{
	return get_metadata(context_, "author");
}

auto js_plugin::get_license() const noexcept -> std::string_view
{
	return get_metadata(context_, "license");
}

auto js_plugin::get_summary() const noexcept -> std::string_view
{
	return get_metadata(context_, "summary");
}

auto js_plugin::get_version() const noexcept -> std::string_view
{
	return get_metadata(context_, "version");
}

auto js_plugin::get_options() const -> map
{
	return get_table(context_, config_property);
}

void js_plugin::set_options(const map& map)
{
	set_table(context_, config_property, map);
}

auto js_plugin::get_formats() const -> map
{
	return get_table(context_, format_property);
}

void js_plugin::set_formats(const map& map)
{
	set_table(context_, format_property, map);
}

auto js_plugin::get_paths() const -> map
{
	return get_table(context_, paths_property);
}

void js_plugin::set_paths(const map& map)
{
	set_table(context_, paths_property, map);
}

void js_plugin::open()
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

void js_plugin::handle_command(irccd&, const message_event& event)
{
	call("onCommand", event.server, event.origin, event.channel, event.message);
}

void js_plugin::handle_connect(irccd&, const connect_event& event)
{
	call("onConnect", event.server);
}

void js_plugin::handle_disconnect(irccd&, const disconnect_event& event)
{
	call("onDisconnect", event.server);
}

void js_plugin::handle_invite(irccd&, const invite_event& event)
{
	call("onInvite", event.server, event.origin, event.channel);
}

void js_plugin::handle_join(irccd&, const join_event& event)
{
	call("onJoin", event.server, event.origin, event.channel);
}

void js_plugin::handle_kick(irccd&, const kick_event& event)
{
	call("onKick", event.server, event.origin, event.channel, event.target, event.reason);
}

void js_plugin::handle_load(irccd&)
{
	call("onLoad");
}

void js_plugin::handle_message(irccd&, const message_event& event)
{
	call("onMessage", event.server, event.origin, event.channel, event.message);
}

void js_plugin::handle_me(irccd&, const me_event& event)
{
	call("onMe", event.server, event.origin, event.channel, event.message);
}

void js_plugin::handle_mode(irccd&, const mode_event& event)
{
	call("onMode", event.server, event.origin, event.channel, event.mode,
		event.limit, event.user, event.mask);
}

void js_plugin::handle_names(irccd&, const names_event& event)
{
	call("onNames", event.server, event.channel, event.names);
}

void js_plugin::handle_nick(irccd&, const nick_event& event)
{
	call("onNick", event.server, event.origin, event.nickname);
}

void js_plugin::handle_notice(irccd&, const notice_event& event)
{
	call("onNotice", event.server, event.origin, event.channel, event.message);
}

void js_plugin::handle_part(irccd&, const part_event& event)
{
	call("onPart", event.server, event.origin, event.channel, event.reason);
}

void js_plugin::handle_reload(irccd&)
{
	call("onReload");
}

void js_plugin::handle_topic(irccd&, const topic_event& event)
{
	call("onTopic", event.server, event.origin, event.channel, event.topic);
}

void js_plugin::handle_unload(irccd&)
{
	call("onUnload");
}

void js_plugin::handle_whois(irccd&, const whois_event& event)
{
	call("onWhois", event.server, event.whois);
}

js_plugin_loader::js_plugin_loader(irccd& irccd,
                                   std::vector<std::string> directories,
                                   std::vector<std::string> extensions) noexcept
	: plugin_loader(std::move(directories), std::move(extensions))
	, irccd_(irccd)
{
}

js_plugin_loader::~js_plugin_loader() noexcept = default;

auto js_plugin_loader::get_modules() const noexcept -> const modules&
{
	return modules_;
}

auto js_plugin_loader::get_modules() noexcept -> modules&
{
	return modules_;
}

auto js_plugin_loader::open(std::string_view id, std::string_view path) -> std::shared_ptr<plugin>
{
	auto plugin = std::make_shared<js_plugin>(std::string(id), std::string(path));

	for (const auto& mod : modules_)
		mod->load(irccd_, plugin);

	plugin->open();

	return plugin;
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
