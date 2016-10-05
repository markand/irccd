/*
 * plugin-js.hpp -- JavaScript plugins for irccd
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

#ifndef IRCCD_PLUGIN_JS_HPP
#define IRCCD_PLUGIN_JS_HPP

/**
 * \file plugin-js.hpp
 * \brief JavaScript plugins for irccd.
 */

#include "duktape.hpp"
#include "path.hpp"
#include "plugin.hpp"

namespace irccd {

/**
 * \brief JavaScript plugins for irccd.
 * \ingroup plugins
 */
class JsPlugin : public Plugin {
public:
    /**
     * Global property where to read/write plugin configuration (object).
     */
    static const char ConfigProperty[];

    /**
     * Global property where to read/write plugin formats (object).
     */
    static const char FormatProperty[];

private:
    // JavaScript context
    UniqueContext m_context;

    // Private helpers.
    std::unordered_map<std::string, std::string> getTable(const char *name) const;
    void putTable(const char *name, const std::unordered_map<std::string, std::string> &vars);
    void call(const std::string &name, unsigned nargs = 0);
    void putVars();
    void putPath(const std::string &varname, const std::string &append, path::Path type);

public:
    /**
     * Constructor.
     *
     * \param name the plugin name
     * \param path the path to the plugin
     */
    IRCCD_EXPORT JsPlugin(std::string name, std::string path);

    /**
     * Access the Duktape context.
     *
     * \return the context
     */
    inline UniqueContext &context() noexcept
    {
        return m_context;
    }

    /**
     * \copydoc Plugin::config
     */
    PluginConfig config() override
    {
        return getTable(ConfigProperty);
    }

    /**
     * \copydoc Plugin::setConfig
     */
    void setConfig(PluginConfig config) override
    {
        putTable(ConfigProperty, config);
    }

    /**
     * \copydoc Plugin::formats
     */
    PluginFormats formats() override
    {
        return getTable(FormatProperty);
    }

    /**
     * \copydoc Plugin::setFormats
     */
    void setFormats(PluginFormats formats) override
    {
        putTable(FormatProperty, formats);
    }

    /**
     * \copydoc Plugin::onCommand
     */
    IRCCD_EXPORT void onCommand(Irccd &irccd, const MessageEvent &event) override;

    /**
     * \copydoc Plugin::onConnect
     */
    IRCCD_EXPORT void onConnect(Irccd &irccd, const ConnectEvent &event) override;

    /**
     * \copydoc Plugin::onChannelMode
     */
    IRCCD_EXPORT void onChannelMode(Irccd &irccd, const ChannelModeEvent &event) override;

    /**
     * \copydoc Plugin::onChannelNotice
     */
    IRCCD_EXPORT void onChannelNotice(Irccd &irccd, const ChannelNoticeEvent &event) override;

    /**
     * \copydoc Plugin::onInvite
     */
    IRCCD_EXPORT void onInvite(Irccd &irccd, const InviteEvent &event) override;

    /**
     * \copydoc Plugin::onJoin
     */
    IRCCD_EXPORT void onJoin(Irccd &irccd, const JoinEvent &event) override;

    /**
     * \copydoc Plugin::onKick
     */
    IRCCD_EXPORT void onKick(Irccd &irccd, const KickEvent &event) override;

    /**
     * \copydoc Plugin::onLoad
     */
    IRCCD_EXPORT void onLoad(Irccd &irccd) override;

    /**
     * \copydoc Plugin::onMessage
     */
    IRCCD_EXPORT void onMessage(Irccd &irccd, const MessageEvent &event) override;

    /**
     * \copydoc Plugin::onMe
     */
    IRCCD_EXPORT void onMe(Irccd &irccd, const MeEvent &event) override;

    /**
     * \copydoc Plugin::onMode
     */
    IRCCD_EXPORT void onMode(Irccd &irccd, const ModeEvent &event) override;

    /**
     * \copydoc Plugin::onNames
     */
    IRCCD_EXPORT void onNames(Irccd &irccd, const NamesEvent &event) override;

    /**
     * \copydoc Plugin::onNick
     */
    IRCCD_EXPORT void onNick(Irccd &irccd, const NickEvent &event) override;

    /**
     * \copydoc Plugin::onNotice
     */
    IRCCD_EXPORT void onNotice(Irccd &irccd, const NoticeEvent &event) override;

    /**
     * \copydoc Plugin::onPart
     */
    IRCCD_EXPORT void onPart(Irccd &irccd, const PartEvent &event) override;

    /**
     * \copydoc Plugin::onQuery
     */
    IRCCD_EXPORT void onQuery(Irccd &irccd, const QueryEvent &event) override;

    /**
     * \copydoc Plugin::onQueryCommand
     */
    IRCCD_EXPORT void onQueryCommand(Irccd &irccd, const QueryEvent &event) override;

    /**
     * \copydoc Plugin::onReload
     */
    IRCCD_EXPORT void onReload(Irccd &irccd) override;

    /**
     * \copydoc Plugin::onTopic
     */
    IRCCD_EXPORT void onTopic(Irccd &irccd, const TopicEvent &event) override;

    /**
     * \copydoc Plugin::onUnload
     */
    IRCCD_EXPORT void onUnload(Irccd &irccd) override;

    /**
     * \copydoc Plugin::onWhois
     */
    IRCCD_EXPORT void onWhois(Irccd &irccd, const WhoisEvent &event) override;
};

/**
 * \brief Implementation for searching Javascript plugins.
 */
class JsPluginLoader : public PluginLoader {
public:
    /**
     * \copydoc PluginLoader::find
     */
    std::shared_ptr<Plugin> open(const std::string &id,
                                 const std::string &path) noexcept override;

    /**
     * \copydoc PluginLoader::find
     */
    std::shared_ptr<Plugin> find(const std::string &id) noexcept override;
};

} // !irccd

#endif // !IRCCD_PLUGIN_JS_HPP
