/*
 * js_plugin.cpp -- Javascript plugins for irccd
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

#include <cstring>
#include <cerrno>
#include <fstream>
#include <iterator>
#include <stdexcept>

#include "irccd.hpp"
#include "logger.hpp"
#include "js_plugin.hpp"
#include "server_jsapi.hpp"

namespace irccd {

const std::string js_plugin::config_property{"\xff""\xff""irccd-plugin-config"};
const std::string js_plugin::format_property{"\xff""\xff""irccd-plugin-format"};
const std::string js_plugin::paths_property{"\xff""\xff""irccd-plugin-paths"};

std::unordered_map<std::string, std::string> js_plugin::get_table(const std::string& name) const
{
    StackAssert sa(context_);
    std::unordered_map<std::string, std::string> result;

    duk_get_global_string(context_, name.c_str());
    dukx_enumerate(context_, -1, 0, true, [&] (auto ctx) {
        result.emplace(duk_to_string(ctx, -2), duk_to_string(ctx, -1));
    });
    duk_pop(context_);

    return result;
}

void js_plugin::put_table(const std::string& name, const std::unordered_map<std::string, std::string>& vars)
{
    StackAssert sa(context_);

    duk_get_global_string(context_, name.c_str());

    for (const auto &pair : vars) {
        dukx_push_std_string(context_, pair.second);
        duk_put_prop_string(context_, -2, pair.first.c_str());
    }

    duk_pop(context_);
}

void js_plugin::call(const std::string& name, unsigned nargs)
{
    duk_get_global_string(context_, name.c_str());

    if (duk_get_type(context_, -1) == DUK_TYPE_UNDEFINED)
        // Function not defined, remove the undefined value and all arguments.
        duk_pop_n(context_, nargs + 1);
    else {
        // Call the function and discard the result.
        duk_insert(context_, -nargs - 1);

        if (duk_pcall(context_, nargs) != 0)
            throw dukx_exception(context_, -1, true);

        duk_pop(context_);
    }
}

js_plugin::js_plugin(std::string name, std::string path)
    : plugin(name, path)
{
    StackAssert sa(context_);

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
    dukx_push_std_string(context_, name);
    duk_put_global_string(context_, "\xff""\xff""name");
    dukx_push_std_string(context_, path);
    duk_put_global_string(context_, "\xff""\xff""path");
}

void js_plugin::open()
{
    std::ifstream input(path());

    if (!input)
        throw std::runtime_error(std::strerror(errno));

    std::string data(
        std::istreambuf_iterator<char>(input.rdbuf()),
        std::istreambuf_iterator<char>()
    );

    if (duk_peval_string(context_, data.c_str()))
        throw dukx_exception(context_, -1);

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

void js_plugin::on_channel_mode(irccd& , const channel_mode_event &event)
{
    StackAssert sa(context_);

    dukx_push_server(context_, std::move(event.server));
    dukx_push_std_string(context_, event.origin);
    dukx_push_std_string(context_, event.channel);
    dukx_push_std_string(context_, event.mode);
    dukx_push_std_string(context_, event.argument);
    call("onChannelMode", 5);
}

void js_plugin::on_channel_notice(irccd& , const channel_notice_event &event)
{
    StackAssert sa(context_);

    dukx_push_server(context_, std::move(event.server));
    dukx_push_std_string(context_, event.origin);
    dukx_push_std_string(context_, event.channel);
    dukx_push_std_string(context_, event.message);
    call("onChannelNotice", 4);
}

void js_plugin::on_command(irccd& , const message_event &event)
{
    StackAssert sa(context_);

    dukx_push_server(context_, std::move(event.server));
    dukx_push_std_string(context_, event.origin);
    dukx_push_std_string(context_, event.channel);
    dukx_push_std_string(context_, event.message);
    call("onCommand", 4);
}

void js_plugin::on_connect(irccd& , const connect_event &event)
{
    StackAssert sa(context_);

    dukx_push_server(context_, std::move(event.server));
    call("onConnect", 1);
}

void js_plugin::on_invite(irccd& , const invite_event &event)
{
    StackAssert sa(context_);

    dukx_push_server(context_, std::move(event.server));
    dukx_push_std_string(context_, event.origin);
    dukx_push_std_string(context_, event.channel);
    call("onInvite", 3);
}

void js_plugin::on_join(irccd& , const join_event &event)
{
    StackAssert sa(context_);

    dukx_push_server(context_, std::move(event.server));
    dukx_push_std_string(context_, event.origin);
    dukx_push_std_string(context_, event.channel);
    call("onJoin", 3);
}

void js_plugin::on_kick(irccd& , const kick_event &event)
{
    StackAssert sa(context_);

    dukx_push_server(context_, std::move(event.server));
    dukx_push_std_string(context_, event.origin);
    dukx_push_std_string(context_, event.channel);
    dukx_push_std_string(context_, event.target);
    dukx_push_std_string(context_, event.reason);
    call("onKick", 5);
}

void js_plugin::on_load(irccd&)
{
    StackAssert sa(context_);

    call("onLoad", 0);
}

void js_plugin::on_message(irccd& , const message_event &event)
{
    StackAssert sa(context_);

    dukx_push_server(context_, std::move(event.server));
    dukx_push_std_string(context_, event.origin);
    dukx_push_std_string(context_, event.channel);
    dukx_push_std_string(context_, event.message);
    call("onMessage", 4);
}

void js_plugin::on_me(irccd& , const me_event &event)
{
    StackAssert sa(context_);

    dukx_push_server(context_, std::move(event.server));
    dukx_push_std_string(context_, event.origin);
    dukx_push_std_string(context_, event.channel);
    dukx_push_std_string(context_, event.message);
    call("onMe", 4);
}

void js_plugin::on_mode(irccd& , const mode_event &event)
{
    StackAssert sa(context_);

    dukx_push_server(context_, std::move(event.server));
    dukx_push_std_string(context_, event.origin);
    dukx_push_std_string(context_, event.mode);
    call("onMode", 3);
}

void js_plugin::on_names(irccd& , const names_event &event)
{
    StackAssert sa(context_);

    dukx_push_server(context_, std::move(event.server));
    dukx_push_std_string(context_, event.channel);
    dukx_push_array(context_, event.names, dukx_push_std_string);
    call("onNames", 3);
}

void js_plugin::on_nick(irccd& , const nick_event &event)
{
    StackAssert sa(context_);

    dukx_push_server(context_, std::move(event.server));
    dukx_push_std_string(context_, event.origin);
    dukx_push_std_string(context_, event.nickname);
    call("onNick", 3);
}

void js_plugin::on_notice(irccd& , const notice_event &event)
{
    StackAssert sa(context_);

    dukx_push_server(context_, std::move(event.server));
    dukx_push_std_string(context_, event.origin);
    dukx_push_std_string(context_, event.message);
    call("onNotice", 3);
}

void js_plugin::on_part(irccd& , const part_event &event)
{
    StackAssert sa(context_);

    dukx_push_server(context_, std::move(event.server));
    dukx_push_std_string(context_, event.origin);
    dukx_push_std_string(context_, event.channel);
    dukx_push_std_string(context_, event.reason);
    call("onPart", 4);
}

void js_plugin::on_query(irccd& , const query_event &event)
{
    StackAssert sa(context_);

    dukx_push_server(context_, std::move(event.server));
    dukx_push_std_string(context_, event.origin);
    dukx_push_std_string(context_, event.message);
    call("onQuery", 3);
}

void js_plugin::on_query_command(irccd& , const query_event &event)
{
    StackAssert sa(context_);

    dukx_push_server(context_, std::move(event.server));
    dukx_push_std_string(context_, event.origin);
    dukx_push_std_string(context_, event.message);
    call("onQueryCommand", 3);
}

void js_plugin::on_reload(irccd& )
{
    StackAssert sa(context_);

    call("onReload");
}

void js_plugin::on_topic(irccd& , const topic_event &event)
{
    StackAssert sa(context_);

    dukx_push_server(context_, std::move(event.server));
    dukx_push_std_string(context_, event.origin);
    dukx_push_std_string(context_, event.channel);
    dukx_push_std_string(context_, event.topic);
    call("onTopic", 4);
}

void js_plugin::on_unload(irccd& )
{
    StackAssert sa(context_);

    call("onUnload");
}

void js_plugin::on_whois(irccd& , const whois_event &event)
{
    StackAssert sa(context_);

    dukx_push_server(context_, std::move(event.server));
    duk_push_object(context_);
    dukx_push_std_string(context_, event.whois.nick);
    duk_put_prop_string(context_, -2, "nickname");
    dukx_push_std_string(context_, event.whois.user);
    duk_put_prop_string(context_, -2, "username");
    dukx_push_std_string(context_, event.whois.realname);
    duk_put_prop_string(context_, -2, "realname");
    dukx_push_std_string(context_, event.whois.host);
    duk_put_prop_string(context_, -2, "host");
    dukx_push_array(context_, event.whois.channels, dukx_push_std_string);
    duk_put_prop_string(context_, -2, "channels");
    call("onWhois", 2);
}

js_plugin_loader::js_plugin_loader(irccd& irccd) noexcept
    : plugin_loader({}, { ".js" })
    , irccd_(irccd)
{
}

js_plugin_loader::~js_plugin_loader() noexcept = default;

std::shared_ptr<plugin> js_plugin_loader::open(const std::string& id,
                                               const std::string& path) noexcept
{
    if (path.rfind(".js") == std::string::npos)
        return nullptr;

    try {
        auto plugin = std::make_shared<js_plugin>(id, path);

        for (const auto& mod : modules_)
            mod->load(irccd_, plugin);

        plugin->open();

        return plugin;
    } catch (const std::exception &ex) {
        log::warning() << "plugin " << id << ": " << ex.what() << std::endl;
    }

    return nullptr;
}

} // !irccd
