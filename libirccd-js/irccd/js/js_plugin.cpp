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
#include <irccd/daemon/logger.hpp>

#include "directory_jsapi.hpp"
#include "duktape_vector.hpp"
#include "elapsed_timer_jsapi.hpp"
#include "file_jsapi.hpp"
#include "irccd_jsapi.hpp"
#include "js_plugin.hpp"
#include "logger_jsapi.hpp"
#include "plugin_jsapi.hpp"
#include "server_jsapi.hpp"
#include "system_jsapi.hpp"
#include "timer_jsapi.hpp"
#include "unicode_jsapi.hpp"
#include "util_jsapi.hpp"

namespace irccd {

const std::string js_plugin::config_property{"\xff""\xff""irccd-plugin-config"};
const std::string js_plugin::format_property{"\xff""\xff""irccd-plugin-format"};
const std::string js_plugin::paths_property{"\xff""\xff""irccd-plugin-paths"};

std::unordered_map<std::string, std::string> js_plugin::get_table(const std::string& name) const
{
    dukx_stack_assert sa(context_);
    std::unordered_map<std::string, std::string> result;

    duk_get_global_string(context_, name.c_str());
    duk_enum(context_, -1, 0);

    while (duk_next(context_, -1, true)) {
        result.emplace(duk_to_string(context_, -2), duk_to_string(context_, -1));
        duk_pop_n(context_, 2);
    }

    duk_pop_n(context_, 2);

    return result;
}

void js_plugin::put_table(const std::string& name, const std::unordered_map<std::string, std::string>& vars)
{
    dukx_stack_assert sa(context_);

    duk_get_global_string(context_, name.c_str());

    for (const auto& pair : vars) {
        dukx_push(context_, pair.second);
        duk_put_prop_string(context_, -2, pair.first.c_str());
    }

    duk_pop(context_);
}

void js_plugin::push() noexcept
{
}

template <typename Value, typename... Args>
void js_plugin::push(Value&& value, Args&&... args)
{
    dukx_push(context_, std::forward<Value>(value));
    push(std::forward<Args>(args)...);
}

template <typename... Args>
void js_plugin::call(const std::string& func, Args&&... args)
{
    dukx_stack_assert sa(context_);

    duk_get_global_string(context_, func.c_str());

    if (duk_get_type(context_, -1) == DUK_TYPE_UNDEFINED) {
        duk_pop(context_);
        return;
    }

    push(std::forward<Args>(args)...);

    if (duk_pcall(context_, sizeof... (Args)) != 0)
        throw plugin_error(plugin_error::exec_error, name(), dukx_stack(context_, -1).stack());

    duk_pop(context_);
}

js_plugin::js_plugin(std::string name, std::string path)
    : plugin(name, path)
{
    dukx_stack_assert sa(context_);

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
    duk_put_global_string(context_, config_property.c_str());
    duk_push_object(context_);
    duk_put_global_string(context_, format_property.c_str());
    duk_push_object(context_);
    duk_put_global_string(context_, paths_property.c_str());

    duk_push_pointer(context_, this);
    duk_put_global_string(context_, "\xff""\xff""plugin");
    dukx_push(context_, name);
    duk_put_global_string(context_, "\xff""\xff""name");
    dukx_push(context_, path);
    duk_put_global_string(context_, "\xff""\xff""path");
}

void js_plugin::open()
{
    std::ifstream input(path());

    // TODO: add error message here.
    if (!input)
        throw plugin_error(plugin_error::exec_error, name(), std::strerror(errno));

    std::string data(
        std::istreambuf_iterator<char>(input.rdbuf()),
        std::istreambuf_iterator<char>()
    );

    if (duk_peval_string(context_, data.c_str()))
        throw plugin_error(plugin_error::exec_error, name(), dukx_stack(context_, -1).stack());

    // Read metadata.
    duk_get_global_string(context_, "info");

    if (duk_get_type(context_, -1) == DUK_TYPE_OBJECT) {
        duk_get_prop_string(context_, -1, "author");
        set_author(duk_is_string(context_, -1) ? duk_get_string(context_, -1) : author());
        duk_get_prop_string(context_, -2, "license");
        set_license(duk_is_string(context_, -1) ? duk_get_string(context_, -1) : license());
        duk_get_prop_string(context_, -3, "summary");
        set_summary(duk_is_string(context_, -1) ? duk_get_string(context_, -1) : summary());
        duk_get_prop_string(context_, -4, "version");
        set_version(duk_is_string(context_, -1) ? duk_get_string(context_, -1) : version());
        duk_pop_n(context_, 4);
    }

    duk_pop(context_);
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

std::unique_ptr<js_plugin_loader> js_plugin_loader::defaults(irccd& irccd)
{
    auto self = std::make_unique<js_plugin_loader>(irccd);

    self->modules().push_back(std::make_unique<irccd_jsapi>());
    self->modules().push_back(std::make_unique<directory_jsapi>());
    self->modules().push_back(std::make_unique<elapsed_timer_jsapi>());
    self->modules().push_back(std::make_unique<file_jsapi>());
    self->modules().push_back(std::make_unique<logger_jsapi>());
    self->modules().push_back(std::make_unique<plugin_jsapi>());
    self->modules().push_back(std::make_unique<server_jsapi>());
    self->modules().push_back(std::make_unique<system_jsapi>());
    self->modules().push_back(std::make_unique<timer_jsapi>());
    self->modules().push_back(std::make_unique<unicode_jsapi>());
    self->modules().push_back(std::make_unique<util_jsapi>());

    return self;
}

js_plugin_loader::js_plugin_loader(irccd& irccd) noexcept
    : plugin_loader({}, { ".js" })
    , irccd_(irccd)
{
}

js_plugin_loader::~js_plugin_loader() noexcept = default;

std::shared_ptr<plugin> js_plugin_loader::open(const std::string& id,
                                               const std::string& path)
{
    if (path.rfind(".js") == std::string::npos)
        return nullptr;

    auto plugin = std::make_shared<js_plugin>(id, path);

    for (const auto& mod : modules_)
        mod->load(irccd_, plugin);

    plugin->open();

    return plugin;
}

void dukx_type_traits<whois_info>::push(duk_context* ctx, const whois_info& whois)
{
    duk_push_object(ctx);
    dukx_push(ctx, whois.nick);
    duk_put_prop_string(ctx, -2, "nickname");
    dukx_push(ctx, whois.user);
    duk_put_prop_string(ctx, -2, "username");
    dukx_push(ctx, whois.realname);
    duk_put_prop_string(ctx, -2, "realname");
    dukx_push(ctx, whois.host);
    duk_put_prop_string(ctx, -2, "host");
    dukx_push(ctx, whois.channels);
    duk_put_prop_string(ctx, -2, "channels");
}

} // !irccd
