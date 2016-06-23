/*
 * plugin-js.cpp -- JavaScript plugins for irccd
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

#include "sysconfig.hpp"

#if defined(HAVE_STAT)
#  include <sys/stat.h>
#  include <cerrno>
#  include <cstring>
#endif

#include "fs.hpp"
#include "irccd.hpp"
#include "logger.hpp"
#include "mod-plugin.hpp"
#include "mod-server.hpp"
#include "plugin-js.hpp"
#include "service-module.hpp"
#include "service-plugin.hpp"
#include "timer.hpp"

namespace irccd {

std::unordered_map<std::string, std::string> JsPlugin::getTable(const char *name) const
{
    StackAssert sa(m_context);
    std::unordered_map<std::string, std::string> result;

    duk_get_global_string(m_context, name);
    dukx_enumerate(m_context, -1, 0, true, [&] (auto ctx) {
        result.emplace(duk_to_string(ctx, -2), duk_to_string(ctx, -1));
    });
    duk_pop(m_context);

    return result;
}

void JsPlugin::putTable(const char *name, const std::unordered_map<std::string, std::string> &vars)
{
    StackAssert sa(m_context);

    duk_get_global_string(m_context, name);

    for (const auto &pair : vars) {
        dukx_push_std_string(m_context, pair.second);
        duk_put_prop_string(m_context, -2, pair.first.c_str());
    }

    duk_pop(m_context);
}

void JsPlugin::call(const std::string &name, unsigned nargs)
{
    duk_get_global_string(m_context, name.c_str());

    if (duk_get_type(m_context, -1) == DUK_TYPE_UNDEFINED)
        // Function not defined, remove the undefined value and all arguments.
        duk_pop_n(m_context, nargs + 1);
    else {
        // Call the function and discard the result.
        duk_insert(m_context, -nargs - 1);

        if (duk_pcall(m_context, nargs) != 0)
            throw dukx_exception(m_context, -1, true);

        duk_pop(m_context);
    }
}

void JsPlugin::putModules(Irccd &irccd)
{
    m_modules = irccd.moduleService().modules();

    for (const auto &module : irccd.moduleService().modules())
        module->load(irccd, std::static_pointer_cast<JsPlugin>(shared_from_this()));
}

void JsPlugin::putVars()
{
    StackAssert sa(m_context);

    duk_push_pointer(m_context, this);
    duk_put_global_string(m_context, "\xff""\xff""plugin");
    dukx_push_std_string(m_context, name());
    duk_put_global_string(m_context, "\xff""\xff""name");
    dukx_push_std_string(m_context, path());
    duk_put_global_string(m_context, "\xff""\xff""path");
}

void JsPlugin::putPath(const std::string &varname, const std::string &append, path::Path type)
{
    StackAssert sa(m_context);

    bool found = true;
    std::string foundpath;

    // Use the first existing directory available.
    for (const auto &p : path::list(type)) {
        foundpath = path::clean(p + append);

        if (fs::exists(foundpath)) {
            found = true;
            break;
        }
    }

    // Use the system as default.
    if (!found)
        foundpath = path::clean(path::get(type, path::OwnerSystem) + append);

    duk_get_global_string(m_context, "Irccd");
    duk_get_prop_string(m_context, -1, "Plugin");
    dukx_push_std_string(m_context, foundpath);
    duk_put_prop_string(m_context, -2, varname.c_str());
    duk_pop_2(m_context);
}

JsPlugin::JsPlugin(std::string name, std::string path)
    : Plugin(name, path)
{
    /*
     * Create two special tables for configuration and formats, they are referenced later as
     *
     *   - Irccd.Plugin.config
     *   - Irccd.Plugin.format
     *
     * In mod-plugin.cpp.
     */
    duk_push_object(m_context);
    duk_put_global_string(m_context, PluginConfigProperty);
    duk_push_object(m_context);
    duk_put_global_string(m_context, PluginFormatProperty);
}

PluginConfig JsPlugin::config()
{
    return getTable(PluginConfigProperty);
}

void JsPlugin::setConfig(PluginConfig config)
{
    putTable(PluginConfigProperty, config);
}

PluginFormats JsPlugin::formats()
{
    return getTable(PluginFormatProperty);
}

void JsPlugin::setFormats(PluginFormats formats)
{
    putTable(PluginFormatProperty, formats);
}

void JsPlugin::onChannelMode(Irccd &, const ChannelModeEvent &event)
{
    StackAssert sa(m_context);

    duk_push_server(m_context, std::move(event.server));
    dukx_push_std_string(m_context, event.origin);
    dukx_push_std_string(m_context, event.channel);
    dukx_push_std_string(m_context, event.mode);
    dukx_push_std_string(m_context, event.argument);
    call("onChannelMode", 5);
}

void JsPlugin::onChannelNotice(Irccd &, const ChannelNoticeEvent &event)
{
    StackAssert sa(m_context);

    duk_push_server(m_context, std::move(event.server));
    dukx_push_std_string(m_context, event.origin);
    dukx_push_std_string(m_context, event.channel);
    dukx_push_std_string(m_context, event.message);
    call("onChannelNotice", 4);
}

void JsPlugin::onCommand(Irccd &, const MessageEvent &event)
{
    StackAssert sa(m_context);

    duk_push_server(m_context, std::move(event.server));
    dukx_push_std_string(m_context, event.origin);
    dukx_push_std_string(m_context, event.channel);
    dukx_push_std_string(m_context, event.message);
    call("onCommand", 4);
}

void JsPlugin::onConnect(Irccd &, const ConnectEvent &event)
{
    StackAssert sa(m_context);

    duk_push_server(m_context, std::move(event.server));
    call("onConnect", 1);
}

void JsPlugin::onInvite(Irccd &, const InviteEvent &event)
{
    StackAssert sa(m_context);

    duk_push_server(m_context, std::move(event.server));
    dukx_push_std_string(m_context, event.origin);
    dukx_push_std_string(m_context, event.channel);
    call("onInvite", 3);
}

void JsPlugin::onJoin(Irccd &, const JoinEvent &event)
{
    StackAssert sa(m_context);

    duk_push_server(m_context, std::move(event.server));
    dukx_push_std_string(m_context, event.origin);
    dukx_push_std_string(m_context, event.channel);
    call("onJoin", 3);
}

void JsPlugin::onKick(Irccd &, const KickEvent &event)
{
    StackAssert sa(m_context);

    duk_push_server(m_context, std::move(event.server));
    dukx_push_std_string(m_context, event.origin);
    dukx_push_std_string(m_context, event.channel);
    dukx_push_std_string(m_context, event.target);
    dukx_push_std_string(m_context, event.reason);
    call("onKick", 5);
}

void JsPlugin::onLoad(Irccd &irccd)
{
    StackAssert sa(m_context);

    /*
     * Duktape currently emit useless warnings when a file do
     * not exists so we do a homemade access.
     */
#if defined(HAVE_STAT)
    struct stat st;

    if (::stat(path().c_str(), &st) < 0)
        throw std::runtime_error(std::strerror(errno));
#endif

    /*
     * dataPath: DATA + plugin/name (e.g ~/.local/share/irccd/plugins/<name>/)
     * configPath: CONFIG + plugin/name (e.g ~/.config/irccd/plugin/<name>/)
     */
    putModules(irccd);
    putVars();
    putPath("dataPath", "plugin/" + name(), path::PathData);
    putPath("configPath", "plugin/" + name(), path::PathConfig);
    putPath("cachePath", "plugin/" + name(), path::PathCache);

    // Try to load the file (does not call onLoad yet).
    if (duk_peval_file(m_context, path().c_str()) != 0)
        throw dukx_exception(m_context, -1, true);

    duk_pop(m_context);

    /*
     * We put configuration and formats after loading the file and before calling onLoad to allow the plugin adding configuration
     * to Irccd.Plugin.(config|format) before the user.
     */
    setConfig(irccd.pluginService().config(name()));
    setFormats(irccd.pluginService().formats(name()));

    // Read metadata .
    duk_get_global_string(m_context, "info");

    if (duk_get_type(m_context, -1) == DUK_TYPE_OBJECT) {
        // 'author'
        duk_get_prop_string(m_context, -1, "author");
        setAuthor(duk_is_string(m_context, -1) ? duk_get_string(m_context, -1) : author());
        duk_pop(m_context);

        // 'license'
        duk_get_prop_string(m_context, -1, "license");
        setLicense(duk_is_string(m_context, -1) ? duk_get_string(m_context, -1) : license());
        duk_pop(m_context);

        // 'summary'
        duk_get_prop_string(m_context, -1, "summary");
        setSummary(duk_is_string(m_context, -1) ? duk_get_string(m_context, -1) : summary());
        duk_pop(m_context);

        // 'version'
        duk_get_prop_string(m_context, -1, "version");
        setVersion(duk_is_string(m_context, -1) ? duk_get_string(m_context, -1) : version());
        duk_pop(m_context);
    }

    duk_pop(m_context);
    call("onLoad", 0);
}

void JsPlugin::onMessage(Irccd &, const MessageEvent &event)
{
    StackAssert sa(m_context);

    duk_push_server(m_context, std::move(event.server));
    dukx_push_std_string(m_context, event.origin);
    dukx_push_std_string(m_context, event.channel);
    dukx_push_std_string(m_context, event.message);
    call("onMessage", 4);
}

void JsPlugin::onMe(Irccd &, const MeEvent &event)
{
    StackAssert sa(m_context);

    duk_push_server(m_context, std::move(event.server));
    dukx_push_std_string(m_context, event.origin);
    dukx_push_std_string(m_context, event.channel);
    dukx_push_std_string(m_context, event.message);
    call("onMe", 4);
}

void JsPlugin::onMode(Irccd &, const ModeEvent &event)
{
    StackAssert sa(m_context);

    duk_push_server(m_context, std::move(event.server));
    dukx_push_std_string(m_context, event.origin);
    dukx_push_std_string(m_context, event.mode);
    call("onMode", 3);
}

void JsPlugin::onNames(Irccd &, const NamesEvent &event)
{
    StackAssert sa(m_context);

    duk_push_server(m_context, std::move(event.server));
    dukx_push_std_string(m_context, event.channel);
    dukx_push_array(m_context, event.names, dukx_push_std_string);
    call("onNames", 3);
}

void JsPlugin::onNick(Irccd &, const NickEvent &event)
{
    StackAssert sa(m_context);

    duk_push_server(m_context, std::move(event.server));
    dukx_push_std_string(m_context, event.origin);
    dukx_push_std_string(m_context, event.nickname);
    call("onNick", 3);
}

void JsPlugin::onNotice(Irccd &, const NoticeEvent &event)
{
    StackAssert sa(m_context);

    duk_push_server(m_context, std::move(event.server));
    dukx_push_std_string(m_context, event.origin);
    dukx_push_std_string(m_context, event.message);
    call("onNotice", 3);
}

void JsPlugin::onPart(Irccd &, const PartEvent &event)
{
    StackAssert sa(m_context);

    duk_push_server(m_context, std::move(event.server));
    dukx_push_std_string(m_context, event.origin);
    dukx_push_std_string(m_context, event.channel);
    dukx_push_std_string(m_context, event.reason);
    call("onPart", 4);
}

void JsPlugin::onQuery(Irccd &, const QueryEvent &event)
{
    StackAssert sa(m_context);

    duk_push_server(m_context, std::move(event.server));
    dukx_push_std_string(m_context, event.origin);
    dukx_push_std_string(m_context, event.message);
    call("onQuery", 3);
}

void JsPlugin::onQueryCommand(Irccd &, const QueryEvent &event)
{
    StackAssert sa(m_context);

    duk_push_server(m_context, std::move(event.server));
    dukx_push_std_string(m_context, event.origin);
    dukx_push_std_string(m_context, event.message);
    call("onQueryCommand", 3);
}

void JsPlugin::onReload(Irccd &)
{
    StackAssert sa(m_context);

    call("onReload");
}

void JsPlugin::onTopic(Irccd &, const TopicEvent &event)
{
    StackAssert sa(m_context);

    duk_push_server(m_context, std::move(event.server));
    dukx_push_std_string(m_context, event.origin);
    dukx_push_std_string(m_context, event.channel);
    dukx_push_std_string(m_context, event.topic);
    call("onTopic", 4);
}

void JsPlugin::onUnload(Irccd &irccd)
{
    StackAssert sa(m_context);

    call("onUnload");

    for (const auto &module : m_modules)
        module->unload(irccd, std::static_pointer_cast<JsPlugin>(shared_from_this()));
}

void JsPlugin::onWhois(Irccd &, const WhoisEvent &event)
{
    StackAssert sa(m_context);

    duk_push_server(m_context, std::move(event.server));
    duk_push_object(m_context);
    dukx_push_std_string(m_context, event.whois.nick);
    duk_put_prop_string(m_context, -2, "nickname");
    dukx_push_std_string(m_context, event.whois.user);
    duk_put_prop_string(m_context, -2, "username");
    dukx_push_std_string(m_context, event.whois.realname);
    duk_put_prop_string(m_context, -2, "realname");
    dukx_push_std_string(m_context, event.whois.host);
    duk_put_prop_string(m_context, -2, "host");
    dukx_push_array(m_context, event.whois.channels, dukx_push_std_string);
    duk_put_prop_string(m_context, -2, "channels");
    call("onWhois", 2);
}

} // !irccd
