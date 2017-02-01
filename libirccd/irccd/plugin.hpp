/*
 * plugin.hpp -- irccd JavaScript plugin interface
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

#ifndef IRCCD_PLUGIN_HPP
#define IRCCD_PLUGIN_HPP

/**
 * \file plugin.hpp
 * \brief Irccd plugins
 */

/**
 * \defgroup plugins Plugins
 * \brief Plugin management.
 */

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "server.hpp"
#include "sysconfig.hpp"
#include "util.hpp"

namespace irccd {

class Irccd;

/**
 * \brief Configuration map extract from config file.
 */
using PluginConfig = std::unordered_map<std::string, std::string>;

/**
 * \brief Formats for plugins.
 */
using PluginFormats = std::unordered_map<std::string, std::string>;

/**
 * \ingroup plugins
 * \brief Abstract plugin.
 *
 * A plugin is identified by name and can be loaded and unloaded at runtime.
 */
class Plugin : public std::enable_shared_from_this<Plugin> {
private:
    // Plugin information
    std::string m_name;
    std::string m_path;

    // Metadata
    std::string m_author{"unknown"};
    std::string m_license{"unknown"};
    std::string m_summary{"unknown"};
    std::string m_version{"unknown"};

public:
    /**
     * Constructor.
     *
     * \param name the plugin id
     * \param path the fully resolved path to the plugin
     * \throws std::runtime_error on errors
     */
    inline Plugin(std::string name, std::string path) noexcept
        : m_name(std::move(name))
        , m_path(std::move(path))
    {
    }

    /**
     * Temporary, close all timers.
     */
    virtual ~Plugin() = default;

    /**
     * Get the plugin name.
     *
     * \return the plugin name
     */
    inline const std::string &name() const noexcept
    {
        return m_name;
    }

    /**
     * Get the plugin path.
     *
     * \return the plugin path
     * \note some plugins may not exist on the disk
     */
    inline const std::string &path() const noexcept
    {
        return m_path;
    }

    /**
     * Get the author.
     *
     * \return the author
     */
    inline const std::string &author() const noexcept
    {
        return m_author;
    }

    /**
     * Set the author.
     *
     * \param author the author
     */
    inline void setAuthor(std::string author) noexcept
    {
        m_author = std::move(author);
    }

    /**
     * Get the license.
     *
     * \return the license
     */
    inline const std::string &license() const noexcept
    {
        return m_license;
    }

    /**
     * Set the license.
     *
     * \param license the license
     */
    inline void setLicense(std::string license) noexcept
    {
        m_license = std::move(license);
    }

    /**
     * Get the summary.
     *
     * \return the summary
     */
    inline const std::string &summary() const noexcept
    {
        return m_summary;
    }

    /**
     * Set the summary.
     *
     * \param summary the summary
     */
    inline void setSummary(std::string summary) noexcept
    {
        m_summary = std::move(summary);
    }

    /**
     * Get the version.
     *
     * \return the version
     */
    inline const std::string &version() const noexcept
    {
        return m_version;
    }

    /**
     * Set the version.
     *
     * \param version the version
     */
    inline void setVersion(std::string version) noexcept
    {
        m_version = std::move(version);
    }

    /**
     * Access the plugin configuration.
     *
     * \return the config
     */
    virtual PluginConfig config()
    {
        return {};
    }

    /**
     * Set the configuration.
     *
     * \param config the configuration
     */
    virtual void setConfig(PluginConfig config)
    {
        util::unused(config);
    }

    /**
     * Access the plugin formats.
     *
     * \return the format
     */
    virtual PluginFormats formats()
    {
        return {};
    }

    /**
     * Set the formats.
     *
     * \param formats the formats
     */
    virtual void setFormats(PluginFormats formats)
    {
        util::unused(formats);
    }

    /**
     * On channel message. This event will call onMessage or
     * onCommand if the messages starts with the command character
     * plus the plugin name.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void onCommand(Irccd &irccd, const MessageEvent &event)
    {
        util::unused(irccd, event);
    }

    /**
     * On successful connection.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void onConnect(Irccd &irccd, const ConnectEvent &event)
    {
        util::unused(irccd, event);
    }

    /**
     * On channel mode.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void onChannelMode(Irccd &irccd, const ChannelModeEvent &event)
    {
        util::unused(irccd, event);
    }

    /**
     * On a channel notice.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void onChannelNotice(Irccd &irccd, const ChannelNoticeEvent &event)
    {
        util::unused(irccd, event);
    }

    /**
     * On invitation.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void onInvite(Irccd &irccd, const InviteEvent &event)
    {
        util::unused(irccd, event);
    }

    /**
     * On join.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void onJoin(Irccd &irccd, const JoinEvent &event)
    {
        util::unused(irccd, event);
    }

    /**
     * On kick.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void onKick(Irccd &irccd, const KickEvent &event)
    {
        util::unused(irccd, event);
    }

    /**
     * On load.
     *
     * \param irccd the irccd instance
     */
    virtual void onLoad(Irccd &irccd)
    {
        util::unused(irccd);
    }

    /**
     * On channel message.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void onMessage(Irccd &irccd, const MessageEvent &event)
    {
        util::unused(irccd, event);
    }

    /**
     * On CTCP Action.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void onMe(Irccd &irccd, const MeEvent &event)
    {
        util::unused(irccd, event);
    }

    /**
     * On user mode change.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void onMode(Irccd &irccd, const ModeEvent &event)
    {
        util::unused(irccd, event);
    }

    /**
     * On names listing.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void onNames(Irccd &irccd, const NamesEvent &event)
    {
        util::unused(irccd, event);
    }

    /**
     * On nick change.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void onNick(Irccd &irccd, const NickEvent &event)
    {
        util::unused(irccd, event);
    }

    /**
     * On user notice.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void onNotice(Irccd &irccd, const NoticeEvent &event)
    {
        util::unused(irccd, event);
    }

    /**
     * On part.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void onPart(Irccd &irccd, const PartEvent &event)
    {
        util::unused(irccd, event);
    }

    /**
     * On user query.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void onQuery(Irccd &irccd, const QueryEvent &event)
    {
        util::unused(irccd, event);
    }

    /**
     * On user query command.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void onQueryCommand(Irccd &irccd, const QueryEvent &event)
    {
        util::unused(irccd, event);
    }

    /**
     * On reload.
     *
     * \param irccd the irccd instance
     */
    virtual void onReload(Irccd &irccd)
    {
        util::unused(irccd);
    }

    /**
     * On topic change.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void onTopic(Irccd &irccd, const TopicEvent &event)
    {
        util::unused(irccd, event);
    }

    /**
     * On unload.
     *
     * \param irccd the irccd instance
     */
    virtual void onUnload(Irccd &irccd)
    {
        util::unused(irccd);
    }

    /**
     * On whois information.
     *
     * \param irccd the irccd instance
     * \param event the event
     */
    virtual void onWhois(Irccd &irccd, const WhoisEvent &event)
    {
        util::unused(irccd, event);
    }
};

/**
 * \brief Abstract interface for searching plugins.
 *
 * This class is used to make loading of plugins extensible, the PluginService
 * knows some predefined plugins loaders and use them to search for available
 * plugins.
 *
 * This makes easier to implement new plugins or new ways of loading them.
 *
 * \see DynlibPluginLoader
 * \see JsPluginLoader
 */
class PluginLoader {
public:
    /**
     * Try to open the plugin specified by path.
     *
     * The implementation must test if the plugin is suitable for opening, by
     * testing extension for example.
     *
     * \param file the file
     */
    virtual std::shared_ptr<Plugin> open(const std::string &id,
                                         const std::string &file) noexcept = 0;

    /**
     * Search for a plugin named by this id.
     *
     * \param id the plugin id
     * \return the plugin
     */
    virtual std::shared_ptr<Plugin> find(const std::string &id) noexcept = 0;
};

} // !irccd

#endif // !IRCCD_PLUGIN_HPP
