/*
 * plugin-dynlib.hpp -- native plugin implementation
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

#ifndef IRCCD_PLUGIN_DYNLIB_HPP
#define IRCCD_PLUGIN_DYNLIB_HPP

/**
 * \file plugin-dynlib.hpp
 * \brief Native plugin implementation.
 */

#include "dynlib.hpp"
#include "plugin.hpp"

namespace irccd {

/**
 * \brief Dynlib based plugin.
 * \ingroup plugins
 */
class DynlibPlugin : public Plugin {
private:
    using OnCommand = void (*)(Irccd &, const MessageEvent &);
    using OnConnect = void (*)(Irccd &, const ConnectEvent &);
    using OnChannelMode = void (*)(Irccd &, const ChannelModeEvent &);
    using OnChannelNotice = void (*)(Irccd &, const ChannelNoticeEvent &);
    using OnInvite = void (*)(Irccd &, const InviteEvent &);
    using OnJoin = void (*)(Irccd &, const JoinEvent &);
    using OnKick = void (*)(Irccd &, const KickEvent &);
    using OnLoad = void (*)(Irccd &, DynlibPlugin &);
    using OnMessage = void (*)(Irccd &, const MessageEvent &);
    using OnMe = void (*)(Irccd &, const MeEvent &);
    using OnMode = void (*)(Irccd &, const ModeEvent &);
    using OnNames = void (*)(Irccd &, const NamesEvent &);
    using OnNick = void (*)(Irccd &, const NickEvent &);
    using OnNotice = void (*)(Irccd &, const NoticeEvent &);
    using OnPart = void (*)(Irccd &, const PartEvent &);
    using OnQuery = void (*)(Irccd &, const QueryEvent &);
    using OnQueryCommand = void (*)(Irccd &, const QueryEvent &);
    using OnReload = void (*)(Irccd &, DynlibPlugin &);
    using OnTopic = void (*)(Irccd &, const TopicEvent &);
    using OnUnload = void (*)(Irccd &, DynlibPlugin &);
    using OnWhois = void (*)(Irccd &, const WhoisEvent &);

    Dynlib m_dso;
    OnCommand m_onCommand;
    OnConnect m_onConnect;
    OnChannelMode m_onChannelMode;
    OnChannelNotice m_onChannelNotice;
    OnInvite m_onInvite;
    OnJoin m_onJoin;
    OnKick m_onKick;
    OnLoad m_onLoad;
    OnMessage m_onMessage;
    OnMe m_onMe;
    OnMode m_onMode;
    OnNames m_onNames;
    OnNick m_onNick;
    OnNotice m_onNotice;
    OnPart m_onPart;
    OnQuery m_onQuery;
    OnQueryCommand m_onQueryCommand;
    OnReload m_onReload;
    OnTopic m_onTopic;
    OnUnload m_onUnload;
    OnWhois m_onWhois;

    // Configuration and formats.
    plugin_config m_config;
    plugin_formats m_formats;

public:
    /**
     * Construct the plugin.
     *
     * \param name the name
     * \param path the fully resolved path (must be absolute)
     * \throw std::exception on failures
     */
    DynlibPlugin(std::string name, std::string path);

    /**
     * \copydoc Plugin::onCommand
     */
    IRCCD_EXPORT void onCommand(irccd &irccd, const MessageEvent &event) override;

    /**
     * \copydoc Plugin::onConnect
     */
    IRCCD_EXPORT void onConnect(irccd &irccd, const ConnectEvent &event) override;

    /**
     * \copydoc Plugin::onChannelMode
     */
    IRCCD_EXPORT void onChannelMode(irccd &irccd, const ChannelModeEvent &event) override;

    /**
     * \copydoc Plugin::onChannelNotice
     */
    IRCCD_EXPORT void onChannelNotice(irccd &irccd, const ChannelNoticeEvent &event) override;

    /**
     * \copydoc Plugin::onInvite
     */
    IRCCD_EXPORT void onInvite(irccd &irccd, const InviteEvent &event) override;

    /**
     * \copydoc Plugin::onJoin
     */
    IRCCD_EXPORT void onJoin(irccd &irccd, const JoinEvent &event) override;

    /**
     * \copydoc Plugin::onKick
     */
    IRCCD_EXPORT void onKick(irccd &irccd, const KickEvent &event) override;

    /**
     * \copydoc Plugin::onLoad
     */
    IRCCD_EXPORT void onLoad(irccd &irccd) override;

    /**
     * \copydoc Plugin::onMessage
     */
    IRCCD_EXPORT void onMessage(irccd &irccd, const MessageEvent &event) override;

    /**
     * \copydoc Plugin::onMe
     */
    IRCCD_EXPORT void onMe(irccd &irccd, const MeEvent &event) override;

    /**
     * \copydoc Plugin::onMode
     */
    IRCCD_EXPORT void onMode(irccd &irccd, const ModeEvent &event) override;

    /**
     * \copydoc Plugin::onNames
     */
    IRCCD_EXPORT void onNames(irccd &irccd, const NamesEvent &event) override;

    /**
     * \copydoc Plugin::onNick
     */
    IRCCD_EXPORT void onNick(irccd &irccd, const NickEvent &event) override;

    /**
     * \copydoc Plugin::onNotice
     */
    IRCCD_EXPORT void onNotice(irccd &irccd, const NoticeEvent &event) override;

    /**
     * \copydoc Plugin::onPart
     */
    IRCCD_EXPORT void onPart(irccd &irccd, const PartEvent &event) override;

    /**
     * \copydoc Plugin::onQuery
     */
    IRCCD_EXPORT void onQuery(irccd &irccd, const QueryEvent &event) override;

    /**
     * \copydoc Plugin::onQueryCommand
     */
    IRCCD_EXPORT void onQueryCommand(irccd &irccd, const QueryEvent &event) override;

    /**
     * \copydoc Plugin::onReload
     */
    IRCCD_EXPORT void onReload(irccd &irccd) override;

    /**
     * \copydoc Plugin::onTopic
     */
    IRCCD_EXPORT void onTopic(irccd &irccd, const TopicEvent &event) override;

    /**
     * \copydoc Plugin::onUnload
     */
    IRCCD_EXPORT void onUnload(irccd &irccd) override;

    /**
     * \copydoc Plugin::onWhois
     */
    IRCCD_EXPORT void onWhois(irccd &irccd, const WhoisEvent &event) override;
};

/**
 * \brief Implementation for searching native plugins.
 */
class DynlibPluginLoader : public PluginLoader {
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

#endif // !IRCCD_PLUGIN_DYNLIB_HPP
